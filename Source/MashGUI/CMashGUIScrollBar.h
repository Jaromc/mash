//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_SCROLL_BAR_H_
#define _C_MASH_GUI_SCROLL_BAR_H_

#include "MashGUIScrollBar.h"

namespace mash
{
	class CMashGUIScrollBar : public MashGUIScrollBar
	{
	private:
		
		enum eBUTTON
		{
			aINCREMENT,
			aDECREMENT,
			aSLIDER,
			aNONE
		};

		struct sButton
		{
			mash::MashRectangle2 absRegion;
		};

		enum eSKINS
		{
			aSKIN_ACTIVE_ITEM,
			aSKIN_INACTIVE_ITEM,
			aSKIN_BACKGROUND,

			aSKIN_COUNT
		};
		
	private:
		MashGUIUnit m_buttonSize;

		sButton m_IncrementButton;
		sButton m_DecrementButton;
		sButton m_ScrollBar;

		eBUTTON m_eSelectedButton;
		eBUTTON m_eMouseHoverButton;

		int32 m_styleElement;

		bool m_bIsVertical;
		void UpdateComponenetPosition();

		f32 m_minValue;
		f32 m_maxValue;
		f32 m_sliderPosition;
		bool m_bDragging;
		f32 m_sliderValue;
		f32 m_buttonIncrement;
		f32 m_sliderPixelRange;

		void ResizeSlider();
		f32 MoveSliderByPixels(f32 value);
		void UpdateScollbar(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
	protected:
		
	public:
		CMashGUIScrollBar(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const mash::MashGUIRect &destinationRect,
			f32 buttonIncrement,
			bool isVertical,
			int32 styleElement);

		~CMashGUIScrollBar();

		void OnStyleChange(MashGUIStyle *style){}
		void OnEvent(const sInputEvent &eventData);
		void Draw();

		bool GetIsVertical()const;
		bool GetIsHorizontal()const;

		void SetSliderMinMaxValues(f32 min, f32 max);
		f32 SetSliderValue(f32 value);
		void SetIncrementButtonAmount(f32 value);
		
		f32 GetSliderMinValue()const;
		f32 GetSliderMaxValue()const;

		f32 GetSliderValue()const;

		eMASH_GUI_TYPE GetGUIType()const;
	};

	inline f32 CMashGUIScrollBar::GetSliderMinValue()const
	{
		return m_minValue;
	}

	inline f32 CMashGUIScrollBar::GetSliderMaxValue()const
	{
		return m_maxValue;
	}

	inline f32 CMashGUIScrollBar::GetSliderValue()const
	{
		return m_sliderValue;
	}

	inline bool CMashGUIScrollBar::GetIsVertical()const
	{
		return m_bIsVertical;
	}

	inline bool CMashGUIScrollBar::GetIsHorizontal()const
	{
		return !m_bIsVertical;
	}

	inline eMASH_GUI_TYPE CMashGUIScrollBar::GetGUIType()const
	{
		return aGUI_SCROLL_BAR;
	}
}

#endif