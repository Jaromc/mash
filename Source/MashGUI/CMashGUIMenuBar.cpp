//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIMenuBar.h"
#include "MashGUIManager.h"
#include "MashGUIPopupMenu.h"

namespace mash
{
	CMashGUIMenuBar::CMashGUIMenuBar(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			MashGUIRect &destination,
			f32 menuBarHeight,
			int32 styleElement):MashGUIMenuBar(pGUIManager, pInputManager, pParent,
			destination), m_fMenuBarHeight(menuBarHeight), m_bForceItemUpdate(false), m_styleElement(styleElement),
			m_itemCounter(0), m_focusedItemId(-1), m_lastSelectedPopup(0)
	{

	}

	CMashGUIMenuBar::~CMashGUIMenuBar()
	{
		MashArray<sItem>::Iterator itemIter = m_itemList.Begin();
		MashArray<sItem>::Iterator itemIterEnd = m_itemList.End();
		for(; itemIter != itemIterEnd; ++itemIter)
		{
			if (itemIter->pSubMenu)
			{
				itemIter->pSubMenu->Drop();
			}
		}

		m_itemList.Clear();
	}

	void CMashGUIMenuBar::OnStyleChange(MashGUIStyle *style)
	{
		m_fMenuBarHeight = style->GetFont()->GetMaxCharacterHeight();

		const uint32 itemCount = m_itemList.Size();
		for(uint32 i = 0; i < itemCount; ++i)
			m_itemList[i].textHandler.SetFont(style->GetFont());
	}

