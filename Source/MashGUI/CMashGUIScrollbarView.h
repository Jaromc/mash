//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SCROLLBAR_VIEW_H_
#define _C_MASH_SCROLLBAR_VIEW_H_

#include "MashGUIScrollbarView.h"
#include "MashRectangle2.h"
namespace mash
{
	const int32 g_mashGUIScrollbarWheelScrollAmount = 25;

	class CMashGUIScrollbarView : public MashGUIScrollbarView
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
	private:
		sButton m_IncrementButton;
		sButton m_DecrementButton;
		sButton m_ScrollBar;

		eBUTTON m_eSelectedButton;
		eBUTTON m_eMouseHoverButton;
		int32 m_styleElement;

		bool m_bIsVertical;

		f32 m_minValue;
		f32 m_maxValue;
		f32 m_sliderPosition;
		bool m_isShortDrawEnabled;

		f32 m_sliderValue;
		f32 m_moveMultiplier;

		bool m_resizeNeeded;
		void CalculateScrollbarRegion();
		void ResizeScrollbar(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void ResizeSlider();
		void OnMouseExit(const mash::MashVector2 &vScreenPos);
	public:
		CMashGUIScrollbarView(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			bool isVertical,
			int32 styleElement);

		~CMashGUIScrollbarView();

		void OnStyleChange(MashGUIStyle *style){}
		void SetAsHorizontal();
		void SetAsVertical();

		void MoveSlider(f32 value);

		void SetDualScrollEnabled(bool state);

		void SetRenderEnabled(bool bEnable);

		void OnEvent(const sInputEvent &eventData);
		void Draw();
		bool GetIsVertical()const;
		bool GetIsHorizontal()const;

		void SetSliderMaxValue(f32 absDist);
		void ResetSlider();

		f32 GetSliderValue()const;
		int32 GetWheelScrollAmount()const;

		eMASH_GUI_TYPE GetGUIType()const;
	};

	inline int32 CMashGUIScrollbarView::GetWheelScrollAmount()const
	{
		return g_mashGUIScrollbarWheelScrollAmount;
	}

	inline f32 CMashGUIScrollbarView::GetSliderValue()const
	{
		return m_sliderValue;
	}

	inline bool CMashGUIScrollbarView::GetIsVertical()const
	{
		return m_bIsVertical;
	}

	inline bool CMashGUIScrollbarView::GetIsHorizontal()const
	{
		return !m_bIsVertical;
	}

	inline eMASH_GUI_TYPE CMashGUIScrollbarView::GetGUIType()const
	{
		return aGUI_SCROLL_BAR_VIEW;
	}

}

#endif