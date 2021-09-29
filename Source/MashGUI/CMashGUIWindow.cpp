//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIWindow.h"
#include "MashGUIManager.h"
#include "MashGUIView.h"

namespace mash
{
	CMashGUIWindow::CMashGUIWindow(MashGUIManager *pGUIManager,
		MashInputManager *pInputManager,
		MashGUIComponent *pParent,
		const MashGUIRect &destination,
		int32 styleElement):MashGUIWindow(pGUIManager, pInputManager, pParent, destination),
		m_menuBarHeight(0.0f), m_titleBarCulled(false), m_view(0), m_styleElement(styleElement),
		m_activateCloseButton(false), m_activateMinimizeButton(false), m_minimizeButtonCulled(false), m_closeButtonCulled(false),
		m_buttonState(aSTATE_UP), m_hoverButton(aWIN_BTN_NONE), m_isMinimized(false), m_mouseDragEnabled(false), m_allowMouseDrag(true),
		m_closeButtonEvent(aCLOSE_AND_DESTROY)
	{
		m_menuBarHeight = 10.0f;

		MashGUIStyle *activeStyle = pGUIManager->GetActiveGUIStyle();
		if (activeStyle)
		{
			m_textHandler.SetFormat(activeStyle->GetFont(), MashGUIFont::aLEFT_CENTER, false);
			m_menuBarHeight = activeStyle->GetFont()->GetMaxCharacterHeight();
		}

		//make the menu bar a little bigger than the text
		m_menuBarHeight += 10.0f;

		m_menuBarHeight = math::Max<f32>(25.0f, m_menuBarHeight);
		m_buttonHeight = 20.0f;

		mash::MashGUIRect viewRect;
		viewRect.right.scale = 1.0f;
		viewRect.bottom.scale = 1.0f;
		viewRect.top.offset += m_menuBarHeight;

		m_view = pGUIManager->_GetGUIFactory()->CreateView(viewRect, this);

		UpdateTitleBarRegion();
		UpdateCloseButtonRegion();
		UpdateMinimizeButtonRegion();
	}

	CMashGUIWindow::~CMashGUIWindow()
	{
		m_view->Drop();
	}

	void CMashGUIWindow::SetLockFocusWhenActivated(bool lockFocus)
	{
		_SetLockFocusWhenActivated(lockFocus);
	}

	void CMashGUIWindow::SetAlwaysOnTop(bool enable)
	{
		_SetAlwaysOnTop(enable);
	}

	void CMashGUIWindow::EnableCloseButton(bool enable)
	{
		if (m_activateCloseButton != enable)
		{
			m_activateCloseButton = enable;
			UpdateCloseButtonRegion();
		}
	}

	void CMashGUIWindow::EnableMinimizeButton(bool enable)
	{
		if (m_activateMinimizeButton != enable)
		{
			m_activateMinimizeButton = enable;
			UpdateMinimizeButtonRegion();
		}
	}

	void CMashGUIWindow::OnStyleChange(MashGUIStyle *style)
	{
		m_textHandler.SetFormat(style->GetFont(), MashGUIFont::aCENTER, false);
		m_view->OnStyleChange(style);
	}

