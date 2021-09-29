//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_WINDOW_H_
#define _MASH_GUI_WINDOW_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUIView;

	/*!
		Windows are basically a view with a title bar added.
		Windows can be dragged by the title bar and have minimize and close buttons added.
		They can also have the focus locked to them when activated so that nothing else
		but it and its children can be selected until the window is either closed or hidden.
	*/
	class MashGUIWindow : public MashGUIComponent
	{
	public:
		enum eCLOSE_BUTTON_EVENT
		{
			aCLOSE_AND_DESTROY,
			aCLOSE_AND_HIDE
		};
	public:
		MashGUIWindow(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUIWindow(){}

		//! Sets the title bar text.
		/*!
			\param text Title bar text.
		*/
		virtual void SetTitleBarText(const MashStringc &text) = 0;

		//! Returns the title bar text.
		/*!
			\return Title bar text.
		*/
		virtual const MashStringc& GetTitleBarText()const = 0;

		//! Enable or disable window mouse drag.
		/*!
			The user must click the title bar and hold the mouse
			button down to drag a window.

			\param state Drag state.
		*/
		virtual void EnableMouseDrag(bool state) = 0;

		//! Returns if mouse drag is enabled.
		/*!
			\return True if mouse drag is enabled, false otherwise.
		*/
		virtual bool GetMouseDragEnabled()const = 0;

		//! Enables or disables the close button
		/*!
			The effect of the close button can be set by SetCloseButtonEvent().
			
			\param enable Enable or disable the close button.
		*/
		virtual void EnableCloseButton(bool enable) = 0;

		//! Enables or disables the minimize button
		/*!
			The minimize button when selected hides the windows view to 
			leave only the title bar.

			\param enable Enable or disable the minimize button.
		*/
		virtual void EnableMinimizeButton(bool enable) = 0;

		//! Gets the close button state.
		/*!
			\return True if the close button is enabled, false otherwise.
		*/
		virtual bool GetCloseButtonEnabled()const = 0;

		//! Gets the minimize button state.
		/*!
			\return True if the minimize button is enabled, false otherwise.
		*/
		virtual bool GetMinimizeButtonEnabled()const = 0;

		//! Sets the event that will occur when the close button is pressed.
		/*!
			Sometimes you may not want the window to be destroyed when closing
			it. You may only want it hidden instead.

			\param e Event to occur.
		*/
		virtual void SetCloseButtonEvent(eCLOSE_BUTTON_EVENT e) = 0;

		//! Gets the close button event.
		/*!
			\return Close button event.
		*/
		virtual eCLOSE_BUTTON_EVENT GetCloseButtonEvent()const = 0;

		//! Enables focus lock for this window.
		/*!
			This makes the window act like a dialog.

			When rendering is enabled, only children from this window will be selectable.
			This behaviour will be stacked if multiple windows are open with this option enabled.
         
            Note this is relative to the parent node, it is not a global action.

			SetAlwaysOnTop() is similar to this method however focus will not be locked to this window.

			\param lockFocus Enable or disable focus lock.
		*/
		virtual void SetLockFocusWhenActivated(bool lockFocus) = 0;

		//! This window will always be rendered on top.
		/*!
			This is similar to SetLockFocusWhenActivated() however the focus will
			not be locked to this window.
         
            Note this is relative to the parent node, it is not a global action.

			\param enable Enables or disables this window to be always on top.
		*/
		virtual void SetAlwaysOnTop(bool enable) = 0;
        
        //! Minimizes or maximizes the window.
        virtual void SetMinimizeState(bool enable) = 0;
        
        //! Closes the window.
        virtual void CloseWindow() = 0;
	};
}

#endif