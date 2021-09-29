//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_VIEW_H_
#define _MASH_GUI_VIEW_H_

#include "MashGUIComponent.h"
#include "MashList.h"

namespace mash
{
	class MashGUICustomRender;

	/*!
		Views are containers that can hold child components.
	*/
	class MashGUIView : public MashGUIComponent
	{
	public:
		MashGUIView(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, 
			pInputManager, pParent, destination){}

		virtual ~MashGUIView(){}

		//! Sets a custom renderer.
		/*!
			This allows a custom background to be rendered.
			The view will grab a copy of the renderer and drop it when this view is destroyed.
			
			\param customRenderer Custom renderer. The renderer may be dropped after calling this.
		*/
		virtual void SetCustomRenderer(MashGUICustomRender *customRenderer) = 0;

		//! Sets the render background state.
		/*!
			This can be set to false to make the view transparent, and only render is children.

			\param state Render background state.
		*/
		virtual void SetRenderBackgroundState(bool state) = 0;

		//! Get the render background state.
		/*!
			\return True if render background is enabled, false otherwise.
		*/
		virtual bool GetRenderbackgroundState()const = 0;

		//! Is the horizontal scrollbar enabled.
		/*!
            Note if this returns true, the scrollbar may not be activated if
            all children are within the view bounds.
         
			\return True if the scrollbar is enabled, false otherwise.
		*/
		virtual bool IsHorizontalScrollEnabled()const = 0;

		//! Is the vertical scrollbar enabled.
		/*!
            Note if this returns true, the scrollbar may not be activated if
            all children are within the view bounds.
         
			\return True if the scrollbar is enabled, false otherwise.
		*/
		virtual bool IsVerticalScrollEnabled()const = 0;
        
        //! Is the horizontal scrollbar enabled and visible.
		/*!
            \return True if the scrollbar is enabled and visible, false otherwise.
         */
		virtual bool IsHorizontalScrollInUse()const = 0;
        
		//! Is the vertical scrollbar enabled and visible.
		/*!         
            \return True if the scrollbar is enabled and visible, false otherwise.
         */
		virtual bool IsVerticalScrollInUse()const = 0;
		
		//! Sets the vertical scrollbar state.
		/*!
			This allows a scrollbar to activate if there are items that
			are positioned past the views bounds. If no items exist or
			they are all within the views bounds then the scrollbars
			will not appear.

			\param state Vertical scrollbar state.			
		*/
		virtual void SetVerticalScrollState(bool state) = 0;

		//! Sets the horizontal scrollbar state.
		/*!
			This allows a scrollbar to activate if there are items that
			are positioned past the views bounds. If no items exist or
			they are all within the views bounds then the scrollbars
			will not appear.

			\param state Horizontal scrollbar state.			
		*/
		virtual void SetHorizontalScrollState(bool state) = 0;
		
		//! Gets the vertical scrollbar value.
		/*!
			\return Vertical scrollbar value.
		*/
		virtual f32 GetVerticalScrollAmount()const = 0;

		//! Gets the horizontal scrollbar value.
		/*!
			\return Horizontal scrollbar value.
		*/
		virtual f32 GetHorizontalScrollAmount()const = 0;
		
		//! Adds a child to this view.
		/*!
			\param child Component to add as a child.
		*/
		virtual void AddChild(MashGUIComponent *child) = 0;

		//! Detaches all children from this view.
		/*!
			This will also drop the children so if there is no more references
			to them, they will be destroyed.
		*/
		virtual void DetachAllChildren() = 0;

		//! Detaches a child.
		/*!
			This will also drop the children so if there is no more references
			to them, they will be destroyed.

			\param child Child to remove.
			\return True if the child was destroyed. False otherwise.
		*/
		virtual bool DetachChild(MashGUIComponent *child) = 0;

		//! Gets the list of children.
		/*!
			\return Child list.
		*/
		virtual const MashList<MashGUIComponent*>& GetChildren()const = 0;

		//! Sends a child to the front for rendering.
		/*!
			\param child Child to send to the front.
		*/
		virtual void SendChildToFront(MashGUIComponent *child) = 0;

		//! Sends a child to the back for rendering.
		/*!
			\param child Child to send to the back.
		*/
		virtual void SendChildToBack(MashGUIComponent *child) = 0;
	};
}

#endif