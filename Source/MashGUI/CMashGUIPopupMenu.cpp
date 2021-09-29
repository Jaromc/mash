//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIPopupMenu.h"
#include "MashGUIManager.h"

namespace mash
{
	CMashGUIPopupMenu::CMashGUIPopupMenu(mash::MashGUIManager *pGUIManager, 
		MashInputManager *pInputManager,
		mash::MashGUIComponent *pParent,
			int32 styleElement):MashGUIPopupMenu(pGUIManager, pInputManager, pParent,
			MashGUIRect()), m_vTopLeft(0.0f, 0.0f),
			m_pPopupOwner(0), m_iAttachmentIndex(-1),
			m_bForceUpdate(false), m_styleElement(styleElement), m_itemIdCounter(0),
			m_itemHorizontalBuffer(5.0f), m_focusedItem(-1), m_lastFocusedItem(-1)
	{
		this->SetRenderEnabled(false);
		this->SetClippingEnabled(false);
	}

	CMashGUIPopupMenu::~CMashGUIPopupMenu()
	{
		MashArray<sItem*>::Iterator itemIter = m_itemList.Begin();
		MashArray<sItem*>::Iterator itemIterEnd = m_itemList.End();
		for(; itemIter != itemIterEnd; ++itemIter)
		{
			if ((*itemIter)->pSubMenu)
				(*itemIter)->pSubMenu->Drop();

			MASH_DELETE_T(sItem, *itemIter);
		}

		m_itemList.Clear();
	}

	void CMashGUIPopupMenu::OnStyleChange(MashGUIStyle *style)
	{
		const uint32 itemCount = m_itemList.Size();
		for(uint32 i = 0; i < itemCount; ++i)
			m_itemList[i]->textHandler.SetFont(style->GetFont());
	}

	void CMashGUIPopupMenu::OnDetach()
	{
		m_pPopupOwner = 0;
		m_iAttachmentIndex = -1;
	}

	void CMashGUIPopupMenu::OnAttach(MashGUIPopupMenu *pOwner, int32 iAttachmentIndex)
	{
		m_pPopupOwner = pOwner;
		m_iAttachmentIndex = iAttachmentIndex;
	}

	void CMashGUIPopupMenu::DetachSubMenu(int32 iAttachmentIndex)
	{
		sItem *selectItem = GetItem(iAttachmentIndex);
		if (!selectItem)
			return;

		if (selectItem->pSubMenu)
		{
			MashGUIPopupMenu *tempPopup = selectItem->pSubMenu;
			selectItem->pSubMenu = 0;
			((CMashGUIPopupMenu*)tempPopup)->OnDetach();
		}
	}

	void CMashGUIPopupMenu::DetachFromPopupOwner()
	{
		if (m_pPopupOwner)
			m_pPopupOwner->DetachSubMenu(m_iAttachmentIndex);
	}

	void CMashGUIPopupMenu::AttachSubMenu(int32 id, MashGUIPopupMenu *pSubMenu)
	{
		sItem *selectItem = GetItem(id);
		if (!selectItem)
			return;

		if (selectItem->pSubMenu)
			((CMashGUIPopupMenu*)selectItem->pSubMenu)->OnDetach();

		selectItem->pSubMenu = pSubMenu;

		((CMashGUIPopupMenu*)pSubMenu)->OnAttach(this, id);
	}