	void CMashGUIMenuBar::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		MashGUIComponent::SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		
		const uint32 itemCount = m_itemList.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_itemList[i].pSubMenu)
				m_itemList[i].pSubMenu->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		}
	}

	CMashGUIMenuBar::sItem* CMashGUIMenuBar::GetItem(int32 id)
	{
		const uint32 itemCount = m_itemList.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_itemList[i].id == id)
				return &m_itemList[i];
		}

		return 0;
	}

	MashGUIPopupMenu* CMashGUIMenuBar::GetItemSubMenu(int32 id)
	{
		sItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return 0;

		return selectedItem->pSubMenu;
	}

	void CMashGUIMenuBar::SetItemText(int32 id, const MashStringc &text)
	{
		sItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return;

		selectedItem->textHandler.SetString(text);
		m_bForceItemUpdate = true;
	}

	void CMashGUIMenuBar::SetItemValue(int32 id, int32 val)
	{
		sItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return;

		selectedItem->userValue = val;
	}

	void CMashGUIMenuBar::RemoveItem(int32 id)
	{
		const uint32 itemCount = m_itemList.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_itemList[i].id == id)
			{
				if (m_itemList[i].pSubMenu)
				{
					m_itemList[i].pSubMenu->Destroy();
				}

				m_itemList[i].pSubMenu = 0;
				m_itemList.Erase(m_itemList.Begin() + i);

				if (m_focusedItemId == id)
				{
					m_focusedItemId = -1;
				}

				break;
			}
		}

		//force update to fill in any holes
		m_bForceItemUpdate = true;
	}

	int32 CMashGUIMenuBar::AddItem(const MashStringc &text, MashGUIPopupMenu *pSubMenu, int32 userValue)
	{
		if (!pSubMenu)
			return -1;

		MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
		sItem newItem;
		newItem.textHandler.SetString(text);
		newItem.textHandler.SetFormat(activeStyle->GetFont(), MashGUIFont::aCENTER, false);
		newItem.pSubMenu = pSubMenu;
		newItem.id = m_itemCounter++;
		newItem.userValue = userValue;
		m_itemList.PushBack(newItem);

		//listen for on lost focus events
		pSubMenu->RegisterReceiver(aGUIEVENT_LOST_INPUTFOCUS, MashGUIEventFunctor(&CMashGUIMenuBar::OnSubMenuLostFocus, this));
		//run popup messages through the menu bar
		pSubMenu->RegisterReceiver(aGUIEVENT_POPUP_SELECTION, MashGUIEventFunctor(&CMashGUIMenuBar::OnSubMenuSelection, this));

		m_bForceItemUpdate = true;
		return newItem.id;
	}

	int32 CMashGUIMenuBar::GetItemUserValue(int32 id)
	{
		sItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return 0;

		return selectedItem->userValue;
	}

	void CMashGUIMenuBar::GetItems(MashArray<sMenuBarItem> &out)const
	{
		const uint32 itemCount = m_itemList.Size();
		out.Resize(itemCount);
		for(uint32 i = 0; i < itemCount; ++i)
		{
			out[i].id = m_itemList[i].id;
			out[i].userValue = m_itemList[i].userValue;
			out[i].text = m_itemList[i].textHandler.GetString();
		}
	}

	void CMashGUIMenuBar::OnMouseEnter(const mash::MashVector2 &vScreenPos)
	{
		UpdateHoverElement(vScreenPos);
	}

	void CMashGUIMenuBar::OnLostFocus()
	{
		/*
			Only loose focus if the active item does not have a popup open
		*/
		sItem *selectedItem = GetItem(m_focusedItemId);
		if (selectedItem && 
			(!selectedItem->pSubMenu || (selectedItem->pSubMenu && !selectedItem->pSubMenu->IsActive())))
		{
			selectedItem->pSubMenu->Deactivate();
			selectedItem->bHasFocus = false;
			m_focusedItemId = -1;
		}
	}

	void CMashGUIMenuBar::OnMouseExit(const mash::MashVector2 &vScreenPos)
	{
		/*
			Only loose focus if the active item does not have a popup open
		*/
		sItem *selectedItem = GetItem(m_focusedItemId);
		if ((selectedItem) && 
			(!selectedItem->pSubMenu || (selectedItem->pSubMenu && !selectedItem->pSubMenu->IsActive())))
		{
			selectedItem->pSubMenu->Deactivate();
			selectedItem->bHasFocus = false;
			m_focusedItemId = -1;
		}
	}

	void CMashGUIMenuBar::UpdateItems(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
		{
			const uint32 iItemCount = m_itemList.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				sItem *item = &m_itemList[i];

				item->absoluteRect.left += deltaX;
				item->absoluteRect.right += deltaX;
				item->absoluteRect.top += deltaY;
				item->absoluteRect.bottom += deltaY;

				item->textHandler.AddPosition(deltaX, deltaY);
			}
		}
		else
		{
			const f32 itemBuffer = 20.0f;
			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();

			//update button rects
			f32 fCurrentWidth = 0.0f;
			const uint32 iItemCount = m_itemList.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				const f32 itemWidth = activeStyle->GetFont()->GetStringLength(m_itemList[i].textHandler.GetString().GetCString()) + itemBuffer;
				m_itemList[i].absoluteRect.left = m_absoluteRegion.left + fCurrentWidth;
				m_itemList[i].absoluteRect.right = m_itemList[i].absoluteRect.left + itemWidth;
				m_itemList[i].absoluteRect.top = m_absoluteRegion.top;
				m_itemList[i].absoluteRect.bottom = m_absoluteRegion.bottom;

				fCurrentWidth += itemWidth;

				m_itemList[i].textHandler.SetRegion(m_itemList[i].absoluteRect, m_absoluteClippedRegion);
			}
		}

		m_bForceItemUpdate = false;
	}

	void CMashGUIMenuBar::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (m_bForceItemUpdate)
			positionChangeOnly = false;

		UpdateItems(positionChangeOnly, deltaX, deltaY);
	}

	void CMashGUIMenuBar::Draw()
	{
		/*
			This occurs when a item is added/removed
		*/
		if (m_bForceItemUpdate)
			UpdateItems(false);

		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *backgroundSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND);

			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, backgroundSkin, m_overrideTransparency);

			if (!m_itemList.Empty())
			{
				const uint32 iItemCount = m_itemList.Size();
				for(uint32 i = 0; i < iItemCount; ++i)
				{
					MashGUISkin *activeSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_INACTIVE);
					if (m_itemList[i].bHasFocus)
						activeSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_ACTIVE);

					//draw buttons
					m_GUIManager->DrawSprite(m_itemList[i].absoluteRect, this->GetAbsoluteClippedRegion(), activeSkin, m_overrideTransparency);

					//draw text
					m_itemList[i].textHandler.Draw(m_GUIManager, activeSkin->fontColour, m_overrideTransparency);
				}
			}
		}
	}
    
	void CMashGUIMenuBar::UpdateHoverElement(const mash::MashVector2 &vMousePos)
	{
		if (m_mouseHover)
		{
			sItem *focusedItem = GetItem(m_focusedItemId);

			//bool focusFound = false;
			const uint32 iItemSize = m_itemList.Size();
			for(uint32 i = 0; i < iItemSize; ++i)
			{
				if (m_itemList[i].absoluteRect.IntersectsGUI(vMousePos))
				{
					//focusFound = true;

					if (!focusedItem || (focusedItem != &m_itemList[i]))
					{
						bool bInstantSelectionEnabled = false;
						/*
							If the focused item had a submenu open then remove
							its focus so that it deactivates, and input is restored
							to the parent(this) object
						*/
						if (focusedItem && focusedItem->pSubMenu && focusedItem->pSubMenu->GetHasFocus())
						{
							focusedItem->pSubMenu->Deactivate();
							m_GUIManager->SetFocusedElement(this);
							bInstantSelectionEnabled = true;
						}

						if (focusedItem)
							focusedItem->bHasFocus = false;

						m_focusedItemId = m_itemList[i].id;
						focusedItem = &m_itemList[i];
						focusedItem->bHasFocus = true;

						if (bInstantSelectionEnabled)
						{
							const mash::MashVector2 vStartPos(focusedItem->absoluteRect.left, focusedItem->absoluteRect.bottom);
							focusedItem->pSubMenu->Activate(vStartPos);
						}
					}

					break;
				}
			}		
		}
	}

	void CMashGUIMenuBar::OnSubMenuLostFocus(const sGUIEvent &eventData)
	{
		if (eventData.component == this)
			return;

		sItem *focusedItem = GetItem(m_focusedItemId);
		/*
			When a popup is deactivated it sends through a lost focus message.
			This allows us to do some cleaning here.
		*/
		if (!m_mouseHover && focusedItem && 
			focusedItem->pSubMenu && 
			!focusedItem->pSubMenu->IsActive())
		{
			focusedItem->bHasFocus = false;
			m_focusedItemId = -1;
		}
	}

	void CMashGUIMenuBar::OnSubMenuSelection(const sGUIEvent &eventData)
	{
		m_lastSelectedPopup = (MashGUIPopupMenu*)eventData.component;
		//send messages
		sGUIEvent newGUIMsg;
		
		newGUIMsg.GUIEvent = aGUIEVENT_MENUBAR_SELECTION;
		newGUIMsg.component = this;
		ImmediateBroadcast(newGUIMsg);
	}

	void CMashGUIMenuBar::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if (m_mouseHover && eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE)
			{
				switch(eventData.action)
				{
				case aMOUSEEVENT_B1:
					{
						if (eventData.isPressed == 1)
						{
							mash::MashVector2 vMousePos = m_inputManager->GetCursorPosition();

							if (m_focusedItemId == -1)
							{
								UpdateHoverElement(vMousePos);

								if (m_focusedItemId == -1)
									break;
							}
							sItem *focusedItem = GetItem(m_focusedItemId);

							if (focusedItem->pSubMenu && !focusedItem->pSubMenu->IsActive())
							{
								const mash::MashVector2 vStartPos(focusedItem->absoluteRect.left, focusedItem->absoluteRect.bottom);
								focusedItem->pSubMenu->Activate(vStartPos);
							}
						}

						break;
					};
				case aMOUSEEVENT_AXISX:
				case aMOUSEEVENT_AXISY:
					{
						mash::MashVector2 vMousePos = m_inputManager->GetCursorPosition();

						UpdateHoverElement(vMousePos);

						break;
					};
				}
			}
		}
	}
}