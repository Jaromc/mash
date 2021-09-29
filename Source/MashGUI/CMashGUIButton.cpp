//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIButton.h"
#include "MashGUIManager.h"
namespace mash
{
	CMashGUIButton::CMashGUIButton(MashGUIManager *pGUIManager,
		MashInputManager *pInputManager,
		MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUIButton(pGUIManager, pInputManager, pParent, destination), m_buttonState(aSTATE_UP),
			m_styleElement(styleElement), m_switchIsPressed(false), m_isSwitch(false), m_upStyle(aGUI_ATTRIB_BUTTON_UP), 
			m_downStyle(aGUI_ATTRIB_BUTTON_DOWN), m_hoverStyle(aGUI_ATTRIB_BUTTON_HOVER)
	{
		MashGUIStyle *style = pGUIManager->GetActiveGUIStyle();
		if (style)
		{
			m_textHandler.SetFormat(style->GetFont(), MashGUIFont::aCENTER, false);
		}
		m_textHandler.SetRegion(m_absoluteRegion, m_absoluteClippedRegion);
	}

	CMashGUIButton::~CMashGUIButton()
	{
		
	}

	void CMashGUIButton::OnStyleChange(MashGUIStyle *style)
	{
		m_textHandler.SetFormat(style->GetFont(), MashGUIFont::aCENTER, false);
	}

	void CMashGUIButton::SetSwitchState(bool isPressed)
	{
		if (m_isSwitch)
		{
			if (m_switchIsPressed != isPressed)
			{
				m_switchIsPressed = isPressed;

				if (m_switchIsPressed)
				{
					sGUIEvent newGUIMsg;
					
					newGUIMsg.GUIEvent = aGUIEVENT_BTN_DOWN;
					newGUIMsg.component = this;
					ImmediateBroadcast(newGUIMsg);
				}
				else
				{
					sGUIEvent newGUIMsg;
					
					newGUIMsg.GUIEvent = aGUIEVENT_BTN_UP_CONFIRM;
					newGUIMsg.component = this;
					ImmediateBroadcast(newGUIMsg);
				}
			}
		}
	}

	void CMashGUIButton::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if (eventData.action == aMOUSEEVENT_B1)
			{
				switch(eventData.isPressed)
				{
				case 1:
					{
						if ((m_mouseHover && (m_buttonState != aSTATE_DOWN)))
						{
							/*
								switches only register on button up events
							*/
							if (!m_isSwitch)
							{
								m_switchIsPressed = true;

								sGUIEvent newGUIMsg;
								
								newGUIMsg.GUIEvent = aGUIEVENT_BTN_DOWN;
								newGUIMsg.component = this;
								ImmediateBroadcast(newGUIMsg);
							}

							m_buttonState = aSTATE_DOWN;
						}
						
						break;
					}
				case 0:
					{
						if (m_isSwitch)
						{
							if (m_mouseHover && (m_buttonState == aSTATE_DOWN))
							{
								if (m_switchIsPressed)
								{
									m_switchIsPressed = false;

									sGUIEvent newGUIMsg;
									
									newGUIMsg.GUIEvent = aGUIEVENT_BTN_UP_CONFIRM;
									newGUIMsg.component = this;
									ImmediateBroadcast(newGUIMsg);

									m_buttonState = aSTATE_UP;
								}
								else
								{
									m_switchIsPressed = true;

									sGUIEvent newGUIMsg;
									
									newGUIMsg.GUIEvent = aGUIEVENT_BTN_DOWN;
									newGUIMsg.component = this;
									ImmediateBroadcast(newGUIMsg);
								}
							}
							/*
								Only register an a button  message if the mouse is
								still hovering over the button. Otherwise it means the user
								wishes to cancel the selection.
							*/
							else if (m_buttonState == aSTATE_DOWN)
							{
								if (m_switchIsPressed)
									m_buttonState = aSTATE_DOWN;
								else
									m_buttonState = aSTATE_UP;
							}
						}
						else
						{
							if ((m_mouseHover && (m_buttonState == aSTATE_DOWN)))
							{
								sGUIEvent newGUIMsg;
								m_switchIsPressed = false;

								
								newGUIMsg.GUIEvent = aGUIEVENT_BTN_UP_CONFIRM;
								newGUIMsg.component = this;
								ImmediateBroadcast(newGUIMsg);

								m_buttonState = aSTATE_UP;
							}
							else if (m_buttonState == aSTATE_DOWN)
							{
								m_buttonState = aSTATE_UP;
								m_switchIsPressed = false;

								sGUIEvent newGUIMsg;
								
								newGUIMsg.GUIEvent = aGUIEVENT_BTN_UP_CANCEL;
								newGUIMsg.component = this;
								ImmediateBroadcast(newGUIMsg);
							}
						}
						
						break;
					}

				};
			}
		}
	}

	void CMashGUIButton::SetText(const MashStringc &text)
	{
		m_textHandler.SetString(text);
	}

	void CMashGUIButton::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
			m_textHandler.AddPosition(deltaX, deltaY);
		else
			m_textHandler.SetRegion(m_absoluteRegion, m_absoluteClippedRegion);
	}

	void CMashGUIButton::SetButtonStyles(int32 elementId, eGUI_STYLE_ATTRIBUTE up, eGUI_STYLE_ATTRIBUTE down, eGUI_STYLE_ATTRIBUTE hover)
	{
		m_styleElement = elementId;
		m_upStyle = up;
		m_downStyle = down;
		m_hoverStyle = hover;
	}

	void CMashGUIButton::Draw()
	{
		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *activeSkin = 0;
			if (m_buttonState == aSTATE_UP)
				activeSkin = activeStyle->GetAttributeSkin(m_styleElement, m_upStyle);
			else if (m_buttonState == aSTATE_DOWN)
				activeSkin = activeStyle->GetAttributeSkin(m_styleElement, m_downStyle);
			else
				activeSkin = activeStyle->GetAttributeSkin(m_styleElement, m_hoverStyle);

			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, activeSkin, m_overrideTransparency);

			m_textHandler.Draw(m_GUIManager, activeSkin->fontColour, m_overrideTransparency);
		}
	}
}