//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_BUTTON_H_
#define _MASH_GUI_BUTTON_H_

#include "MashGUIComponent.h"

namespace mash
{
	/*!
		Switches will only return aGUIEVENT_BTN_DOWN && aGUIEVENT_BTN_UP_CONFIRM events.
		aGUIEVENT_BTN_DOWN will only be called when the down event is confirmed, that is, when
		the user has released the mouse button while over the gui button.

		Normal buttons will return all events. aGUIEVENT_BTN_DOWN will be called immediately when
		the user clicks the gui button.
	*/
	class MashGUIButton : public MashGUIComponent
	{
	public:
		MashGUIButton(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUIButton(){}

		//! This allows you to override the defult styles. Eg, use scroll bar arrow etc...
		/*!
			\param elementId element id.
			\param up Up attribute.
			\param down Down attribute.
			\param hover Hover attribute.
		*/
		virtual void SetButtonStyles(int32 elementId, eGUI_STYLE_ATTRIBUTE up, eGUI_STYLE_ATTRIBUTE down, eGUI_STYLE_ATTRIBUTE hover) = 0;

		//! Sets the text that will be displayed on this button.
		/*!
			\param text Button text.
		*/
		virtual void SetText(const MashStringc &text) = 0;

		//! Returns the text on this button.
		/*!
			\return Button string.
		*/
		virtual const MashStringc& GetText()const = 0;
		
		//! Enable switch mode.
		/*!
			This button will act like a on/off switch rather than a spring push button.
			\param state Enable or disable the switch state.
		*/	
		virtual void SetIsSwitch(bool state) = 0;

		//! Is in switch mode.
		/*!
			\return Is in switch mode.
		*/
		virtual bool IsSwitch()const = 0;

		//! Push a switch on or off.
		/*!
			This is for manual operation rather than mouse input.
			Only works when this button is set as a switch.
			\param isPressed True for pressed, false for released.
		*/
		virtual void SetSwitchState(bool isPressed) = 0;

		//! Is this button/switch currently down.
		/*!
			For button or switch mode.
			\return Is the button currently down.
		*/
		virtual bool IsButtonDown()const = 0;
	};
}

#endif