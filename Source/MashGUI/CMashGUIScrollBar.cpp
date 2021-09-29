//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIScrollBar.h"
#include "CMashGUIButton.h"
#include "MashGUIManager.h"
namespace mash
{
	CMashGUIScrollBar::CMashGUIScrollBar(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const mash::MashGUIRect &destinationRect,
			f32 buttonIncrement,
			bool isVertical,
			int32 styleElement):MashGUIScrollBar(pGUIManager, pInputManager, pParent, destinationRect),
		m_buttonSize(0.0f, 30.0f),m_minValue(0), m_maxValue(100),
		m_sliderPosition(0),m_bDragging(false), m_buttonIncrement(buttonIncrement), m_sliderPixelRange(0.0f),
		m_bIsVertical(isVertical), m_styleElement(styleElement)
		
	{
		UpdateScollbar(false);
	}

	CMashGUIScrollBar::~CMashGUIScrollBar()
	{
		
	}
    
	f32 CMashGUIScrollBar::MoveSliderByPixels(f32 value)
	{
		f32 oldSliderValue = m_sliderValue;		
		m_sliderPosition += value;

		ResizeSlider();

		f32 normalizedPixelRange = m_sliderPosition / m_sliderPixelRange;
		m_sliderValue = normalizedPixelRange * (m_maxValue - m_minValue);

		//send messages
		sGUIEvent newGUIMsg;
		
		newGUIMsg.GUIEvent = aGUIEVENT_SB_VALUE_CHANGE;
		newGUIMsg.component = this;
		newGUIMsg.value = m_sliderValue - oldSliderValue;
		if (newGUIMsg.value != 0)
			ImmediateBroadcast(newGUIMsg);

		return m_sliderValue;
	}

	f32 CMashGUIScrollBar::SetSliderValue(f32 value)
	{
		value = math::Clamp<f32>(m_minValue, m_maxValue, value);

		if (value == m_sliderValue)
			return m_sliderValue;

		f32 oldSliderValue = m_sliderValue;
		m_sliderValue = value;
		f32 normalizedValue = m_sliderValue / (m_maxValue - m_minValue);
		
		m_sliderPosition = normalizedValue * m_sliderPixelRange;

		ResizeSlider();

		//send messages
		sGUIEvent newGUIMsg;
		
		newGUIMsg.GUIEvent = aGUIEVENT_SB_VALUE_CHANGE;
		newGUIMsg.component = this;
		newGUIMsg.value = m_sliderValue - oldSliderValue;
		ImmediateBroadcast(newGUIMsg);

		return m_sliderValue;
	}

	void CMashGUIScrollBar::SetIncrementButtonAmount(f32 value)
	{
		m_buttonIncrement = value;
		if (m_buttonIncrement < 0.0f)
			m_buttonIncrement = 0.0f;
	}

	void CMashGUIScrollBar::SetSliderMinMaxValues(f32 min, f32 max)
	{
		m_minValue = min;
		m_maxValue = max;

		if (m_maxValue < 1)
			m_maxValue = 1;

		if (m_minValue > m_maxValue)
			math::Swap<f32>(m_minValue, m_maxValue);
	}

