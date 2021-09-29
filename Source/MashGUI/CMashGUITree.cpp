//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUITree.h"
#include "MashDevice.h"
#include "MashTimer.h"

namespace mash
{
	const f32 g_indent = 20.0f;
	const f32 g_buttonSize = 15.0f;
	const f32 g_buttonTextGap = 10.0f;
	const f32 g_itemGap = 2.0f;

	CMashGUITree::CMashGUITree(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUITree(pGUIManager, pInputManager, pParent, destination),
			m_bUpdateNeeded(true), m_selectedItemId(-1), m_pVerticalScrollBar(0), m_pHorizontalScrollBar(0), m_itemCounter(0), m_iDoubleClickTimeLimit(400), m_iLastDoubleClickTime(0),
			m_styleElement(styleElement)
	{
		m_pVerticalScrollBar = m_GUIManager->_GetGUIFactory()->CreateScrollBarView(true, this);
		m_pVerticalScrollBar->SetRenderEnabled(false);
		m_pVerticalScrollBar->RegisterReceiver(aGUIEVENT_SB_VALUE_CHANGE, MashGUIEventFunctor(&CMashGUITree::OnVerticalSBValueChange, this));

		m_pHorizontalScrollBar = m_GUIManager->_GetGUIFactory()->CreateScrollBarView(false, this);
		m_pHorizontalScrollBar->SetRenderEnabled(false);
		m_pHorizontalScrollBar->RegisterReceiver(aGUIEVENT_SB_VALUE_CHANGE, MashGUIEventFunctor(&CMashGUITree::OnHorizontalSBValueChange, this));
	}

	CMashGUITree::~CMashGUITree()
	{
		if (m_pVerticalScrollBar)
		{
			m_pVerticalScrollBar->Drop();
			m_pVerticalScrollBar = 0;
		}

		if (m_pHorizontalScrollBar)
		{
			m_pHorizontalScrollBar->Drop();
			m_pHorizontalScrollBar = 0;
		}
	}

	void CMashGUITree::CalculateVerticalScrollDistance(sItem *root, MashGUIFont *font, f32 &distance)
	{
		const uint32 itemCount = root->children.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			distance += (font->GetMaxCharacterHeight() + g_itemGap);
			if (root->children[i].expand && !root->children[i].children.Empty())
				CalculateVerticalScrollDistance(&root->children[i], font, distance);
		}
	}

