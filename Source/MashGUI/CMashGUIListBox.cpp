//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIListBox.h"
#include "MashGUIManager.h"
#include "MashDevice.h"
#include "MashTimer.h"

namespace mash
{
	CMashGUIListBox::CMashGUIListBox(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUIListBox(pGUIManager, pInputManager, pParent, destination),
				m_iSelectedItem(-1), m_pScrollBar(0), m_iDoubleClickTimeLimit(500), m_iLastDoubleClickTime(0),
				m_bUpdateNeeded(false), m_itemHeight(0.0f), m_styleElement(styleElement), m_itemIdCounter(0),
				m_displayIcons(false)
				/*m_iSelectedItem(0)*/
	{
		/*
			The list box calls all the functions that would be called by a parent
			for the scroll bar
		*/
		m_pScrollBar = m_GUIManager->_GetGUIFactory()->CreateScrollBarView(/*scrollRect, */true, this);//MASH_NEW_COMMON CMashGUIScrollbarView(pGUIManager, pInputManager, this, scrollRect, true);
		m_pScrollBar->SetRenderEnabled(false);
		m_pScrollBar->RegisterReceiver(aGUIEVENT_SB_VALUE_CHANGE, MashGUIEventFunctor(&CMashGUIListBox::OnSBValueChange, this));

		EnableIcons(m_displayIcons);
		UpdateDefaultRegions();

		m_itemHeight = 0;
		MashGUIStyle *style = pGUIManager->GetActiveGUIStyle();
		if (style)
		{
			m_itemHeight = style->GetFont()->GetMaxCharacterHeight();
		}

		m_itemHeight += 10;
	}

	CMashGUIListBox::~CMashGUIListBox()
	{
		if (m_pScrollBar)
		{
			m_pScrollBar->Drop();
			m_pScrollBar = 0;
		}

		MashArray<sListBoxItem>::Iterator lbIter = m_listBoxItems.Begin();
		MashArray<sListBoxItem>::Iterator lbIterEnd = m_listBoxItems.End();
		for(; lbIter != lbIterEnd; ++lbIter)
		{
			if (lbIter->pIcon)
			{
				lbIter->pIcon->Drop();
			}
		}

		m_listBoxItems.Clear();
	}

	void CMashGUIListBox::OnStyleChange(MashGUIStyle *style)
	{
		const uint32 itemCount = m_listBoxItems.Size();
		for(uint32 i = 0; i < itemCount; ++i)
			m_listBoxItems[i].textHandler.SetFont(style->GetFont());
	}

	void CMashGUIListBox::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		MashGUIComponent::SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		
		m_pScrollBar->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
	}

	void CMashGUIListBox::UpdateDefaultRegions()
	{
		if (m_displayIcons)
		{
			const f32 textBuffer = 5.0f;
			f32 maxCharacterHeight = m_itemHeight;
			MashGUIStyle *style = m_GUIManager->GetActiveGUIStyle();
			if (style)
			{
				maxCharacterHeight = style->GetFont()->GetMaxCharacterHeight();
			}

			m_ItemIconDestination = MashGUIRect(MashGUIUnit(0.0f, textBuffer), MashGUIUnit(0.0f, textBuffer), 
				MashGUIUnit(0.0f, textBuffer + maxCharacterHeight), MashGUIUnit(0.0f, textBuffer + maxCharacterHeight));

			m_ItemTextDestination = m_ItemIconDestination;
			m_ItemTextDestination.left.offset = m_ItemIconDestination.right.offset + textBuffer;
			m_ItemTextDestination.right.offset = -textBuffer;
			m_ItemTextDestination.right.scale = 1.0f;
		}
		else
		{
			const f32 textBuffer = 5.0f;
			m_ItemTextDestination = MashGUIRect(MashGUIUnit(0.0f, textBuffer), MashGUIUnit(0.0f, textBuffer),
				MashGUIUnit(1.0f, -textBuffer), MashGUIUnit(1.0f, -textBuffer));

			m_ItemIconDestination = MashGUIRect(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
					MashGUIUnit(0.0f, 20.0f), MashGUIUnit(0.0f, 20.0f));
		}
	}

	void CMashGUIListBox::EnableIcons(bool state, bool setIconAndTextRegionsToDefault)
	{
		if (m_displayIcons != state)
		{
			m_displayIcons = state;
			m_bUpdateNeeded = true;
		}

		if (setIconAndTextRegionsToDefault)
			UpdateDefaultRegions();
	}

