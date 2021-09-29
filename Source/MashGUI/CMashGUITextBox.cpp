//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUITextBox.h"
#include "MashGUIManager.h"
#include "MashDevice.h"
#include "MashTimer.h"
#include "MashHelper.h"
#include "MashGUIFont.h"
#include <cctype>

namespace mash
{
	const f32 g_mashGUIButtonWidth = 10.0f;
	const f32 g_mashGUIButtonTextBuffer = 5.0f;

	CMashGUITextBox::CMashGUITextBox(MashGUIManager *pGUIManager,
		MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUITextBox(pGUIManager, pInputManager, pParent, destination), m_vCursorPos(0.0f, 0.0f), m_styleElement(styleElement),
			m_acceptReturn(true), m_cursorBlinkTime(500), m_accumCursorBlinkTimer(0), m_lastCursorBlinkTime(0), m_drawCarret(true),
			m_textFormat(aGUI_TEXT_FORMAT_ANY), m_backspaceKeyTimer(0), m_backspaceKeyDelay(50), m_backspaceKeyLastTime(0) ,
			m_integerButtonsEnabled(false), m_incrementButton(0), m_decrementButton(0), m_floatPrecision(3),
			m_numberKeyDelay(50), m_numberKeyLastTime(0), m_numberKeyTimer(0), m_numberKeyHeld(false),
    m_numberMinVal(mash::math::MinInt32()), m_numberMaxVal(mash::math::MaxInt32()), m_repearCharEnabled(false)
	{
		m_textHandler.EnableCarat(true);

		MashGUIStyle *style = pGUIManager->GetActiveGUIStyle();
		if (style)
		{
			m_textHandler.SetFormat(style->GetFont(), MashGUIFont::aLEFT_CENTER, false);
		}
		m_textHandler.SetRegion(m_absoluteRegion, m_absoluteClippedRegion);
	}

	CMashGUITextBox::~CMashGUITextBox()
	{
		if (m_incrementButton)
		{
			m_incrementButton->Destroy();
			m_incrementButton = 0;
		}

		if (m_decrementButton)
		{
			m_decrementButton->Destroy();
			m_decrementButton = 0;
		}
	}

	void CMashGUITextBox::SetNumberMinMax(f32 min, f32 max)
	{
		//validate
		m_numberMinVal = min;
		m_numberMaxVal = max;

		if (m_numberMinVal > m_numberMaxVal)
		{
			f32 temp = m_numberMaxVal;
			m_numberMaxVal = m_numberMinVal;
			m_numberMinVal = temp;
		}

		/*
			Note that floats are clamped to int32 range so that so that everything
			plays nicely
		*/
		m_numberMinVal = math::Clamp<f32>((f32)mash::math::MinInt32(), (f32)mash::math::MaxInt32(), m_numberMinVal);
		m_numberMaxVal = math::Clamp<f32>((f32)mash::math::MinInt32(), (f32)mash::math::MaxInt32(), m_numberMaxVal);
	}

	void CMashGUITextBox::OnStyleChange(MashGUIStyle *style)
	{
		m_textHandler.SetFormat(style->GetFont(), MashGUIFont::aTOP_LEFT, false);

		if (m_incrementButton)
			m_incrementButton->OnStyleChange(style);
		if (m_decrementButton)
			m_decrementButton->OnStyleChange(style);
	}

	int32 CMashGUITextBox::GetTextAsInt()
	{
		return atoi(m_textHandler.GetString().GetCString());
	}

	f32 CMashGUITextBox::GetTextAsFloat()
	{
		return (f32)atof(m_textHandler.GetString().GetCString());
	}

	void CMashGUITextBox::OnTextConfirmed(const MashStringc &text)
	{
		//if this string has already been confirmed then dont do anything
		if (m_lastConfirmedString == text)
			return;

		m_textHandler.SetString(text);

		SetCursorPos(m_textHandler.GetString().Size());

		m_lastConfirmedString = m_textHandler.GetString();

		//send messages only on success
		sGUIEvent newGUIMsg;
		
		newGUIMsg.GUIEvent = aGUIEVENT_TB_CONFIRM;
		newGUIMsg.component = this;
		ImmediateBroadcast(newGUIMsg);
	}

	void CMashGUITextBox::SetTextInt(int32 num)
	{
		num = (int32)math::Clamp<f32>(m_numberMinVal, m_numberMaxVal, (f32)num);

		int8 buffer[256];
		mash::helpers::PrintToBuffer(buffer, 256, "%d", num);
		
		OnTextConfirmed(buffer);
	}

	void CMashGUITextBox::SetWordWrap(bool bEnable)
	{
		m_textHandler.SetWordWrap(bEnable);
	}

	void CMashGUITextBox::SetTextFloat(f32 num)
	{
		num = math::Clamp<f32>(m_numberMinVal, m_numberMaxVal, num);
		int8 buffer[256];
		mash::helpers::PrintToBuffer(buffer, 256, "%.*f", m_floatPrecision, num);

		OnTextConfirmed(buffer);
	}

	void CMashGUITextBox::SetText(const MashStringc &text)
	{
		//if this string has already been confirmed then dont do anything
		if (m_lastConfirmedString == text)
			return;

		m_textHandler.SetString(text);
		
		ValidateString();
		m_textHandler.SendCaratToFront();
	}

	void CMashGUITextBox::AddString(const MashStringc &text)
	{
		m_textHandler.AddString(text);
		SetCursorPos(m_textHandler.GetString().Size());
	}

	void CMashGUITextBox::AddCharacter(int8 c)
	{
		m_textHandler.AddCharacter(c);
		SetCursorPos(m_textHandler.GetString().Size());
	}

	void CMashGUITextBox::InsertString(uint32 iLocation, const int8 *string)
	{
		const uint32 iSize = m_textHandler.GetString().Size();
		if ((iLocation >= 0) && (iLocation <= iSize))
		{
			m_textHandler.InsertString(iLocation, string);
			SetCursorPos(iLocation);
		}
	}

	void CMashGUITextBox::InsertCharacter(uint32 iLocation, int8 newChar)
	{
		const uint32 iSize = m_textHandler.GetString().Size();
		if ((iLocation >= 0) && (iLocation <= iSize))
		{
			m_textHandler.InsertCharacter(iLocation, newChar);
			SetCursorPos(iLocation);
		}
	}

	void CMashGUITextBox::SetCursorPos(uint32 iIndex)
	{
	}

	void CMashGUITextBox::RemoveCharacters(uint32 iCount)
	{
		m_textHandler.RemoveCharacters(iCount);
		SetCursorPos(m_textHandler.GetString().Size());
	}

	void CMashGUITextBox::SetTextColour(const sMashColour &colour)
	{
		m_backgroundSkin.fontColour = colour;
	}

	void CMashGUITextBox::OnFocusGained()
	{
		m_textHandler.SendCaratToBack();
	}

	void CMashGUITextBox::OnLostFocus()
	{
		m_repearCharEnabled = false;

		m_textHandler.SendCaratToFront();

		//only check when a change has occured
		if (m_lastConfirmedString == m_textHandler.GetString())
			return;

		ValidateString();
	}

	void CMashGUITextBox::ValidateString()
	{		
		bool valid = true;
		if (m_textFormat == aGUI_TEXT_FORMAT_INT)
		{
			const int8 *text = m_textHandler.GetString().GetCString();
			const uint32 textSize = m_textHandler.GetString().Size();
			for(uint32 i = 0; i < textSize; ++i)
			{
				if (!((text[i] == '-') || isdigit(text[i])))
				{
					valid = false;
					break;
				}
			}

			if (valid)
			{
				SetTextInt(atoi(m_textHandler.GetString().GetCString()));
			}
		}
		else if (m_textFormat == aGUI_TEXT_FORMAT_FLOAT)
		{
			bool decimalFound = false;
			const int8 *text = m_textHandler.GetString().GetCString();
			const uint32 textSize = m_textHandler.GetString().Size();
			for(uint32 i = 0; i < textSize; ++i)
			{
				if ((i == 0) && (text[i] == '-'))
					continue;
				else if (isdigit(text[i]))
					continue;
				else if (text[i] == '.' && !decimalFound)
				{
					decimalFound = true;
				}
				else
				{
					valid = false;
					break;	
				}
			}

			if (valid)
			{
				SetTextFloat(atof(m_textHandler.GetString().GetCString()));
			}
		}
		else
		{
			OnTextConfirmed(m_textHandler.GetString());
		}

		if (!valid)
		{
			m_textHandler.SetString(m_lastConfirmedString);
			SetCursorPos(m_textHandler.GetString().Size());
			return;
		}
	}

	void CMashGUITextBox::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if (eventData.eventType == sInputEvent::aEVENTTYPE_KEYBOARD)
			{
				if (m_hasFocus)
				{
					if (eventData.isPressed == 1)
					{
						if (m_acceptReturn && (eventData.action == aKEYEVENT_RETURN))
						{
							//only check when a change has occured
							if (m_lastConfirmedString != m_textHandler.GetString())
								ValidateString();
							
							//remove focus so it stops recieving text input
							m_GUIManager->SetFocusedElement(0);
						}
						else
						{
							int32 newChar = eventData.character;

							MashStringc textBefore = m_textHandler.GetString();

							bool startRepeat = true;

							//backspace
							if (eventData.action == aKEYEVENT_BACKSPACE)
							{
								RemoveCharacters(1);
							}
							else if (eventData.action == aKEYEVENT_LEFT)
							{
								m_textHandler.MoveCaratLeft();
							}
							else if (eventData.action == aKEYEVENT_RIGHT)
							{
								m_textHandler.MoveCaratRight();
							}
							else if (isdigit(newChar) || 
								isalpha(newChar) ||
								ispunct(newChar) ||
								isspace(newChar))
							{
								AddCharacter(newChar);
							}
							else
								startRepeat = false;

							if (startRepeat && !m_repearCharEnabled)
							{
								m_repeatChar = newChar;
								m_repeatEventType = eventData.action;
								m_repearCharEnabled = true;
								m_backspaceKeyTimer = -m_backspaceKeyDelay * 5.0f;
								m_backspaceKeyLastTime = MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart();
							}

							if (textBefore != m_textHandler.GetString())
							{
								//send messages
								sGUIEvent newGUIMsg;
								
								newGUIMsg.GUIEvent = aGUIEVENT_TB_TEXT_CHANGE;
								newGUIMsg.component = this;
								ImmediateBroadcast(newGUIMsg);
							}
						}
					}
					else
					{
						m_repearCharEnabled = false;
					}
				}
			}
		}
	}

	void CMashGUITextBox::SetFloatPrecision(uint32 precision)
	{
		m_floatPrecision = math::Clamp<uint32>(1, 20, precision);

		if (m_textFormat == aGUI_TEXT_FORMAT_FLOAT)
		{
			f32 currentNum = GetTextAsFloat();
			SetTextFloat(currentNum);
		}
	}

	void CMashGUITextBox::SetNumberButtonState(bool enable)
	{
		if (m_integerButtonsEnabled != enable)
		{
			m_integerButtonsEnabled = enable;

			if (m_incrementButton)
			{
				m_incrementButton->Destroy();
				m_incrementButton = 0;
			}

			if (m_decrementButton)
			{
				m_decrementButton->Destroy();
				m_decrementButton = 0;
			}

			if (m_integerButtonsEnabled)
			{
				MashGUIRect incrementButtonRegion(MashGUIUnit(1.0f, -g_mashGUIButtonWidth), MashGUIUnit(0.0f, 0.0f), MashGUIUnit(1.0f, 0.0f), MashGUIUnit(0.5f, 0.0f));
				MashGUIRect decrementButtonRegion(MashGUIUnit(1.0f, -g_mashGUIButtonWidth), MashGUIUnit(0.5f, 0.0f), MashGUIUnit(1.0f, 0.0f), MashGUIUnit(1.0f, 0.0f));
				m_incrementButton = m_GUIManager->_GetGUIFactory()->CreateButton(incrementButtonRegion, this);
				m_decrementButton = m_GUIManager->_GetGUIFactory()->CreateButton(decrementButtonRegion, this);

				m_incrementButton->SetEventsEnabled(GetEventsEnabled());
				m_decrementButton->SetEventsEnabled(GetEventsEnabled());

				m_incrementButton->SetButtonStyles(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWUP_UP, aGUI_ATTRIB_SCROLLBAR_ARROWUP_DOWN, aGUI_ATTRIB_SCROLLBAR_ARROWUP_HOVER);
				m_decrementButton->SetButtonStyles(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_UP, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_DOWN, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_HOVER);

				m_incrementButton->RegisterReceiver(aGUIEVENT_BTN_DOWN, MashGUIEventFunctor(&CMashGUITextBox::OnIntegerButtonIncrementChange, this));
				m_incrementButton->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&CMashGUITextBox::OnIntegerButtonIncrementChange, this));
				m_incrementButton->RegisterReceiver(aGUIEVENT_BTN_UP_CANCEL, MashGUIEventFunctor(&CMashGUITextBox::OnIntegerButtonIncrementChange, this));
				m_decrementButton->RegisterReceiver(aGUIEVENT_BTN_DOWN, MashGUIEventFunctor(&CMashGUITextBox::OnIntegerButtonDecrementChange, this));
				m_decrementButton->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&CMashGUITextBox::OnIntegerButtonDecrementChange, this));
				m_decrementButton->RegisterReceiver(aGUIEVENT_BTN_UP_CANCEL, MashGUIEventFunctor(&CMashGUITextBox::OnIntegerButtonDecrementChange, this));
			}

			MashRectangle2 textRegion = m_absoluteRegion;

			if (m_integerButtonsEnabled)
				textRegion.right -= (g_mashGUIButtonWidth + g_mashGUIButtonTextBuffer);

			m_textHandler.SetRegion(m_absoluteRegion, m_absoluteClippedRegion);
		}
	}

	f32 CMashGUITextBox::GetNumberIncrement(bool increment)
	{
		MashVector2 mousePos = m_inputManager->GetCursorPosition();
		f32 multiplier;
		if (increment)
			multiplier = m_incrementButton->GetAbsoluteRegion().top - mousePos.y;
		else
			multiplier = mousePos.y - m_decrementButton->GetAbsoluteRegion().bottom;

		f32 incrementNumber = 1.0f;
		if (m_textFormat == aGUI_TEXT_FORMAT_FLOAT)
		{
			incrementNumber = incrementNumber / powf(10, m_floatPrecision);
		}
		else
		{
			multiplier /= 50.0f;//magic numbers
		}
		
		multiplier = math::Clamp<f32>(1.0f, 50.0f, multiplier);

		return incrementNumber * multiplier;
	}

	void CMashGUITextBox::IncrementNumber()
	{
		const f32 numberIncrement = GetNumberIncrement(true);

		if (m_textFormat == aGUI_TEXT_FORMAT_INT)
		{
			int32 num = GetTextAsInt();
			if (num > 0)
			{
				//test for max int32
				num += numberIncrement;
				if (num < 0)
					num = m_numberMinVal;
			}
			else
				num += numberIncrement;

			SetTextInt(num);	
		}
		else if (m_textFormat == aGUI_TEXT_FORMAT_FLOAT)
		{
			f32 num = GetTextAsFloat();
			if (num > 0.0f)
			{
				//test for max f32
				num += numberIncrement;
				if (num >= (f32)mash::math::MaxInt32())
					num = m_numberMinVal;
			}
			else
			{
				num += numberIncrement;
			}

			SetTextFloat(num);	
		}
	}

	void CMashGUITextBox::DecrementNumber()
	{
		const f32 numberIncrement = GetNumberIncrement(false);

		if (m_textFormat == aGUI_TEXT_FORMAT_INT)
		{
			int32 num = GetTextAsInt();
			if (num < 0)
			{
				//test for min int32
				num -= numberIncrement;
				if (num > 0)
					num = m_numberMaxVal;
			}
			else
				num -= numberIncrement;

			SetTextInt(num);				
		}
		else if (m_textFormat == aGUI_TEXT_FORMAT_FLOAT)
		{
			f32 num = GetTextAsFloat();
			if (num < 0.0f)
			{
				//test for min f32
				num -= numberIncrement;
				if (num <= mash::math::MinInt32())
					num = m_numberMaxVal;
			}
			else
				num -= numberIncrement;

			SetTextFloat(num);				
		}
	}

	void CMashGUITextBox::OnIntegerButtonIncrementChange(const sGUIEvent &eventData)
	{
		if (m_incrementButton->IsButtonDown())
		{
			IncrementNumber();		

			m_numberKeyHeld = true;
			m_numberKeyTimer = -m_numberKeyDelay * 5.0f;
			m_numberKeyLastTime = MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart();
			m_numberIncrementKeyAction = true;
		}
		else
		{
			m_numberKeyHeld = false;
		}
	}

	void CMashGUITextBox::OnIntegerButtonDecrementChange(const sGUIEvent &eventData)
	{
		if (m_decrementButton->IsButtonDown())
		{
			DecrementNumber();

			m_numberKeyHeld = true;
			m_numberKeyTimer = -m_numberKeyDelay * 5.0f;
			m_numberKeyLastTime = MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart();
			m_numberIncrementKeyAction = false;
		}
		else
		{
			m_numberKeyHeld = false;
		}
	}

	void CMashGUITextBox::SetEventsEnabled(bool state)
	{
		MashGUIComponent::SetEventsEnabled(state);

		if (m_incrementButton)
			m_incrementButton->SetEventsEnabled(state);
		if (m_decrementButton)
			m_decrementButton->SetEventsEnabled(state);
	}

	MashGUIComponent* CMashGUITextBox::GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren)
	{
		//test children first
		if (bTestAllChildren || (GetRenderEnabled() && GetCanHaveFocus() && (m_cullState != aCULL_CULLED)))
		{
			if (m_incrementButton)
			{
				MashGUIComponent *pIntersects = m_incrementButton->GetClosestIntersectingChild(vScreenPos, bTestAllChildren);
				if (pIntersects)
					return pIntersects;
			}
			if (m_decrementButton)
			{
				MashGUIComponent *pIntersects = m_decrementButton->GetClosestIntersectingChild(vScreenPos, bTestAllChildren);
				if (pIntersects)
					return pIntersects;
			}

			//now test this rect
			MashGUIComponent *pIntersects = MashGUIComponent::GetClosestIntersectingChild(vScreenPos, bTestAllChildren);
			if (pIntersects)
				return pIntersects;
		}

		return 0;
	}

	void CMashGUITextBox::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
		{
			m_textHandler.AddPosition(deltaX, deltaY);
		}
		else
		{
			MashRectangle2 textRegion = m_absoluteRegion;

			if (m_incrementButton || m_decrementButton)
				textRegion.right -= (g_mashGUIButtonWidth + g_mashGUIButtonTextBuffer);

			m_textHandler.SetRegion(textRegion, m_absoluteClippedRegion);
		}

		if (m_incrementButton)
			m_incrementButton->UpdateRegion();
		if (m_decrementButton)
			m_decrementButton->UpdateRegion();
	}

	void CMashGUITextBox::Draw()
	{
		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			uint32 newTime = MashDevice::StaticDevice->GetTimer()->GetTimeSinceProgramStart();

			if (m_repearCharEnabled)
			{
				m_backspaceKeyTimer += (newTime - m_backspaceKeyLastTime);
				m_backspaceKeyLastTime = newTime;
				if (m_backspaceKeyTimer >= m_backspaceKeyDelay)
				{
					sInputEvent eventData;
					eventData.character = m_repeatChar;
					eventData.action = m_repeatEventType;
					eventData.eventType = sInputEvent::aEVENTTYPE_KEYBOARD;
					eventData.isPressed = 1;
					OnEvent(eventData);

					m_backspaceKeyTimer = 0;
				}
			}

			if (m_numberKeyHeld)
			{
				m_numberKeyTimer += (newTime - m_numberKeyLastTime);
				m_numberKeyLastTime = newTime;
				if (m_numberKeyTimer >= m_numberKeyDelay)
				{
					if (m_numberIncrementKeyAction)
						IncrementNumber();
					else
						DecrementNumber();

					m_numberKeyTimer = 0;
				}
			}

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *activeSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND);

			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, activeSkin, m_overrideTransparency);

			m_textHandler.Draw(m_GUIManager, activeSkin->fontColour, m_overrideTransparency);

			//draw carret
			if (GetEventsEnabled() && GetHasFocus())
			{
				m_accumCursorBlinkTimer += (newTime - m_lastCursorBlinkTime);
				m_lastCursorBlinkTime = newTime;
				if (m_accumCursorBlinkTimer >= m_cursorBlinkTime)
				{
					m_accumCursorBlinkTimer = 0;
					m_drawCarret = !m_drawCarret;
				}

				if (m_drawCarret)
				{
					mash::MashVector2 cursorBottom = m_textHandler.GetCaratAbsPosition();
					mash::MashVector2 cursorTop = cursorBottom;
					cursorBottom.y += activeStyle->GetFont()->GetMaxCharacterHeight();

					//TODO : clip carret? Maybe not necessary?

					sMashColour fontColour = activeSkin->fontColour;
					if (m_overrideTransparency.enableOverrideTransparency && m_overrideTransparency.affectFontAlpha)
						fontColour.SetAlpha(m_overrideTransparency.alphaValue);

					m_GUIManager->DrawLine(cursorTop, cursorBottom, fontColour);
				}
			}

			if (m_incrementButton || m_decrementButton)
				m_GUIManager->FlushBuffers();

			if (m_incrementButton)
				m_incrementButton->Draw();
			if (m_decrementButton)
				m_decrementButton->Draw();
		}
	}
}