//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIScrollbarView.h"
#include "MashGUIManager.h"

namespace mash
{
	const f32 g_mashGUIScrollbarWidth = 15.0f;

	CMashGUIScrollbarView::CMashGUIScrollbarView(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			bool isVertical,
			int32 styleElement):MashGUIScrollbarView(pGUIManager, pInputManager, pParent, mash::MashGUIRect(mash::MashGUIUnit(1.0f,-g_mashGUIScrollbarWidth), mash::MashGUIUnit(0.0f,0.0f),
				mash::MashGUIUnit(1.0f, 0), mash::MashGUIUnit(1.0f, 0.0f))),
		m_bIsVertical(isVertical),m_minValue(0), m_maxValue(100),
		m_sliderPosition(0), m_moveMultiplier(0.0f), m_sliderValue(0.0f), m_styleElement(styleElement),
		m_isShortDrawEnabled(false), m_resizeNeeded(true)
		
	{
		CalculateScrollbarRegion();
	}

	CMashGUIScrollbarView::~CMashGUIScrollbarView()
	{
		
	}

	void CMashGUIScrollbarView::CalculateScrollbarRegion()
	{
		if (m_bIsVertical)
		{
			MashGUIRect rect(mash::MashGUIUnit(1.0f,-g_mashGUIScrollbarWidth), mash::MashGUIUnit(0.0f,0.0f),
				mash::MashGUIUnit(1.0f, 0), mash::MashGUIUnit(1.0f, 0.0f));

			if (m_isShortDrawEnabled)
				rect.bottom.offset = -g_mashGUIScrollbarWidth;

			SetDestinationRegion(rect);
		}
		else
		{
			MashGUIRect rect(mash::MashGUIUnit(0.0f,0.0f), mash::MashGUIUnit(1.0f,-g_mashGUIScrollbarWidth),
				mash::MashGUIUnit(1.0f, 0.0f), mash::MashGUIUnit(1.0f, 0));

			if (m_isShortDrawEnabled)
				rect.right.offset = -g_mashGUIScrollbarWidth;

			SetDestinationRegion(rect);
		}
	}

	void CMashGUIScrollbarView::SetAsHorizontal()
	{
		m_bIsVertical = false;
	}

	void CMashGUIScrollbarView::SetAsVertical()
	{
		m_bIsVertical = true;
	}

