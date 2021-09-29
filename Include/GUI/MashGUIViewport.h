//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_VIEWPORT_H_
#define _MASH_GUI_VIEWPORT_H_

#include "MashGUIComponent.h"
#include "MashEventTypes.h"

namespace mash
{
	/*!
		This component will create a viewport that is equal in size to this component.
		This can be handy if a windows size may change.

		Use SetViewport() before rendering anything you want to appear within this viewport
		and RestoreOriginalViewport() when rendering is complete.
	*/
	class MashGUIViewport : public MashGUIComponent
	{
	public:
		MashGUIViewport(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUIViewport(){}

		//! Calculates the viewport.
		/*!
			\param out Viewport to fill.
		*/
		virtual void GetViewport(sMashViewPort &out) = 0;
		
		//! Sets a callback that will receive raw input messages when this component is focused.
		/*!
			\param callback Input callback.
		*/
		virtual void SetInputEventCallback(const MashInputEventFunctor &callback) = 0;

		//! Calculates the viewports width and height.
		/*!
			\param x Viewport width.
			\param y Viewport height.
		*/
		virtual void GetViewportWidthHeight(f32 &x, f32 &y)const = 0;

		//! Sets the viewport to the renderer.
		/*!
			Call this before rendering the objects you want to appear within this viewport.
		*/
		virtual void SetViewport() = 0;

		//! Restores the viewport to how it was before calling SetViewport()
		virtual void RestoreOriginalViewport() = 0;
	};
}

#endif