	void CMashGUITree::CalculateHorizontalScrollDistance(sItem *root, MashGUIFont *font, f32 &distance, f32 &indent)
	{
		const uint32 itemCount = root->children.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			distance = math::Max<f32>(distance, 10 + indent + g_buttonSize + g_buttonTextGap + font->GetStringLength(root->children[i].textHandler.GetString().GetCString()));
			if (root->children[i].expand && !root->children[i].children.Empty())
			{
				indent += g_indent;
				CalculateHorizontalScrollDistance(&root->children[i], font, distance, indent);
				indent -= g_indent;
			}
		}
	}

	void CMashGUITree::SetItemText(int32 id, const MashStringc &text)
	{
		sItem *selectedItem = GetItemById(&m_rootItem, id);
		if (selectedItem)
		{
			selectedItem->textHandler.SetString(text);

			OnResizeItemOnly(selectedItem, m_GUIManager->GetActiveGUIStyle()->GetFont());
		}
	}

	void CMashGUITree::SetActiveItem(int32 id)
	{
		sItem *selectedItem = 0;
		
		if (id > -1)
			selectedItem = GetItemById(&m_rootItem, id);
		
		if (selectedItem)
		{
			if (m_selectedItemId != selectedItem->id)
			{
				m_selectedItemId = selectedItem->id;

				sGUIEvent newGUIMsg;
				
				newGUIMsg.GUIEvent = aGUIEVENT_TREE_SELECTION_CHANGE;
				newGUIMsg.component = this;
				ImmediateBroadcast(newGUIMsg);
			}
		}
		else
		{
			m_selectedItemId = -1;
		}
	}

	void CMashGUITree::SetActiveItemByUserId(int32 id)
	{
		sItem *selectedItem = 0;

		if (id > -1)
			selectedItem = GetItemByUserId(&m_rootItem, id);

		if (selectedItem)
		{
			if (m_selectedItemId != selectedItem->id)
			{
				m_selectedItemId = selectedItem->id;

				sGUIEvent newGUIMsg;
				
				newGUIMsg.GUIEvent = aGUIEVENT_TREE_SELECTION_CHANGE;
				newGUIMsg.component = this;
				ImmediateBroadcast(newGUIMsg);
			}
		}
		else
		{
			m_selectedItemId = -1;
		}
	}

	CMashGUITree::sItem* CMashGUITree::CheckElementSelection(sItem *root, uint32 currentTime, const mash::MashVector2 &cursorPos)
	{
		const uint32 itemCount = root->children.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (root->children[i].textHandler.GetAbsoluteClippedRegion().IntersectsGUI(cursorPos))
			{
				if (m_selectedItemId != root->children[i].id)
				{
					m_selectedItemId = root->children[i].id;

					sGUIEvent newGUIMsg;
					
					newGUIMsg.GUIEvent = aGUIEVENT_TREE_SELECTION_CHANGE;
					newGUIMsg.component = this;
					ImmediateBroadcast(newGUIMsg);

					return &root->children[i];
				}
				else
				{
					if ((currentTime - m_iLastDoubleClickTime) <= m_iDoubleClickTimeLimit)
					{
						//only expand if it has children
						if (!root->children[i].children.Empty())
						{
							if (root->children[i].expand)
								root->children[i].expand = false;
							else
							{
								root->children[i].expand = true;
							}

							m_bUpdateNeeded = true;
						}

						sGUIEvent newGUIMsg;
						
						newGUIMsg.GUIEvent = aGUIEVENT_TREE_SELECTION_CONFIRMED;
						newGUIMsg.component = this;
						ImmediateBroadcast(newGUIMsg);

						return &root->children[i];
					}
				}
				break;
			}
			else if (root->children[i].expandButtonAbsRegion.IntersectsGUI(cursorPos))
			{
				//only expand if it has children
				if (!root->children[i].children.Empty())
				{
					if (root->children[i].expand)
						root->children[i].expand = false;
					else
					{
						root->children[i].expand = true;
					}

					m_bUpdateNeeded = true;
				}

				return &root->children[i];
			}

			if (root->children[i].expand)
			{
				sItem *selection = CheckElementSelection(&root->children[i], currentTime, cursorPos);
				if (selection)
					return selection;
			}
		}

		return 0;
	}

	void CMashGUITree::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if (eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE && m_hasFocus)
			{
				switch(eventData.action)
				{
				case aMOUSEEVENT_AXISZ:
					{
						break;
					}
				case aMOUSEEVENT_B1:
					{
						if ((eventData.isPressed == 1) && m_mouseHover)
						{
							uint32 iNewClickTime = MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart();
							mash::MashVector2 vMousePos = m_inputManager->GetCursorPosition();

							CheckElementSelection(&m_rootItem, iNewClickTime, vMousePos);

							//if f64 click was engaged then reset the counter to prevent following
							//clicks to also act as a f64 click
							if ((iNewClickTime - m_iLastDoubleClickTime) <= m_iDoubleClickTimeLimit)
								m_iLastDoubleClickTime = 0.0f;
							else
								m_iLastDoubleClickTime = iNewClickTime;
						}
					}
				}
			}
		}
	}

	void CMashGUITree::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		MashGUIComponent::SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);

		m_pHorizontalScrollBar->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		m_pVerticalScrollBar->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
	}

	CMashGUITree::sItem* CMashGUITree::GetItemByUserId(sItem *root, int32 id)const
	{
		if (root->userId == id)
			return root;

		const uint32 itemSize = root->children.Size();
		for(uint32 i = 0; i < itemSize; ++i)
		{
			sItem *item = GetItemByUserId(&root->children[i], id);
			if (item)
				return item;
		}

		return 0;
	}

	CMashGUITree::sItem* CMashGUITree::GetItemById(sItem *root, int32 id)const
	{
		if (root->id == id)
			return root;

		const uint32 itemSize = root->children.Size();
		for(uint32 i = 0; i < itemSize; ++i)
		{
			sItem *item = GetItemById(&root->children[i], id);
			if (item)
				return item;
		}

		return 0;
	}

	bool CMashGUITree::RemoveItem(sItem *root, int32 id)
	{
		const uint32 itemSize = root->children.Size();
		for(uint32 i = 0; i < itemSize; ++i)
		{
			if (root->children[i].id == id)
			{
				root->children.Erase(root->children.Begin() + i);
				m_bUpdateNeeded = true;
				return true;
			}
			else
			{
				if (RemoveItem(&root->children[i], id))
					return true;
			}
		}

		return false;
	}

	void CMashGUITree::RemoveAllItems(sItem *root)
	{
		MashArray<sItem>::Iterator iter = root->children.Begin();
		MashArray<sItem>::Iterator iterEnd = root->children.End();
		for(; iter != iterEnd; ++iter)
		{
			RemoveAllItems(&(*iter));
		}

		root->children.Clear();
		m_bUpdateNeeded = true;
	}

	void CMashGUITree::RemoveAllItems()
	{
		RemoveAllItems(&m_rootItem);
	}

	void CMashGUITree::RemoveItem(int32 id)
	{
		RemoveItem(&m_rootItem, id);
	}

	void CMashGUITree::GetItems(sItem *root, MashArray<sItemData> &out)
	{
		sItemData newItem;
		newItem.id = root->userId;
		newItem.text = root->textHandler.GetString();
		
		for(uint32 i = 0; i < root->children.Size(); ++i)
			GetItems(&root->children[i], newItem.children);

		out.PushBack(newItem);
	}

	void CMashGUITree::GetItems(MashArray<sItemData> &out)
	{
		for(uint32 i = 0; i < m_rootItem.children.Size(); ++i)
			GetItems(&m_rootItem.children[i], out);	
	}

	int32 CMashGUITree::AddItem(const MashStringc &text, int32 parentID, int32 userId)
	{
		MashGUIFont *activeFont = m_GUIManager->GetActiveGUIStyle()->GetFont();
		if (parentID == -1)
		{
			sItem newItem;
			newItem.id = m_itemCounter++;
			newItem.userId = userId;
			newItem.textHandler.SetFormat(activeFont, MashGUIFont::aBOTTOM_LEFT, false);
			newItem.textHandler.SetString(text);
			m_rootItem.children.PushBack(newItem);
			m_bUpdateNeeded = true;
			return newItem.id;
		}
		else
		{
			sItem *parent = GetItemById(&m_rootItem, parentID);
			if (parent)
			{
				sItem newItem;
				newItem.id = m_itemCounter++;
				newItem.userId = userId;
				newItem.textHandler.SetFormat(activeFont, MashGUIFont::aBOTTOM_LEFT, false);
				newItem.textHandler.SetString(text);
				parent->children.PushBack(newItem);
				m_bUpdateNeeded = true;
				return newItem.id;
			}
			else
			{
				return -1;
			}
		}

		return -1;
	}

	void CMashGUITree::SetItemUserId(int32 id, int32 userId)
	{
		sItem *item = GetItemById(&m_rootItem, id);
		if (item)
			item->userId = userId;
	}

	const MashStringc& CMashGUITree::GetItemText(int32 id)
	{
		sItem *item = GetItemById(&m_rootItem, id);
		if (!item)
			return g_staticDefaultString;

		return item->textHandler.GetString();
	}

	int32 CMashGUITree::GetItemUserId(int32 id)
	{
		sItem *item = GetItemById(&m_rootItem, id);
		if (item)
			return item->userId;

		return 0;
	}

	int32 CMashGUITree::GetItemIdByUserValue(int32 userValue)
	{
		sItem *item = GetItemByUserId(&m_rootItem, userValue);
		if (item)
			return item->id;

		return -1;
	}

	void CMashGUITree::OnVerticalSBValueChange(const sGUIEvent &eventData)
	{
		m_bUpdateNeeded = true;
	}

	void CMashGUITree::OnHorizontalSBValueChange(const sGUIEvent &eventData)
	{
		m_bUpdateNeeded = true;
	}

	MashGUIComponent* CMashGUITree::GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren)
	{
		if (bTestAllChildren || (GetRenderEnabled() && GetCanHaveFocus() && (m_cullState != aCULL_CULLED)))
		{
			if (m_pVerticalScrollBar->GetClosestIntersectingChild(vScreenPos, bTestAllChildren))
				return m_pVerticalScrollBar;

			if (m_pHorizontalScrollBar->GetClosestIntersectingChild(vScreenPos, bTestAllChildren))
				return m_pHorizontalScrollBar;

			if (MashGUIComponent::GetClosestIntersectingChild(vScreenPos, bTestAllChildren))
				return this;
		}

		return 0;
	}

	void CMashGUITree::OnStyleChange(MashGUIStyle *style)
	{
		if (m_pVerticalScrollBar)
			m_pVerticalScrollBar->OnStyleChange(style);

		if (m_pHorizontalScrollBar)
			m_pHorizontalScrollBar->OnStyleChange(style);

		m_bUpdateNeeded = true;
	}

	void CMashGUITree::OnResizeItemOnly(sItem *root, MashGUIFont *font)
	{
		mash::MashRectangle2 textAbsRegion;
		textAbsRegion.left = root->expandButtonAbsRegion.right + g_buttonTextGap;
		textAbsRegion.top = root->expandButtonAbsRegion.top;
		textAbsRegion.right = textAbsRegion.left + font->GetStringLength(root->textHandler.GetString().GetCString());
		textAbsRegion.bottom = textAbsRegion.top + font->GetMaxCharacterHeight();

		root->textHandler.SetRegion(textAbsRegion, GetAbsoluteClippedRegion());
	}

	void CMashGUITree::OnResizeItems(sItem *root, MashGUIFont *font, mash::MashVector2 &topLeftPosition)
	{
		mash::MashRectangle2 textAbsRegion;
		const uint32 iItemCount = root->children.Size();
		for(uint32 i = 0; i < iItemCount; ++i)
		{
			root->children[i].expandButtonAbsRegion.left = topLeftPosition.x;
			root->children[i].expandButtonAbsRegion.top = topLeftPosition.y;
			root->children[i].expandButtonAbsRegion.right = topLeftPosition.x + g_buttonSize;
			root->children[i].expandButtonAbsRegion.bottom = topLeftPosition.y + g_buttonSize;

			textAbsRegion.left = root->children[i].expandButtonAbsRegion.right + g_buttonTextGap;
			textAbsRegion.top = topLeftPosition.y;
			textAbsRegion.right = textAbsRegion.left + font->GetStringLength(root->children[i].textHandler.GetString().GetCString());
			textAbsRegion.bottom = textAbsRegion.top + font->GetMaxCharacterHeight();

			root->children[i].textHandler.SetRegion(textAbsRegion, GetAbsoluteClippedRegion());

			topLeftPosition.y = root->children[i].expandButtonAbsRegion.bottom + g_itemGap;

			if (root->children[i].expand)
			{
				topLeftPosition.x += g_indent;
				OnResizeItems(&root->children[i], font, topLeftPosition);
				topLeftPosition.x -= g_indent;
			}
		}
	}

	void CMashGUITree::UpdateItemPositionsOnly(sItem *root, f32 deltaX, f32 deltaY)
	{
		const uint32 iItemCount = root->children.Size();
		for(uint32 i = 0; i < iItemCount; ++i)
		{
			sItem *item = &root->children[i];
			item->expandButtonAbsRegion.left += deltaX;
			item->expandButtonAbsRegion.right += deltaX;
			item->expandButtonAbsRegion.top += deltaY;
			item->expandButtonAbsRegion.bottom += deltaY;

			item->textHandler.AddPosition(deltaX, deltaY);

			if (root->children[i].expand)
				UpdateItemPositionsOnly(&root->children[i], deltaX, deltaY);
		}
	}

	void CMashGUITree::UpdateTree(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
		{
			UpdateItemPositionsOnly(&m_rootItem, deltaX, deltaY);
		}
		else
		{
			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUIFont *activeFont = activeStyle->GetFont();

			//does the vertical scroll bar need to resize?
			f32 totalItemHeight = 10.0f;
			CalculateVerticalScrollDistance(&m_rootItem, activeFont, totalItemHeight);
			f32 currentWindowHeight = GetAbsoluteRegion().bottom - GetAbsoluteRegion().top;

			//add scroll bar height so the item is not blocked
			if (m_pHorizontalScrollBar->GetRenderEnabled())
				totalItemHeight += m_pHorizontalScrollBar->GetAbsoluteRegion().bottom - m_pHorizontalScrollBar->GetAbsoluteRegion().top;

			if (totalItemHeight > currentWindowHeight)
			{
				m_pVerticalScrollBar->SetSliderMaxValue(totalItemHeight - currentWindowHeight);
				m_pVerticalScrollBar->SetRenderEnabled(true);
			}
			else
			{
				m_pVerticalScrollBar->SetRenderEnabled(false);
				m_pVerticalScrollBar->ResetSlider();
			}

			//does the horizontal scroll bar need to resize?
			f32 totalItemWidth = 0.0f;
			f32 currentIndent = 0.0f;
			CalculateHorizontalScrollDistance(&m_rootItem, activeFont, totalItemWidth, currentIndent);
			f32 currentWindowWidth = GetAbsoluteRegion().right - GetAbsoluteRegion().left;

			//add scroll bar width so the item is not blocked
			if (m_pVerticalScrollBar->GetRenderEnabled())
				totalItemWidth += m_pVerticalScrollBar->GetAbsoluteRegion().right - m_pVerticalScrollBar->GetAbsoluteRegion().left;

			if (totalItemWidth > currentWindowWidth)
			{
				m_pHorizontalScrollBar->SetSliderMaxValue(totalItemWidth - currentWindowWidth);
				m_pHorizontalScrollBar->SetRenderEnabled(true);
			}
			else
			{
				m_pHorizontalScrollBar->SetRenderEnabled(false);
				m_pHorizontalScrollBar->ResetSlider();
			}

			//if both bars are enabled then we need to resize them so they dont overlap
			if (m_pVerticalScrollBar->GetRenderEnabled() && m_pHorizontalScrollBar->GetRenderEnabled())
			{
				m_pVerticalScrollBar->SetDualScrollEnabled(true);
				m_pHorizontalScrollBar->SetDualScrollEnabled(true);
			}
			else
			{
				m_pVerticalScrollBar->SetDualScrollEnabled(false);
				m_pHorizontalScrollBar->SetDualScrollEnabled(false);
			}

			mash::MashVector2 topLeftPosition(10.0f, 10.0f);
			//add parent position
			topLeftPosition.x += GetAbsoluteRegion().left;
			topLeftPosition.y += GetAbsoluteRegion().top;
			//add scroll bar
			topLeftPosition.x -= m_pHorizontalScrollBar->GetSliderValue();
			topLeftPosition.y -= m_pVerticalScrollBar->GetSliderValue();

			OnResizeItems(&m_rootItem, activeFont, topLeftPosition);
		}

		m_bUpdateNeeded = false;
	}

	void CMashGUITree::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		m_pVerticalScrollBar->UpdateRegion();
		m_pHorizontalScrollBar->UpdateRegion();

		if (m_bUpdateNeeded)
			positionChangeOnly = false;

		UpdateTree(positionChangeOnly, deltaX, deltaY);
	}

	void CMashGUITree::DrawItems(sItem *root, MashGUISkin *activeItemSkin, MashGUISkin *inactiveItemSkin, MashGUISkin *expandButtonSkin, MashGUISkin *retractButtonSkin)
	{
		const uint32 itemCount = root->children.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			MashGUISkin *currentSkin = 0;
			if (!root->children[i].expand)
				currentSkin = expandButtonSkin;
			else
				currentSkin = retractButtonSkin;

			//draw button
			if (!root->children[i].children.Empty())//only draw the button if this element has children
				m_GUIManager->DrawSprite(root->children[i].expandButtonAbsRegion, GetAbsoluteClippedRegion(), currentSkin, m_overrideTransparency);

			if (root->children[i].id == m_selectedItemId)
				currentSkin = activeItemSkin;
			else
				currentSkin = inactiveItemSkin;

			//draw text
			root->children[i].textHandler.Draw(m_GUIManager, currentSkin->fontColour, m_overrideTransparency);

			//draw children
			if (root->children[i].expand)
				DrawItems(&root->children[i], activeItemSkin, inactiveItemSkin, expandButtonSkin, retractButtonSkin);
		}
	}

	void CMashGUITree::Draw()
	{
		MashGUIComponent::Draw();

		if (m_bUpdateNeeded)
			UpdateTree(false);

		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *backgroundSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND);
			MashGUISkin *activeItemSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_ACTIVE);
			MashGUISkin *inactiveItemSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_INACTIVE);
			MashGUISkin *expandButtonSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_INCREMENT);
			MashGUISkin *retractButtonSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_DECREMENT);

			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, backgroundSkin, m_overrideTransparency);

			/*
				Skins are passed in so they are not being searched for, for each item.
			*/
			DrawItems(&m_rootItem, activeItemSkin, inactiveItemSkin, expandButtonSkin, retractButtonSkin);

			if (m_pHorizontalScrollBar->GetRenderEnabled())
				m_pHorizontalScrollBar->Draw();

			if (m_pVerticalScrollBar->GetRenderEnabled())
				m_pVerticalScrollBar->Draw();
		}
	}
}