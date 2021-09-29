//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_SCROLL_BAR_H_
#define _MASH_GUI_SCROLL_BAR_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUIScrollBar : public MashGUIComponent
	{
	public:
		MashGUIScrollBar(MashGUIManager *pGUIManager, 
			MashInputManager *pInputManager,
			MashGUIComponent *pParent, 
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}
		virtual ~MashGUIScrollBar(){}

		//! Is this a vertical scrollbar.
		/*!
			\return True if this is a vertical scrollbar, false otherwise.
		*/
		virtual bool GetIsVertical()const = 0;

		//! Is this a horizontal scrollbar.
		/*!
			\return True if this is a horizontal scrollbar, false otherwise.
		*/
		virtual bool GetIsHorizontal()const = 0;

		//! Manually sets the slider value.
		/*!
			Sets the slider value rather than by mouse input.
			\param value New slider value.
			\return New validated slider value.
		*/
		virtual f32 SetSliderValue(f32 value) = 0;

		//! Returns the current slider value.
		/*!
			\return Current slider value.
		*/
		virtual f32 GetSliderValue()const = 0;

		//! Sets the min and max values of the slider.
		/*!
			The slider will then lerp between these values.
			\param min Minimum value.
			\param max Maximum value.
		*/
		virtual void SetSliderMinMaxValues(f32 min, f32 max) = 0;

		//! Returns the sliders minimum value.
		virtual f32 GetSliderMinValue()const = 0;

		//! Returns the sliders maximum value.
		virtual f32 GetSliderMaxValue()const = 0;

		//! Sets how much the slider will increment when the increment/decrement buttons are pressed.
		/*!
			\param increment Increment value.
		*/
		virtual void SetIncrementButtonAmount(f32 value) = 0;
	};
}

#endif