	void CMashGUIWindow::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		MashGUIComponent::SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		m_view->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
	}

	MashGUIComponent* CMashGUIWindow::GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren)
	{
		if (bTestAllChildren || (GetRenderEnabled() && GetCanHaveFocus() && (m_cullState != aCULL_CULLED)))
		{
			if (m_isMinimized)
			{
				if (m_titleBarClippedAbs.IntersectsGUI(vScreenPos))
					return this;
				else
					return 0;
			}

			MashGUIComponent *intersects =  m_view->GetClosestIntersectingChild(vScreenPos, bTestAllChildren);
			if (intersects)
				return intersects;

			intersects =  MashGUIComponent::GetClosestIntersectingChild(vScreenPos, bTestAllChildren);
			if (intersects)
				return intersects;
		}

		return 0;
	}

	MashGUIView* CMashGUIWindow::GetClosestParentableObject(const mash::MashVector2 &screenPosition)
	{
		if (GetRenderEnabled() && GetCanHaveFocus())
		{
			if (m_isMinimized)
			{
				return 0;
			}

			MashGUIView *intersects =  m_view->GetClosestParentableObject(screenPosition);
			if (intersects)
				return intersects;

			intersects =  MashGUIComponent::GetClosestParentableObject(screenPosition);
			if (intersects)
				return intersects;
		}

		return 0;
	}

	void CMashGUIWindow::AddChild(MashGUIComponent *pChild)
	{
		m_view->AddChild(pChild);
	}

	void CMashGUIWindow::SetTitleBarText(const MashStringc &text)
	{
		m_textHandler.SetString(text);
	}

	MashGUIComponent* CMashGUIWindow::GetElementByName(const MashStringc &name, bool searchChildren)
	{
		MashGUIComponent *element = MashGUIComponent::GetElementByName(name, searchChildren);
		if (element)
			return element;

		return m_view->GetElementByName(name, searchChildren);
	}

	void CMashGUIWindow::OnMouseExit(const mash::MashVector2 &vScreenPos)
	{
		if (m_buttonState != aSTATE_DOWN)
		{
			m_buttonState = aSTATE_UP;
			m_hoverButton = aWIN_BTN_NONE;
		}
	}

	void CMashGUIWindow::OnLostFocus()
	{
		if (m_buttonState != aSTATE_DOWN)
		{
			m_buttonState = aSTATE_UP;
			m_hoverButton = aWIN_BTN_NONE;
		}
	}

	void CMashGUIWindow::UpdateCloseButtonRegion()
	{
		m_closeButtonAbs.top = m_titleBarAbs.top - 1;
		m_closeButtonAbs.right = m_titleBarAbs.right - 1;
		m_closeButtonAbs.left = m_closeButtonAbs.right - m_buttonHeight;
		m_closeButtonAbs.bottom = m_titleBarAbs.bottom - 1;

		m_closeButtonClippedAbs = m_closeButtonAbs;
		if (m_closeButtonClippedAbs.ClipGUI(m_absoluteClippedRegion) == aCULL_CULLED)
			m_closeButtonCulled = true;
		else
			m_closeButtonCulled = false;
	}

	void CMashGUIWindow::UpdateMinimizeButtonRegion()
	{
		m_minimizeButtonAbs.top = m_titleBarAbs.top - 1;
		m_minimizeButtonAbs.right = m_closeButtonAbs.left - 1;
		m_minimizeButtonAbs.left = m_minimizeButtonAbs.right - m_buttonHeight;
		m_minimizeButtonAbs.bottom = m_titleBarAbs.bottom - 1;

		m_minimizeButtonClippedAbs = m_minimizeButtonAbs;
		if (m_minimizeButtonClippedAbs.ClipGUI(m_absoluteClippedRegion) == aCULL_CULLED)
			m_minimizeButtonCulled = true;
		else
			m_minimizeButtonCulled = false;
	}

	void CMashGUIWindow::UpdateTitleBarRegion()
	{
		m_titleBarAbs = m_absoluteRegion;
		m_titleBarAbs.bottom = m_titleBarAbs.top + m_menuBarHeight;

		m_titleBarClippedAbs = m_titleBarAbs;
		if (m_titleBarClippedAbs.ClipGUI(m_absoluteClippedRegion) == aCULL_CULLED)
			m_titleBarCulled = true;
		else
			m_titleBarCulled = false;

		m_textHandler.SetRegion(m_titleBarAbs, m_titleBarClippedAbs);
	}

	void CMashGUIWindow::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		m_view->UpdateRegion();

		if (positionChangeOnly)
		{
			m_closeButtonAbs.left += deltaX;
			m_closeButtonAbs.right += deltaX;
			m_closeButtonAbs.top += deltaY;
			m_closeButtonAbs.bottom += deltaY;

			m_closeButtonClippedAbs.left += deltaX;
			m_closeButtonClippedAbs.right += deltaX;
			m_closeButtonClippedAbs.top += deltaY;
			m_closeButtonClippedAbs.bottom += deltaY;

			m_titleBarAbs.left += deltaX;
			m_titleBarAbs.right += deltaX;
			m_titleBarAbs.top += deltaY;
			m_titleBarAbs.bottom += deltaY;

			m_titleBarClippedAbs.left += deltaX;
			m_titleBarClippedAbs.right += deltaX;
			m_titleBarClippedAbs.top += deltaY;
			m_titleBarClippedAbs.bottom += deltaY;

			m_minimizeButtonAbs.left += deltaX;
			m_minimizeButtonAbs.right += deltaX;
			m_minimizeButtonAbs.top += deltaY;
			m_minimizeButtonAbs.bottom += deltaY;

			m_minimizeButtonClippedAbs.left += deltaX;
			m_minimizeButtonClippedAbs.right += deltaX;
			m_minimizeButtonClippedAbs.top += deltaY;
			m_minimizeButtonClippedAbs.bottom += deltaY;

			m_textHandler.AddPosition(deltaX, deltaY);
		}
		else
		{
			UpdateTitleBarRegion();
			UpdateCloseButtonRegion();
			UpdateMinimizeButtonRegion();
		}
	}
    
    void CMashGUIWindow::SetMinimizeState(bool enable)
    {
        if (m_isMinimized)
        {
            //SetDestinationRegion(m_destinationBeforeMinimize);
            m_isMinimized = false;
            m_view->SetRenderEnabled(true);
            
            sGUIEvent newGUIMsg;
            
            newGUIMsg.GUIEvent = aGUIEVENT_WINDOW_MAXIMIZE;
            newGUIMsg.component = this;
            ImmediateBroadcast(newGUIMsg);
        }
        else
        {
            m_isMinimized = true;
            m_view->SetRenderEnabled(false);
            
            sGUIEvent newGUIMsg;
            
            newGUIMsg.GUIEvent = aGUIEVENT_WINDOW_MINIMIZE;
            newGUIMsg.component = this;
            ImmediateBroadcast(newGUIMsg);
        }
    }
    
    void CMashGUIWindow::CloseWindow()
    {
        if (GetRenderEnabled())
        {
            sGUIEvent newGUIMsg;
            
            newGUIMsg.GUIEvent = aGUIEVENT_WINDOW_CLOSE;
            newGUIMsg.component = this;
            ImmediateBroadcast(newGUIMsg);

            if (m_closeButtonEvent == aCLOSE_AND_DESTROY)
                this->Destroy();
            else
                SetRenderEnabled(false);
        }
    }

	void CMashGUIWindow::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			switch(eventData.action)
			{
			case aMOUSEEVENT_B1:
				{
					switch(eventData.isPressed)
					{
					case 1:
						{
							if (m_hoverButton != aWIN_BTN_NONE)
							{
								m_buttonState = aSTATE_DOWN;
							}
							else if (m_allowMouseDrag)
							{
								mash::MashVector2 cursorPosition = m_inputManager->GetCursorPosition();
								if (m_titleBarClippedAbs.IntersectsGUI(cursorPosition))
									m_mouseDragEnabled = true;
							}

							break;
						}
					case 0:
						{
							m_mouseDragEnabled = false;

							if ((m_hoverButton != aWIN_BTN_NONE) && (m_buttonState == aSTATE_DOWN))
							{
								if (m_hoverButton == aWIN_BTN_CLOSE)
								{
									CloseWindow();
								}
								else
								{
                                    SetMinimizeState(m_isMinimized?false:true);
								}

								m_buttonState = aSTATE_UP;
							}
							/*
								Only register an a button up message if the mouse is
								still hovering over the button. Otherwise it means the user
								wishes to cancel the selection.
							*/
							else if (m_buttonState == aSTATE_DOWN)
							{
								m_buttonState = aSTATE_UP;
							}
							break;
						}

					};
					break;
				}
			case aMOUSEEVENT_AXISX:
			case aMOUSEEVENT_AXISY:
				{
					if (!m_mouseDragEnabled)
					{
						mash::MashVector2 cursorPosition = m_inputManager->GetCursorPosition();
						if (m_activateCloseButton && m_closeButtonClippedAbs.IntersectsGUI(cursorPosition))
							m_hoverButton = aWIN_BTN_CLOSE;
						else if (m_activateMinimizeButton && m_minimizeButtonClippedAbs.IntersectsGUI(cursorPosition))
							m_hoverButton = aWIN_BTN_MINIMIZE;
						else
							m_hoverButton = aWIN_BTN_NONE;
					}
					else
					{
						if (eventData.action == aMOUSEEVENT_AXISX)
							AddPosition(eventData.value, 0.0f);
						else
							AddPosition(0.0f, eventData.value);
					}

					break;
				}
			};
		}
	}

	void CMashGUIWindow::Draw()
	{
		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();
			m_view->Draw();

			if (m_titleBarCulled)
			{
				MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
				MashGUISkin *titleBarSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_TITLEBAR);
				m_GUIManager->DrawSprite(m_titleBarAbs, m_titleBarClippedAbs, titleBarSkin, m_overrideTransparency);

				m_textHandler.Draw(m_GUIManager, titleBarSkin->fontColour, m_overrideTransparency);

				if (m_activateCloseButton && !m_closeButtonCulled)
				{
					eGUI_STYLE_ATTRIBUTE style;

					if (m_hoverButton == aWIN_BTN_CLOSE && m_buttonState == aSTATE_DOWN)
						style = aGUI_ATTRIB_WINDOW_CLOSE_DOWN;
					else if (m_hoverButton == aWIN_BTN_CLOSE && m_buttonState == aSTATE_UP)
						style = aGUI_ATTRIB_WINDOW_CLOSE_HOVER;
					else
						style = aGUI_ATTRIB_WINDOW_CLOSE_UP;

					MashGUISkin *closeButtonSkin = activeStyle->GetAttributeSkin(m_styleElement, style);
					m_GUIManager->DrawSprite(m_closeButtonAbs, m_closeButtonClippedAbs, closeButtonSkin, m_overrideTransparency);
				}

				if (m_activateMinimizeButton && !m_minimizeButtonCulled)
				{
					eGUI_STYLE_ATTRIBUTE style;
					if (m_hoverButton == aWIN_BTN_MINIMIZE && m_buttonState == aSTATE_DOWN)
						style = aGUI_ATTRIB_WINDOW_MINIMIZE_DOWN;
					else if (m_hoverButton == aWIN_BTN_MINIMIZE && m_buttonState == aSTATE_UP)
						style = aGUI_ATTRIB_WINDOW_MINIMIZE_HOVER;
					else
						style = aGUI_ATTRIB_WINDOW_MINIMIZE_UP;

					MashGUISkin *minimizeButtonSkin = activeStyle->GetAttributeSkin(m_styleElement, style);
					m_GUIManager->DrawSprite(m_minimizeButtonAbs, m_minimizeButtonClippedAbs, minimizeButtonSkin, m_overrideTransparency);
				}

				/*
					Buffers need to be flushed at this point to avoid overlaps.
				*/
				m_GUIManager->FlushBuffers();
			}
		}
	}


}