	CMashGUIPopupMenu::sItem* CMashGUIPopupMenu::GetItem(int32 id)
	{
		const uint32 itemCount = m_itemList.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_itemList[i]->id == id)
				return m_itemList[i];
		}

		return 0;
	}

	void CMashGUIPopupMenu::GetItems(MashArray<sPopupItem> &out)const
	{
		const uint32 itemCount = m_itemList.Size();
		out.Resize(itemCount);
		for(uint32 i = 0; i < itemCount; ++i)
		{
			out[i].id = m_itemList[i]->id;
			out[i].returnValue = m_itemList[i]->returnValue;
			out[i].text = m_itemList[i]->textHandler.GetString();

			if (m_itemList[i]->pSubMenu)
				out[i].hasSubMenu = true;
			else 
				out[i].hasSubMenu = false;
		}
	}

	MashGUIPopupMenu* CMashGUIPopupMenu::GetItemSubMenu(int32 id)
	{
		sItem *selectedItem = GetItem(id);
		if (!selectedItem)
			return 0;

		return selectedItem->pSubMenu;
	}

	int32 CMashGUIPopupMenu::GetItemUserValue(int32 id)
	{
		sItem *selectedItem = (sItem*)GetItem(id);
		if (!selectedItem)
			return -1;

		return selectedItem->returnValue;
	}

	void CMashGUIPopupMenu::SetItemText(int32 id, const MashStringc &text)
	{
		sItem *selectedItem = (sItem*)GetItem(id);
		if (!selectedItem)
			return;

		selectedItem->textHandler.SetString(text);
		m_bForceUpdate = true;
	}

	void CMashGUIPopupMenu::SetItemUserValue(int32 id, int32 value)
	{
		sItem *selectedItem = (sItem*)GetItem(id);
		if (!selectedItem)
			return;

		selectedItem->returnValue = value;
	}

	void CMashGUIPopupMenu::RemoveItem(int32 id)
	{
		const uint32 itemCount = m_itemList.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			if (m_itemList[i]->id == id)
			{
				if (m_itemList[i]->pSubMenu)
				{
					m_itemList[i]->pSubMenu->Destroy();
				}

				m_itemList[i]->pSubMenu = 0;
				MASH_DELETE_T(sItem, m_itemList[i]);
				m_itemList.Erase(m_itemList.Begin() + i);

				if (m_focusedItem == id)
				{
					m_focusedItem = -1;
				}

				break;
			}
		}

		//force update to fill in any holes
		m_bForceUpdate = true;
	}

	int32 CMashGUIPopupMenu::AddItem(const MashStringc &text, int32 returnValue/*, bool bIsChecked*/, MashGUIPopupMenu *pSubMenu)
	{
		MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
		sItem *newItem = MASH_NEW_T_COMMON(sItem);
		newItem->textHandler.SetString(text);
		newItem->textHandler.SetFormat(activeStyle->GetFont(), MashGUIFont::aTOP_LEFT, false);
		newItem->returnValue = returnValue;
		newItem->id = m_itemIdCounter++;

		newItem->pSubMenu = 0;
		m_itemList.PushBack(newItem);

		if (pSubMenu)
			AttachSubMenu(newItem->id, pSubMenu);

		/*
			If for some reason objects are added while this is being rendered
		*/
		m_bForceUpdate = true;

		return newItem->id;
	}

	void CMashGUIPopupMenu::_SetHasFocus(bool bHasFocus)
	{
		if (bHasFocus == m_hasFocus)
			return;

		sItem *focusedItem = GetItem(m_focusedItem);
		/*
			Opening a submenu forces a change in focus, causing the parent
			object to lose focus. This is fine however we dont want the
			parent to be deactivated. So this check stops that from occuring.
		*/
		if (!(focusedItem && focusedItem->pSubMenu && focusedItem->pSubMenu->GetRenderEnabled()))
		{
			if (m_hasFocus && !bHasFocus)
			{
				Deactivate();
			}
			
			MashGUIComponent::_SetHasFocus(bHasFocus);
		}

		/*
			If we have lost focus and the mouse is not over the parent then it should
			be deactivated to.
		*/
		if (m_pPopupOwner && !m_hasFocus)
		{
			if (!m_pPopupOwner->GetMouseHover())
				m_pPopupOwner->_SetHasFocus(false);
		}
	}

	void CMashGUIPopupMenu::Activate(const mash::MashVector2 &vTopLeft)
	{
		if (!m_renderEnabled)
		{
			this->SetRenderEnabled(true);
			m_GUIManager->SetFocusedElement(this);

			//needs to be parented to the main window so that it gets rendered!
			m_GUIManager->GetRootWindow()->AddChild(this);

			m_vTopLeft = vTopLeft;
			m_bForceUpdate = true;
		}
	}

	void CMashGUIPopupMenu::Deactivate()
	{
		this->SetRenderEnabled(false);

		/*
			Note, the focus is not removed here. Popup deactivation is
			called by the gui manager when it looses focus.
		*/

		const uint32 iItemCount = m_itemList.Size();
		for(uint32 i = 0; i < iItemCount; ++i)
		{
			m_itemList[i]->bHasFocus = false;
		}

		sItem *focusedItem = GetItem(m_focusedItem);

		if (focusedItem && focusedItem->pSubMenu)
		{
			focusedItem->pSubMenu->Deactivate();
		}

		Detach();

		m_focusedItem = -1;
	}

	bool CMashGUIPopupMenu::IsActive()const
	{
		return m_renderEnabled;
	}

	void CMashGUIPopupMenu::UpdatePopup(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
		{
			const uint32 iItemCount = m_itemList.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				sItem *item = m_itemList[i];
				item->absoluteRect.left += deltaX;
				item->absoluteRect.right += deltaX;
				item->absoluteRect.top += deltaY;
				item->absoluteRect.bottom += deltaY;

				item->textHandler.AddPosition(deltaX, deltaY);
			}
		}
		else
		{
			/*
				Note, a popup will only ever be parented to the gui manager
			*/
			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUIFont *activeFont = activeStyle->GetFont();

			m_destinationRegion.Zero();
			m_destinationRegion.left.offset = m_vTopLeft.x;
			m_destinationRegion.top.offset = m_vTopLeft.y;
			m_destinationRegion.bottom.offset = m_destinationRegion.top.offset + (activeFont->GetMaxCharacterHeight() * m_itemList.Size());
			m_destinationRegion.right.offset = 0.0f;

			const uint32 iItemCount = m_itemList.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				f32 fElementLength = m_destinationRegion.left.offset + activeFont->GetStringLength(m_itemList[i]->textHandler.GetString().GetCString());
				m_destinationRegion.right.offset = math::Max<f32>(m_destinationRegion.right.offset, fElementLength);
			}

			const f32 subMenuIndicatorSize = activeFont->GetMaxCharacterHeight();
			const f32 itemRightBuffer = (m_itemHorizontalBuffer * 2) + subMenuIndicatorSize;
			/*
				...Gap...Text...Gap...Indicator...Gap
			*/
			m_destinationRegion.right.offset += m_itemHorizontalBuffer + itemRightBuffer;

			//update the absolute region immediatly
			m_destinationRegion.GetAbsoluteValue(m_parent->GetAbsoluteRegion(), m_absoluteRegion);
			m_absoluteClippedRegion = m_absoluteRegion;
			m_cullState = m_absoluteClippedRegion.ClipGUI(m_parent->GetAbsoluteClippedRegion());

			f32 fCurrentHeight = m_absoluteRegion.top;
			const uint32 fTextHeight = activeFont->GetMaxCharacterHeight();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				m_itemList[i]->absoluteRect.left = m_absoluteRegion.left;
				m_itemList[i]->absoluteRect.right = m_absoluteRegion.right;
				m_itemList[i]->absoluteRect.top = fCurrentHeight;
				m_itemList[i]->absoluteRect.bottom = m_itemList[i]->absoluteRect.top + fTextHeight;

				fCurrentHeight += fTextHeight;

				mash::MashRectangle2 textRect = m_itemList[i]->absoluteRect;
				textRect.left += m_itemHorizontalBuffer;
				textRect.right -= itemRightBuffer;

				m_itemList[i]->textHandler.SetRegion(textRect, textRect);
			}
			
			//TODO : Add extra width for check box
		}
		

		m_bForceUpdate = false;

		
	}

	void CMashGUIPopupMenu::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{	
		if (m_bForceUpdate)
			positionChangeOnly = false;

		UpdatePopup(positionChangeOnly, deltaX, deltaY);
	}

	void CMashGUIPopupMenu::Draw()
	{
		if (m_bForceUpdate)
			UpdatePopup(false);

		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			if (!m_itemList.Empty())
			{
				MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
				MashGUISkin *inactiveSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_INACTIVE);
				MashGUISkin *activeSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_ITEM_ACTIVE);
				MashGUISkin *backgroundSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND);

				mash::MashVertexColour::sMashVertexColour subMenuIndicatorVerts[3];
				const f32 subMenuIndicatorHeight = activeStyle->GetFont()->GetMaxCharacterHeight() * 0.5f;

				MashGUISkin *skinToUse = 0;
				const uint32 iItemCount = m_itemList.Size();
				for(uint32 i = 0; i < iItemCount; ++i)
				{
					//draw buttons
					if (m_itemList[i]->bHasFocus)
						skinToUse = activeSkin;
					else
						skinToUse = inactiveSkin;

					m_GUIManager->DrawSprite(m_itemList[i]->absoluteRect, this->GetAbsoluteClippedRegion(), skinToUse, m_overrideTransparency);

					//draw text
					m_itemList[i]->textHandler.Draw(m_GUIManager, skinToUse->fontColour, m_overrideTransparency);
				}

				//need to flush the buffers so the tris get rendered on top
				m_GUIManager->FlushBuffers();

				for(uint32 i = 0; i < iItemCount; ++i)
				{
					//draw indicator to show this item has a submenu
					if (m_itemList[i]->pSubMenu)
					{
						if (m_itemList[i]->bHasFocus)
							skinToUse = activeSkin;
						else
							skinToUse = inactiveSkin;

						sMashColour fontColour = skinToUse->fontColour;
						if (m_overrideTransparency.enableOverrideTransparency && m_overrideTransparency.affectFontAlpha)
							fontColour = m_overrideTransparency.alphaValue;

						mash::MashRectangle2 indicatorRect(-subMenuIndicatorHeight * 0.5f, subMenuIndicatorHeight * 0.5f, subMenuIndicatorHeight * 0.5f, -subMenuIndicatorHeight * 0.5f);
						f32 moveToPosition = m_itemList[i]->absoluteRect.right - subMenuIndicatorHeight - m_itemHorizontalBuffer;;
						indicatorRect.left += moveToPosition;
						indicatorRect.right += moveToPosition;
						indicatorRect.top += m_itemList[i]->absoluteRect.top;
						indicatorRect.bottom += m_itemList[i]->absoluteRect.bottom;

						subMenuIndicatorVerts[0].position = mash::MashVector3(indicatorRect.left, indicatorRect.top, 0.0f);
						subMenuIndicatorVerts[2].position = mash::MashVector3(indicatorRect.left, indicatorRect.bottom, 0.0f);
						subMenuIndicatorVerts[1].position = mash::MashVector3(indicatorRect.right, indicatorRect.top + (subMenuIndicatorHeight * 0.5f), 0.0f);

						subMenuIndicatorVerts[0].colour = fontColour;
						subMenuIndicatorVerts[1].colour = fontColour;
						subMenuIndicatorVerts[2].colour = fontColour;

						m_GUIManager->DrawSolidTriangles(subMenuIndicatorVerts, 3);
					}
				}

				m_GUIManager->FlushBuffers();

				if (backgroundSkin->renderBoarder)
				{
					sMashColour borderColour = backgroundSkin->borderColour;
					if (m_overrideTransparency.enableOverrideTransparency)
						borderColour.SetAlpha(m_overrideTransparency.alphaValue);

					m_GUIManager->DrawBorder(m_absoluteClippedRegion, borderColour);
				}
			}
		}
	}

	void CMashGUIPopupMenu::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if (eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE)
			{
				switch(eventData.action)
				{
				case aMOUSEEVENT_B1:
					{
						if ((m_focusedItem != -1) && (eventData.isPressed == 1))
						{
							sItem *focusedItem = GetItem(m_focusedItem);

							/*
								open the submenu if it is not already open
							*/
							if (focusedItem->pSubMenu && !focusedItem->pSubMenu->GetRenderEnabled())
							{
								mash::MashVector2 vStartPos(focusedItem->absoluteRect.right, focusedItem->absoluteRect.top);
								focusedItem->pSubMenu->Activate(vStartPos);
							}
							/*
								if there is no submenu then we post an event
								and shut down the popup
							*/
							else if (!focusedItem->pSubMenu)
							{
								sGUIEvent newGUIMsg;
								
								newGUIMsg.GUIEvent = aGUIEVENT_POPUP_SELECTION;
								newGUIMsg.component = this;
								ImmediateBroadcast(newGUIMsg);

								m_GUIManager->SetFocusedElement(0);
							}
						}

						break;
					};
				case aMOUSEEVENT_AXISX:
				case aMOUSEEVENT_AXISY:
					{
						if (m_hasFocus)
						{
							mash::MashVector2 vMousePos = m_inputManager->GetCursorPosition();

							const uint32 iItemSize = m_itemList.Size();
							for(uint32 i = 0; i < iItemSize; ++i)
							{
								if (m_itemList[i]->absoluteRect.IntersectsGUI(vMousePos))
								{
									if ((m_focusedItem == -1) || (m_focusedItem != m_itemList[i]->id))
									{
										sItem *focusedItem = GetItem(m_focusedItem);
										/*
											If the focused item had a submenu open then remove
											its focus so that it deactivates, and input is restored
											to the parent(this) object
										*/
										if (focusedItem && focusedItem->pSubMenu && focusedItem->pSubMenu->GetHasFocus())
										{
											m_GUIManager->SetFocusedElement(this);
										}

										if (focusedItem)
											focusedItem->bHasFocus = false;

										m_focusedItem = m_itemList[i]->id;
										focusedItem = m_itemList[i];
										focusedItem->bHasFocus = true;

										m_lastFocusedItem = m_focusedItem;
									}

									break;
								}
							}
							
						}
						break;
					};
				}
			}

			/*
				Pass messages onto the popup that opened this submenu
			*/
			if (m_pPopupOwner)
				m_pPopupOwner->OnEvent(eventData);
		}
	}
}