	void CMashGUIListBox::CalculateMaxVisibleItems()
	{
		//update values for scroll bar
		mash::MashRectangle2 rect = GetAbsoluteRegion();

		if (m_itemHeight == 0)
			m_itemHeight = 1;

		f32 currentRange = rect.bottom - rect.top;
		f32 newRange = m_itemHeight * m_listBoxItems.Size();
		m_pScrollBar->SetSliderMaxValue(newRange - currentRange);

		if (newRange > currentRange)
		{
			m_pScrollBar->SetRenderEnabled(true);
		}
		else
		{
			m_pScrollBar->SetRenderEnabled(false);
		}
	}

	void CMashGUIListBox::SetActiveItem(int32 id, bool sendConfirmedMessage)
	{
		sListBoxItem *item = GetItem(id);
		if (item)
		{
			if (item->id != m_iSelectedItem)
			{
				m_iSelectedItem = item->id;

				sGUIEvent newGUIMsg;
				

				if (sendConfirmedMessage)
					newGUIMsg.GUIEvent = aGUIEVENT_LB_SELECTION_CONFIRMED;
				else
					newGUIMsg.GUIEvent = aGUIEVENT_LB_SELECTION_CHANGE;

				newGUIMsg.component = this;
				ImmediateBroadcast(newGUIMsg);
			}
		}
		else
			m_iSelectedItem = -1;
	}

	void CMashGUIListBox::SetActiveItemByUserValue(int32 val, bool sendConfirmedMessage)
	{
		sListBoxItem *item = GetItemByUserId(val);
		if (item)
		{
			if (item->id != m_iSelectedItem)
			{
				m_iSelectedItem = item->id;

				sGUIEvent newGUIMsg;
				

				if (sendConfirmedMessage)
					newGUIMsg.GUIEvent = aGUIEVENT_LB_SELECTION_CONFIRMED;
				else
					newGUIMsg.GUIEvent = aGUIEVENT_LB_SELECTION_CHANGE;

				newGUIMsg.component = this;
				ImmediateBroadcast(newGUIMsg);
			}
		}
		else
			m_iSelectedItem = -1;
	}

	void CMashGUIListBox::SetItemHeight(uint32 height)
	{
		m_itemHeight = height;
		m_bUpdateNeeded = true;//TODO : Make the changes immediate

		CalculateMaxVisibleItems();
	}

	void CMashGUIListBox::SetItemIcon(int32 id, mash::MashTexture *icon, const mash::MashRectangle2 *source)
	{
		sListBoxItem *item = GetItem(id);
		if (item)
		{
			if (icon)
				icon->Grab();

			if (item->pIcon)
				item->pIcon->Drop();

			item->pIcon = icon;

			if (item->pIcon)
			{
				if (source)
					item->vIconTexCoords = *source;
				else
				{
					uint32 iWidth, iHeight;
					item->pIcon->GetSize(iWidth, iHeight);
					item->vIconTexCoords.left = 0.0f;
					item->vIconTexCoords.top = 0.0f;
					item->vIconTexCoords.right = iWidth;
					item->vIconTexCoords.bottom = iHeight;
				}
			}

			m_bUpdateNeeded = true;
		}
	}

	void CMashGUIListBox::SetItemIconRegion(const MashGUIRect &destination)
	{
		m_ItemIconDestination = destination;
		m_bUpdateNeeded = true;
	}

	void CMashGUIListBox::SetItemTextRegion(const MashGUIRect &destination)
	{
		m_ItemTextDestination = destination;
		m_bUpdateNeeded = true;
	}

	mash::MashRectangle2 CMashGUIListBox::GetItemIconOffsetRegion()const
	{
		MashGUIRect itemDestination(MashGUIRect(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(1.0f, 0.0f), MashGUIUnit(0.0f, m_itemHeight)));

		mash::MashRectangle2 itemRegion;
		itemDestination.GetLocalValue(this->GetAbsoluteRegion(), itemRegion);

		mash::MashRectangle2 finalRegion;
		m_ItemIconDestination.GetLocalValue(itemRegion, finalRegion);
		return finalRegion;
	}

	mash::MashRectangle2 CMashGUIListBox::GetItemTextOffsetRegion()const
	{
		MashGUIRect itemDestination(MashGUIRect(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(1.0f, 0.0f), MashGUIUnit(0.0f, m_itemHeight)));

		mash::MashRectangle2 itemRegion;
		itemDestination.GetLocalValue(this->GetAbsoluteRegion(), itemRegion);

		mash::MashRectangle2 finalRegion;
		m_ItemTextDestination.GetLocalValue(itemRegion, finalRegion);
		return finalRegion;
	}