	void CMashGUIScrollBar::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if ((eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE))
			{
				int32 mouseX, mouseY;
				m_inputManager->GetCursorPosition(mouseX, mouseY);

				switch(eventData.action)
				{
				case aMOUSEEVENT_B1:
					{
						if ((eventData.isPressed == 1) && (m_eMouseHoverButton != aNONE))
						{
							if (m_eMouseHoverButton == aINCREMENT)
							{
								m_eSelectedButton = aINCREMENT;
								SetSliderValue(m_sliderValue + m_buttonIncrement);
							}
							else if (m_eMouseHoverButton == aDECREMENT)
							{
								m_eSelectedButton = aDECREMENT;
								SetSliderValue(m_sliderValue - m_buttonIncrement);
							}
							else if (m_eMouseHoverButton == aSLIDER)
							{
								m_eSelectedButton = aSLIDER;
							}
						}
						else if (eventData.isPressed == 0)
							m_eSelectedButton = aNONE;

						break;
					}
				case aMOUSEEVENT_AXISZ:
					{
						f32 moveAmount = 1.0f;
						if (eventData.value < 0.0f)
						{
							MoveSliderByPixels(moveAmount);
						}
						else
						{
							MoveSliderByPixels(-moveAmount);
						}

						break;
					}
				case aMOUSEEVENT_AXISX:
				case aMOUSEEVENT_AXISY:
					{					
						if (m_hasFocus)
						{
							if (m_eSelectedButton == aSLIDER)
							{
								f32 moveAmount = eventData.value;
					
								MoveSliderByPixels(moveAmount);
							}

							m_eMouseHoverButton = aNONE;
							if (m_mouseHover)
							{
								mash::MashVector2 vCurrentMousePos = m_inputManager->GetCursorPosition();

								if (m_IncrementButton.absRegion.IntersectsGUI(vCurrentMousePos))
								{
									m_eMouseHoverButton = aINCREMENT;
								}
								else if (m_DecrementButton.absRegion.IntersectsGUI(vCurrentMousePos))
								{
									m_eMouseHoverButton = aDECREMENT;
								}
								else if (m_ScrollBar.absRegion.IntersectsGUI(vCurrentMousePos))
								{
									m_eMouseHoverButton = aSLIDER;
								}
								else
								{
									
								}
							}
						}
					}
				};
			}
		}
	}

	void CMashGUIScrollBar::ResizeSlider()
	{
		MashGUIRect buttonRect;

		if (m_bIsVertical)
		{
			buttonRect.left = MashGUIUnit(0.0f, 0.0f);
			buttonRect.right = MashGUIUnit(1.0f, 0.0f);

			m_sliderPixelRange = (m_absoluteRegion.bottom - m_absoluteRegion.top) - m_buttonSize.offset - m_buttonSize.offset - m_buttonSize.offset;
			m_sliderPosition = math::Clamp<f32>(0, m_sliderPixelRange, m_sliderPosition);

			//set slider rect
			buttonRect.top = MashGUIUnit(0.0f, m_buttonSize.offset + m_sliderPosition);
			buttonRect.bottom = MashGUIUnit(0.0f, m_buttonSize.offset + m_buttonSize.offset + m_sliderPosition);
			buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_ScrollBar.absRegion);
		}
		else
		{
			buttonRect.top = MashGUIUnit(0.0f, 0.0f);
			buttonRect.bottom = MashGUIUnit(1.0f, 0.0f);

			m_sliderPixelRange = (m_absoluteRegion.right - m_absoluteRegion.left) - m_buttonSize.offset - m_buttonSize.offset - m_buttonSize.offset;
			m_sliderPosition = math::Clamp<f32>(0, m_sliderPixelRange, m_sliderPosition);

			//set slider rect
			buttonRect.left = MashGUIUnit(0.0f, m_buttonSize.offset + m_sliderPosition);
			buttonRect.right = MashGUIUnit(0.0f, m_buttonSize.offset + m_buttonSize.offset + m_sliderPosition);
			buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_ScrollBar.absRegion);
		}
	}

	void CMashGUIScrollBar::UpdateScollbar(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
		{
			m_DecrementButton.absRegion.left += deltaX;
			m_DecrementButton.absRegion.right += deltaX;
			m_DecrementButton.absRegion.top += deltaY;
			m_DecrementButton.absRegion.bottom += deltaY;

			m_IncrementButton.absRegion.left += deltaX;
			m_IncrementButton.absRegion.right += deltaX;
			m_IncrementButton.absRegion.top += deltaY;
			m_IncrementButton.absRegion.bottom += deltaY;

			m_ScrollBar.absRegion.left += deltaX;
			m_ScrollBar.absRegion.right += deltaX;
			m_ScrollBar.absRegion.top += deltaY;
			m_ScrollBar.absRegion.bottom += deltaY;
		}
		else
		{
			MashGUIRect buttonRect;

			if (m_bIsVertical)
			{
				//set decrement button rect
				buttonRect.left = MashGUIUnit(0.0f, 0.0f);
				buttonRect.right = MashGUIUnit(1.0f, 0.0f);
				buttonRect.top = MashGUIUnit(0.0f, 0.0f);
				buttonRect.bottom = MashGUIUnit(0.0f, m_buttonSize.offset);
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_DecrementButton.absRegion);

				//set increment button rect
				buttonRect.top = MashGUIUnit(1.0f, -m_buttonSize.offset);
				buttonRect.bottom = MashGUIUnit(1.0f, 0.0f);
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_IncrementButton.absRegion);
			}
			else
			{
				//set decrement button rect
				buttonRect.left = MashGUIUnit(0.0f, 0.0f);
				buttonRect.right = MashGUIUnit(0.0f, m_buttonSize.offset);
				buttonRect.top = MashGUIUnit(0.0f, 0.0f);
				buttonRect.bottom = MashGUIUnit(1.0f, 0.0f);
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_DecrementButton.absRegion);

				//set increment button rect
				buttonRect.left = MashGUIUnit(1.0f, -m_buttonSize.offset);
				buttonRect.right = MashGUIUnit(1.0f, 0.0f);
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_IncrementButton.absRegion);
			}

			ResizeSlider();
		}
	}

	void CMashGUIScrollBar::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		UpdateScollbar(positionChangeOnly, deltaX, deltaY);
	}

	void CMashGUIScrollBar::Draw()
	{
		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *backgroundSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_BACKGROUND);
			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, backgroundSkin, m_overrideTransparency);

			MashGUISkin *incrementSkin = 0;
			MashGUISkin *decrementSkin = 0;
			MashGUISkin *sliderSkin = 0;

			if (m_bIsVertical)
			{
				incrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_UP);
				decrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWUP_UP);

				if (m_eSelectedButton == aINCREMENT)
					incrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_DOWN);
				else if (m_eMouseHoverButton == aINCREMENT)
					incrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_HOVER);

				if (m_eSelectedButton == aDECREMENT)
					decrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWUP_DOWN);
				else if (m_eMouseHoverButton == aDECREMENT)
					decrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWUP_HOVER);
			}
			else
			{
				incrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_UP);
				decrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_UP);

				if (m_eSelectedButton == aINCREMENT)
					incrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_DOWN);
				else if (m_eMouseHoverButton == aINCREMENT)
					incrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_HOVER);

				if (m_eSelectedButton == aDECREMENT)
					decrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_DOWN);
				else if (m_eMouseHoverButton == aDECREMENT)
					decrementSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_HOVER);
			}

			sliderSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_SLIDER_UP);

			if (m_eSelectedButton == aSLIDER)
				sliderSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_SLIDER_DOWN);
			else if (m_eMouseHoverButton == aSLIDER)
				sliderSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_SLIDER_HOVER);

			m_GUIManager->DrawSprite(m_IncrementButton.absRegion, m_absoluteClippedRegion, incrementSkin, m_overrideTransparency);
			m_GUIManager->DrawSprite(m_DecrementButton.absRegion, m_absoluteClippedRegion, decrementSkin, m_overrideTransparency);
			m_GUIManager->DrawSprite(m_ScrollBar.absRegion, m_absoluteClippedRegion, sliderSkin, m_overrideTransparency);
		}
	}
}