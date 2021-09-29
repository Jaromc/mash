//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_SCROLL_BAR_VIEW_H_
#define _MASH_GUI_SCROLL_BAR_VIEW_H_

#include "MashGUIComponent.h"

namespace mash
{
	/*!
		These are used only for scrolling items within a component.
	*/
	class MashGUIScrollbarView : public MashGUIComponent
	{
	public:
		MashGUIScrollbarView(MashGUIManager *pGUIManager, 
			MashInputManager *pInputManager,
			MashGUIComponent *pParent, 
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}
		virtual ~MashGUIScrollbarView(){}

		//! Is this scrollbar vertical.
		/*!
			\return True if this scrollbar is vertical, false otherwise.
		*/
		virtual bool GetIsVertical()const = 0;

		//! Is this scrollbar horizontal.
		/*!
			\return True if this scrollbar is horizontal, false otherwise.
		*/
		virtual bool GetIsHorizontal()const = 0;

		//! Sets the slider distance.
		/*!
			The slider will then lerp between 0 and this value.

			\param dist Slider distance.
		*/
		virtual void SetSliderMaxValue(f32 dist) = 0;

		//! Returns the slider value.
		/*!
			\return current slider value.
		*/
		virtual f32 GetSliderValue()const = 0;

		//! Moves the slider by the given value.
		/*!
			\param value Moves the slider by this amount. Maybe positive or negative.
		*/
		virtual void MoveSlider(f32 value) = 0;

		//! Sets the slider back to zero.
		virtual void ResetSlider() = 0;

		//! Sets dual scroll.
		/*!
			An owner would call this when both horizontal and vertical have been enabled.
			It shortens the length of the scrollbar so both scrollbars don't overlap
			each other.

			\param Dual scroll state.
		*/
		virtual void SetDualScrollEnabled(bool state) = 0;

		//! Gets the amount the slider moves for the mouse wheel.
		/*!
			\return Mouse wheel movement.
		*/
		virtual int32 GetWheelScrollAmount()const = 0;

	};
}

#endif