	const mash::MashRectangle2& CMashGUIListBox::GetItemIconSourceRegion(int32 id)
	{
		sListBoxItem *item = GetItem(id);
		if (item)
			item->vIconTexCoords;

		return mash::MashRectangle2();
	}

	void CMashGUIListBox::SetItemText(int32 id, const MashStringc &text)
	{
		const uint32 itemCount = m_listBoxItems.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_listBoxItems[i].id == id)
			{
				m_listBoxItems[i].textHandler.SetString(text);
				break;
			}
		}
	}

	void CMashGUIListBox::SetItemUserValue(int32 id, int32 val)
	{
		const uint32 itemCount = m_listBoxItems.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_listBoxItems[i].id == id)
			{
				m_listBoxItems[i].returnValue = val;
				break;
			}
		}
	}

	void CMashGUIListBox::SetItemIconSourceRegion(int32 id, const mash::MashRectangle2 &source)
	{
		MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
		MashGUIFont *activeFont = activeStyle->GetFont();
		const uint32 itemCount = m_listBoxItems.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_listBoxItems[i].id == id)
			{
				m_listBoxItems[i].vIconTexCoords = source;
				UpdateItem(&m_listBoxItems[i], i, activeFont);
				break;
			}
		}
	}

	CMashGUIListBox::sListBoxItem* CMashGUIListBox::GetItem(int32 id)
	{
		const uint32 itemCount = m_listBoxItems.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_listBoxItems[i].id == id)
			{
				return &m_listBoxItems[i];
			}
		}

		return 0;
	}

	CMashGUIListBox::sListBoxItem* CMashGUIListBox::GetItemByUserId(int32 val)
	{
		const uint32 itemCount = m_listBoxItems.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_listBoxItems[i].returnValue == val)
			{
				return &m_listBoxItems[i];
			}
		}

		return 0;
	}

	void CMashGUIListBox::UpdateItem(sListBoxItem *pItem, int32 itemPos, MashGUIFont *font)
	{
		f32 iYIndex = (itemPos * m_itemHeight) - m_pScrollBar->GetSliderValue();

		MashGUIRect itemDestination (MashGUIRect(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, iYIndex),
			MashGUIUnit(1.0f, 0.0f), MashGUIUnit(0.0f, (f32)(iYIndex + m_itemHeight))));

		itemDestination.GetAbsoluteValue(this->GetAbsoluteRegion(), pItem->absoluteRect);
		pItem->absoluteClippedRect = pItem->absoluteRect;
		if (pItem->absoluteClippedRect.ClipGUI(this->GetAbsoluteClippedRegion()) == aCULL_CULLED)
			pItem->bCulled = true;
		else
		{
			pItem->bCulled = false;

			//only update text thats not culled
			mash::MashRectangle2 textRegion;
			m_ItemTextDestination.GetAbsoluteValue(pItem->absoluteRect, textRegion);
			pItem->textHandler.SetAlignment(MashGUIFont::aLEFT_CENTER);
			pItem->textHandler.SetRegion(textRegion, pItem->absoluteClippedRect);
		}		
	}

	void CMashGUIListBox::UpdateAllItems(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
		{
			const uint32 iItemCount = m_listBoxItems.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				sListBoxItem *pItem = &m_listBoxItems[i];
				if (!pItem->bCulled)
				{
					pItem->absoluteClippedRect.left += deltaX;
					pItem->absoluteClippedRect.right += deltaX;
					pItem->absoluteClippedRect.top += deltaY;
					pItem->absoluteClippedRect.bottom += deltaY;

					pItem->absoluteRect.left += deltaX;
					pItem->absoluteRect.right += deltaX;
					pItem->absoluteRect.top += deltaY;
					pItem->absoluteRect.bottom += deltaY;

					pItem->textHandler.AddPosition(deltaX, deltaY);
				}
			}
		}
		else
		{
			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUIFont *activeFont = activeStyle->GetFont();
			const uint32 iItemCount = m_listBoxItems.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				UpdateItem(&m_listBoxItems[i], i, activeFont);
			}
		}
		

		m_bUpdateNeeded = false;
	}

	void CMashGUIListBox::OnSBValueChange(const sGUIEvent &eventData)
	{
		m_bUpdateNeeded = true;
	}

	void CMashGUIListBox::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if (eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE && m_hasFocus)
			{
				switch(eventData.action)
				{
				case aMOUSEEVENT_AXISZ:
					{
						if (m_pScrollBar->GetRenderEnabled())
						{
							if (eventData.value < 0)
							{
								m_pScrollBar->MoveSlider(m_pScrollBar->GetWheelScrollAmount());
							}
							else
							{
								m_pScrollBar->MoveSlider(-m_pScrollBar->GetWheelScrollAmount());
							}

							m_bUpdateNeeded = true;
						}

						break;
					}
				case aMOUSEEVENT_B1:
					{
						if ((eventData.isPressed == 1) && m_mouseHover)
						{
							uint32 iNewClickTime = MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart();
							mash::MashVector2 vMousePos = m_inputManager->GetCursorPosition();

							const uint32 iItemCount = m_listBoxItems.Size();
							for(uint32 i = 0; i < iItemCount; ++i)
							{
								sListBoxItem *pItem = &m_listBoxItems[i];

								if (pItem->absoluteClippedRect.IntersectsGUI(vMousePos))
								{
									if (m_iSelectedItem != pItem->id)
									{
										m_iSelectedItem = pItem->id;

										sGUIEvent newGUIMsg;
										
										newGUIMsg.GUIEvent = aGUIEVENT_LB_SELECTION_CHANGE;
										newGUIMsg.component = this;
										ImmediateBroadcast(newGUIMsg);
									}
									else
									{
										
										if ((iNewClickTime - m_iLastDoubleClickTime) <= m_iDoubleClickTimeLimit)
										{
											sGUIEvent newGUIMsg;
											
											newGUIMsg.GUIEvent = aGUIEVENT_LB_SELECTION_CONFIRMED;
											newGUIMsg.component = this;
											ImmediateBroadcast(newGUIMsg);

											//avoids continuous f64 clicks
											iNewClickTime = 0;
										}
									}
									break;
								}
							}

							m_iLastDoubleClickTime = iNewClickTime;
						}
					}
				}
			}
		}
	}

	void CMashGUIListBox::RemoveItem(int32 id) 
	{
		const uint32 itemCount = m_listBoxItems.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_listBoxItems[i].id == id)
			{
				m_listBoxItems.Erase(m_listBoxItems.Begin() + i);
				m_bUpdateNeeded = true;
				break;
			}
		}

		CalculateMaxVisibleItems();
	}

	int32 CMashGUIListBox::AddItem(const MashStringc &text, int32 userValue, mash::MashTexture *pIcon, const mash::MashRectangle2 *iconDestRegion)
	{
		sListBoxItem newItem;

		const int32 newId = m_itemIdCounter++;
		newItem.id = newId;
		newItem.returnValue = userValue;

		MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
		newItem.textHandler.SetFormat(activeStyle->GetFont(), MashGUIFont::aLEFT_CENTER, false);
		newItem.textHandler.SetString(text);

		if (pIcon)
		{
			newItem.pIcon = pIcon;
			pIcon->Grab();

			if (iconDestRegion)
				newItem.vIconTexCoords = *iconDestRegion;
			else
			{
				uint32 iWidth, iHeight;
				pIcon->GetSize(iWidth, iHeight);
				newItem.vIconTexCoords.left = 0.0f;
				newItem.vIconTexCoords.top = 0.0f;
				newItem.vIconTexCoords.right = iWidth;
				newItem.vIconTexCoords.bottom = iHeight;
			}
		}		

		//add item to list
		m_listBoxItems.PushBack(newItem);

		CalculateMaxVisibleItems();
		
		UpdateItem(&m_listBoxItems.Back(), m_listBoxItems.Size() - 1, activeStyle->GetFont());

		return newId;	
	}

	uint32 CMashGUIListBox::GetItemCount()const
	{
		return m_listBoxItems.Size();
	}

	MashGUIComponent* CMashGUIListBox::GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren)
	{
		if (bTestAllChildren || (GetRenderEnabled() && GetCanHaveFocus() && (m_cullState != aCULL_CULLED)))
		{
			if (m_pScrollBar->GetClosestIntersectingChild(vScreenPos, bTestAllChildren))
				return m_pScrollBar;

			if (MashGUIComponent::GetClosestIntersectingChild(vScreenPos, bTestAllChildren))
				return this;
		}

		return 0;
	}

	int32 CMashGUIListBox::GetItemUserValue(int32 id)
	{
		sListBoxItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return -1;

		return selectedItem->returnValue;
	}

	const MashStringc& CMashGUIListBox::GetItemText(int32 id)
	{
		sListBoxItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return g_staticDefaultString;

		return selectedItem->textHandler.GetString();
	}

	mash::MashTexture* CMashGUIListBox::GetItemIcon(int32 id)
	{
		sListBoxItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return 0;

		return selectedItem->pIcon;
	}

	void CMashGUIListBox::ClearAllItems()
	{
		MashArray<sListBoxItem>::Iterator lbIter = m_listBoxItems.Begin();
		MashArray<sListBoxItem>::Iterator lbIterEnd = m_listBoxItems.End();
		for(; lbIter != lbIterEnd; ++lbIter)
		{
			if (lbIter->pIcon)
			{
				lbIter->pIcon->Drop();
			}
		}

		m_listBoxItems.Clear();
		m_iSelectedItem = -1;

		m_pScrollBar->ResetSlider();
		CalculateMaxVisibleItems();
	}

	void CMashGUIListBox::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		m_pScrollBar->UpdateRegion();

		if (m_bUpdateNeeded)
			positionChangeOnly = false;

		UpdateAllItems(positionChangeOnly, deltaX, deltaY);

		if (!positionChangeOnly)
			CalculateMaxVisibleItems();
	}

	void CMashGUIListBox::Draw()
	{
		if (m_bUpdateNeeded)
			UpdateAllItems(false);

		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			/*
				Setup the custom batch for fast rendering
			*/
			//m_GUIManager->SetCustomBatch(m_pListBoxBatch);
			//m_GUIManager->BeginBatch();

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *backgroundSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND);
			MashGUISkin *inactiveItemSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_INACTIVE);
			MashGUISkin *activeItemSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_ACTIVE);

			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, backgroundSkin, m_overrideTransparency);

			const uint32 iItemCount = m_listBoxItems.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				sListBoxItem *pItem = &m_listBoxItems[i];

				/*
					Because we only draw non culled object, it means all items that are culled
					will not have their text updated. This is a very good thing!
				*/
				if (!pItem->bCulled)
				{
					MashGUISkin *itemSkin = inactiveItemSkin;

					if (pItem->id == m_iSelectedItem)
						itemSkin = activeItemSkin;

					m_GUIManager->DrawSprite(pItem->absoluteRect, this->GetAbsoluteClippedRegion(), itemSkin, m_overrideTransparency);

					pItem->textHandler.Draw(m_GUIManager, itemSkin->fontColour, m_overrideTransparency);
				}
			}
			
			//m_GUIManager->EndBatch();

			/*
				Note, icon rendering was seperated from other rendering
				to improve batching
			*/
			//m_GUIManager->BeginBatch();

			//MashGUISkin *pCurrentSkin = &m_inactiveItemSkin;
			//mash::MashRectangle2 oldSource = pCurrentSkin->baseSource;
			//mash::MashTexture *oldTexture = pCurrentSkin->pBaseTexture;

			if (m_displayIcons)
			{
				MashGUISkin *iconSkin = (MashGUISkin*)inactiveItemSkin;
				mash::MashRectangle2 origBaseSource = iconSkin->baseSource;
				mash::MashTexture *origTexture = iconSkin->GetTexture();
				sMashColour origColour = iconSkin->baseColour;

				iconSkin->baseColour = sMashColour(255, 255, 255, 255);

				for(uint32 i = 0; i < iItemCount; ++i)
				{
					sListBoxItem *pItem = &m_listBoxItems[i];

					if (!pItem->bCulled)
					{
						if (pItem->pIcon)
						{
							mash::MashRectangle2 iconRegion;
							m_ItemIconDestination.GetAbsoluteValue(pItem->absoluteRect, iconRegion);
							//set texture details
							iconSkin->baseSource = pItem->vIconTexCoords;
							iconSkin->SetTexture(pItem->pIcon);
							//draw icon
							m_GUIManager->DrawSprite(iconRegion, pItem->absoluteClippedRect, iconSkin, m_overrideTransparency);
						}
					}
				}

				iconSkin->baseSource = origBaseSource;
				iconSkin->SetTexture(origTexture);
				iconSkin->baseColour = origColour;
			}

			//m_GUIManager->EndBatch();

			//pCurrentSkin->baseSource = oldSource;
			//pCurrentSkin->pBaseTexture = oldTexture;

			//m_GUIManager->SetDefaultBatch();
			/*
				Draw scrollbar after the batch so it appears on top
			*/
			m_pScrollBar->Draw();
		}
	}
}