	void CMashGUIScrollbarView::ResizeSlider()
	{
		if (m_bIsVertical)
		{
			MashGUIRect buttonRect;
			buttonRect.left = MashGUIUnit(0.0f, 0.0f);
			buttonRect.right = MashGUIUnit(1.0f, 0.0f);

			//subtract 2 buttons from range
			f32 windowRange = (m_absoluteRegion.bottom - m_absoluteRegion.top) - g_mashGUIScrollbarWidth - g_mashGUIScrollbarWidth;
			f32 scrollRange = m_maxValue - m_minValue;
			f32 buttonLength = 0.0f;
			if (windowRange >= scrollRange)
			{
				m_sliderPosition = 0;
				buttonLength = windowRange;
				m_moveMultiplier = 0.0f;
			}
			else
			{
				f32 moveRange = scrollRange - windowRange;
				buttonLength = (windowRange - moveRange);

				if (buttonLength < g_mashGUIScrollbarWidth)
				{
					/*
						g_mashGUIScrollbarWidth - buttonLength = get remainder
						windowRange - g_mashGUIScrollbarWidth = minus g_mashGUIScrollbarWidth for button width
					*/
					f32 denom = windowRange;
					if (denom != 0.0f)
						m_moveMultiplier = (g_mashGUIScrollbarWidth - buttonLength) / (denom - g_mashGUIScrollbarWidth);

					buttonLength = g_mashGUIScrollbarWidth;
				}

				m_sliderPosition = math::Clamp<f32>(0, windowRange - buttonLength, m_sliderPosition);
			}

			//set slider rect
			buttonRect.top = MashGUIUnit(0.0f, g_mashGUIScrollbarWidth + m_sliderPosition);
			buttonRect.bottom = MashGUIUnit(0.0f, g_mashGUIScrollbarWidth + buttonLength + m_sliderPosition);
			buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_ScrollBar.absRegion);
		}
		else
		{
			MashGUIRect buttonRect;
			buttonRect.top = MashGUIUnit(0.0f, 0.0f);
			buttonRect.bottom = MashGUIUnit(1.0f, 0.0f);

			//subtract 2 buttons from range
			f32 windowRange = (m_absoluteRegion.right - m_absoluteRegion.left) - g_mashGUIScrollbarWidth - g_mashGUIScrollbarWidth;
			f32 scrollRange = m_maxValue - m_minValue;
			f32 buttonLength = 0.0f;
			if (windowRange >= scrollRange)
			{
				m_sliderPosition = 0;
				buttonLength = windowRange;
				m_moveMultiplier = 0.0f;
			}
			else
			{
				f32 moveRange = scrollRange - windowRange;
				buttonLength = (windowRange - moveRange);

				if (buttonLength < g_mashGUIScrollbarWidth)
				{
					/*
						g_mashGUIScrollbarWidth - buttonLength = get remainder
						windowRange - g_mashGUIScrollbarWidth = minus g_mashGUIScrollbarWidth for button width
					*/	
					m_moveMultiplier = (g_mashGUIScrollbarWidth - buttonLength) / (windowRange - g_mashGUIScrollbarWidth);
					buttonLength = g_mashGUIScrollbarWidth;
				}

				m_sliderPosition = math::Clamp<f32>(0, windowRange - buttonLength, m_sliderPosition);
			}

			//set slider rect
			buttonRect.left = MashGUIUnit(0.0f, g_mashGUIScrollbarWidth + m_sliderPosition);
			buttonRect.right = MashGUIUnit(0.0f, g_mashGUIScrollbarWidth + buttonLength + m_sliderPosition);
			buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_ScrollBar.absRegion);
		}
	}

	void CMashGUIScrollbarView::SetRenderEnabled(bool bEnable)
	{
		MashGUIComponent::SetRenderEnabled(bEnable);

		if (!GetRenderEnabled())
		{
			//reset some values
			m_eSelectedButton = aNONE;
		}
	}

	void CMashGUIScrollbarView::MoveSlider(f32 value)
	{
		m_sliderPosition += value;
		ResizeSlider();
		f32 oldSliderValue = m_sliderValue;

		/*
			Small precision values were causing problems where text
			would be offset by a small amount after resizing. Flooring
			the value seems to fix this issue.
		*/
		m_sliderValue = floor(m_sliderPosition + (m_moveMultiplier * m_sliderPosition));

		//send messages
		sGUIEvent newGUIMsg;
		
		newGUIMsg.GUIEvent = aGUIEVENT_SB_VALUE_CHANGE;
		newGUIMsg.component = this;
		newGUIMsg.value = m_sliderValue - oldSliderValue;
		if (newGUIMsg.value != 0)
			ImmediateBroadcast(newGUIMsg);
	}

	void CMashGUIScrollbarView::SetSliderMaxValue(f32 absDist)
	{
		f32 windowRange = 0.0f;
		if (m_bIsVertical)
			windowRange = (m_absoluteRegion.bottom - m_absoluteRegion.top) - g_mashGUIScrollbarWidth - g_mashGUIScrollbarWidth;
		else
			windowRange = (m_absoluteRegion.right - m_absoluteRegion.left) - g_mashGUIScrollbarWidth - g_mashGUIScrollbarWidth;

		f32 newMaxValue = windowRange + absDist;
		if (m_maxValue != newMaxValue)
		{
			m_minValue = 0.0f;
			m_maxValue = newMaxValue;
			
			if (m_maxValue < 1)
				m_maxValue = 1;

			ResizeSlider();
		}
	}

	void CMashGUIScrollbarView::ResetSlider()
	{
		m_minValue = 0.0f;
		m_sliderValue = 0.0f;
		m_sliderPosition = 0.0f;
		m_moveMultiplier = 0.0f;
	}

	void CMashGUIScrollbarView::OnMouseExit(const mash::MashVector2 &vScreenPos)
	{
		if (!m_hasFocus)
			m_eMouseHoverButton = aNONE;
	}

	void CMashGUIScrollbarView::OnEvent(const sInputEvent &eventData)
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
						if (m_hasFocus && (eventData.isPressed == 1) && (m_eMouseHoverButton != aNONE))
						{
							f32 moveAmount = g_mashGUIScrollbarWheelScrollAmount;

							if (m_eMouseHoverButton == aINCREMENT)
							{
								m_eSelectedButton = aINCREMENT;
								MoveSlider(moveAmount);
							}
							else if (m_eMouseHoverButton == aDECREMENT)
							{
								m_eSelectedButton = aDECREMENT;
								MoveSlider(-moveAmount);
							}
							else if (m_eMouseHoverButton == aSLIDER)
							{
								m_eSelectedButton = aSLIDER;
							}
						}
						else if (eventData.isPressed == 0)
						{
							//send messages that a move button has been released
							sGUIEvent newGUIMsg;
							
							newGUIMsg.GUIEvent = aGUIEVENT_SB_BUTTON_RELEASE;
							newGUIMsg.component = this;
							newGUIMsg.value = 0.0f;
							ImmediateBroadcast(newGUIMsg);

							m_eSelectedButton = aNONE;
						}

						break;
					}
				case aMOUSEEVENT_AXISZ:
					{
						f32 moveAmount = g_mashGUIScrollbarWheelScrollAmount;// * m_moveFactor;
						if (eventData.value < 0)
						{
							MoveSlider(moveAmount);
						}
						else
						{
							MoveSlider(-moveAmount);
						}

						break;
					}
				case aMOUSEEVENT_AXISX:
				case aMOUSEEVENT_AXISY:
					{
						//if (m_hasFocus)
						{
							if (m_eSelectedButton == aSLIDER)
							{
								f32 moveAmount = eventData.value;
					
								MoveSlider(moveAmount);
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

	void CMashGUIScrollbarView::ResizeScrollbar(bool positionChangeOnly, f32 deltaX, f32 deltaY)
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

			ResizeSlider();
		}
		else
		{
			MashGUIRect buttonRect;

			if (m_bIsVertical)
			{
				MashGUIUnit bottom = MashGUIUnit(0.0f, g_mashGUIScrollbarWidth);
				MashGUIUnit top = MashGUIUnit(1.0f, -g_mashGUIScrollbarWidth);

				buttonRect.left = MashGUIUnit(0.0f, 0.0f);
				buttonRect.right = MashGUIUnit(1.0f, 0.0f);
				buttonRect.top = MashGUIUnit(0.0f, 0.0f);
				buttonRect.bottom = bottom;
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_DecrementButton.absRegion);

				buttonRect.top = top;
				buttonRect.bottom = MashGUIUnit(1.0f, 0.0f);
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_IncrementButton.absRegion);
			}
			else
			{
				buttonRect.left = MashGUIUnit(0.0f, 0.0f);
				buttonRect.right = MashGUIUnit(0.0f, g_mashGUIScrollbarWidth);
				buttonRect.top = MashGUIUnit(0.0f, 0.0f);
				buttonRect.bottom = MashGUIUnit(1.0f, 0.0f);
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_DecrementButton.absRegion);

				buttonRect.left = MashGUIUnit(1.0f, -g_mashGUIScrollbarWidth);
				buttonRect.right = MashGUIUnit(1.0f, 0.0f);
				buttonRect.GetAbsoluteValue(this->GetAbsoluteRegion(), m_IncrementButton.absRegion);
			}

			ResizeSlider();
		}
	}

	void CMashGUIScrollbarView::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (m_resizeNeeded)
			positionChangeOnly = false;

		ResizeScrollbar(positionChangeOnly, deltaX, deltaY);

		m_resizeNeeded = false;
	}

	void CMashGUIScrollbarView::SetDualScrollEnabled(bool state)
	{
		if (m_isShortDrawEnabled != state)
		{
			m_isShortDrawEnabled = state;
			CalculateScrollbarRegion();
			m_resizeNeeded = true;
		}
	}

	void CMashGUIScrollbarView::Draw()
	{
		MashGUIComponent::Draw();

		if (m_resizeNeeded)
			ResizeScrollbar(false);

		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			

			/*
				These force the manager to flush the buffers so that text
				behind the scrollbars does not appear on top. This is due
				to the text batch rendering after sprites.
			*/
			m_GUIManager->FlushBuffers();

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

			/*
				If short draw is enabled then it is assumed both a vertical and horizontal bar is enabled
				by the parent. In this case we get one of them to render a small corner peice.
			*/
			if (m_isShortDrawEnabled && m_bIsVertical && m_parent)
			{
				mash::MashRectangle2 rect(m_absoluteRegion.right - g_mashGUIScrollbarWidth, m_absoluteRegion.bottom,
												m_absoluteRegion.right, m_absoluteRegion.bottom + g_mashGUIScrollbarWidth);
				m_GUIManager->DrawSprite(rect, m_parent->GetAbsoluteClippedRegion(), activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_SCROLLBAR_END_CAP), m_overrideTransparency);
			}
		}
	}
}