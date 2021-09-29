//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_COMPONENT_H_
#define _MASH_GUI_COMPONENT_H_

#include "MashCompileSettings.h"
#include "MashTypes.h"
#include "MashGUIEventDispatch.h"
#include "MashList.h"
#include "MashGUIStyle.h"
#include "MashGUICore.h"

namespace mash
{
	class MashGUIManager;
	class MashGUIView;
	class MashInputManager;
	struct sInputEvent;

	/*!
		Components don't really act like a node graph.
		All components can be parented to another, but only Views can have children.
		This design can make some things a little complicated, but it's a waste of memory
		and speed to factor in children to objects such as buttons that will probably
		never need children.

		Use Destroy() to remove a component from its parent and from the engine.
	*/
	class MashGUIComponent : public MashGUIEventDispatch, public MashReferenceCounter
	{
	private:
		
		virtual void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0) = 0;
	protected:

		enum GUI_UPDATE_FLAGS
		{
			aGUI_UPDATE_POSITION_ONLY = 1,
			aGUI_UPDATE_REGION = 0xFFFFFFFF
		};

		uint32 m_updateFlags;
		f32 m_positionChangeX;
		f32 m_positionChangeY;
		bool m_renderEnabled;

		/*
			Most of the time a parent will be a view. The main reasons this class does not support children
			is to :
				- best support batch rendering
				- most object will not need children, so it would be a waste of both memory and speed
			The main exceptions to this rule is for creating composite components in which
			case you would use the GUI factory to create the components, rather than the manager.
		*/
		MashGUIComponent *m_parent;
		//Screen position in local space
		MashGUIRect m_destinationRegion;
		//Screen position in world space
		mash::MashRectangle2 m_absoluteClippedRegion;
		mash::MashRectangle2 m_absoluteRegion;
		mash::MashVector2 m_lastParentPosition;

		MashStringc m_elementName;

		bool m_eventsEnabled;
		bool m_clippingEnabled;
		eCULL m_cullState;
		bool m_hasFocus;
		bool m_canHaveFocus;
		bool m_mouseHover;
		bool m_lockFocusWhenActivated;
		sGUIOverrideTransparency m_overrideTransparency;
		MashGUIManager *m_GUIManager;
		MashInputManager *m_inputManager;
		bool m_drawDebugBounds;
		bool m_alwaysOnTop;

		virtual void OnAddChildFromConstructor(MashGUIComponent *component);
		virtual void OnFocusGained(){}
		virtual void OnLostFocus(){}
		virtual void OnMouseEnter(const mash::MashVector2 &vScreenPos){}
		virtual void OnMouseExit(const mash::MashVector2 &vScreenPos){}
		virtual void OnChildRegionChange(MashGUIComponent *component){}
		virtual void OnChildAlwaysOnTop(MashGUIComponent *component){}
		virtual void OnRemoveChildAlwaysOnTop(MashGUIComponent *component){}
	public:
		MashGUIComponent(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination);

		virtual ~MashGUIComponent();

		//! Detaches this object from its parent.
		/*!
			If it's attached to a parent then it will be detached from it then dropped.
			If this parent was the last element referencing the element then it will
			be destroyed. So be sure to call Grab() if you want to keep it around.
			
			\return True if this element was destroyed. False if it has 0 references.
		*/
		virtual bool Detach();

		//! Detaches amd Deletes an object.
		virtual void Destroy();

		//! Allow this object to gain focus when selected.
		/*!
			\param value Enable or disable focus.
		*/
		void SetCanHaveFocus(bool value);

		//! Can this item have focus
		/*!
			\return True if this can have focus, false otherwise.
		*/
		bool GetCanHaveFocus()const;
		
		//! Moves this object by the given amount.
		/*!
			Adds these values onto the current offset values.
			\param x Delta X.
			\param y Delta Y.
		*/
		void AddPosition(f32 x, f32 y);

		//! Sets the destination for this object relative to its parent.
		/*!
			\param rect New Destination region.
		*/
		void SetDestinationRegion(const MashGUIRect &rect);

		//! Sets the name for this object.
		/*!
			\param name Sets an identifier for this object.
		*/
		void SetName(const MashStringc &name);

		//! Returns this objects name.
		/*!
			\return Object name.
		*/
		const MashStringc& GetName()const;
		
		//! Handles raw event messages when focused.
		/*!
			The GUI manager will pass on events to this function when
			an item is focused.

			\param eventData Event data.
		*/
		virtual void OnEvent(const sInputEvent &eventData){}

		//! Enables or disables rendering for this object and its children.
		/*!
			If an object has its rendering disabled then that object will not
			be able to gain focus.

			\param enable Enables or disables rendering.
		*/
		virtual void SetRenderEnabled(bool enable);

		//! Returns if rendering is enabled.
		/*!
			\return True if rendering is enabled. False otherwise.
		*/
		bool GetRenderEnabled()const;

		//! Returns the destination region relative to its parent.
		/*!
			\return Destination region.
		*/
		const MashGUIRect& GetDestinationRegion()const;

		//! Returns true if no other object can gain focus when this item gains focus.
		/*!
			This is a value that is set in the window class.

			\return Focus lock.
		*/
		bool LockFocusWhenActivated()const;

		//! Returns true if this object is always rendered on top relative to its parent.
		/*!
			This is similar to LockFocusWhenActivated() but allows other objects to
			gain focus.

			This is a value that is set in the window class.

			\return Always on top.
		*/
		bool GetAlwaysOnTop()const;

		//! Gets the abs area this object.
		/*!
			This region is not clipped.

			\return Abs region.
		*/
		const mash::MashRectangle2& GetAbsoluteRegion()const;

		//! Gets the abs clipped area this object will be rendered to.
		/*!
			This can be used for cursor collision.

			\return Abs clipped region.
		*/
		const mash::MashRectangle2& GetAbsoluteClippedRegion()const;

		//! Calculates the local destination region + a parents scrollbar value.
		/*!
			This adds a views scrollbar values onto this object.
			\return Local virtual region.
		*/
		mash::MashRectangle2 GetLocalVirtualRegion()const;

		//! Calculates the absolute destination region + a parents scrollbar value.
		/*!
			This adds a views scrollbar values onto this object.
			\return Absolute virtual region.
		*/
		mash::MashRectangle2 GetAbsoluteVirtualRegion()const;

		//! Returns this objects parent.
		/*!
			\return Parent object.
		*/
		MashGUIComponent* GetParent()const;

		//! Allow an object to be clipped for rendering.
		/*!
			\param clip Clipping state.
		*/
		void SetClippingEnabled(bool clip);

		//! Returns the clipping state.
		/*!
			\return True if clipping is enabled, false otherwise.
		*/
		bool GetClippingEnabled()const;

		//! Returns how this object is currently clipped in its current position.
		/*!
			This is updated as region updates occur.
			\return Clip state.
		*/
		eCULL GetClipState()const;

		//! Sets the transparency state of all skins within this component and its children
		/*!
			\param state Enable or disable transparecy.
			\param alpha (0 == invisible) (255 == fully solid).
			\param affectFont Set to true if font is to go transparent too.
			\param alphaMaskThreshold Alpha values less than or equal to this value will be transparent.
		*/
		virtual void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);

		//! Sends this item to the front relative to its parent
		virtual void SendToFront();

		//! Sends this item to the back relative to its parent
		virtual void SendToBack();

		//! Enables a bounding box to be rendered equal to the destination region.
		/*!
			\param state Debug rendering state.
		*/
		void SetDrawDebugBounds(bool state);

		//! Drawing function implimented by each component.
		virtual void Draw() = 0;

		//! GUI Type.
		/*!
			\return GUI type.
		*/
		virtual eMASH_GUI_TYPE GetGUIType()const = 0;

		//! Enables or disables input events for this object.
		/*!
			Children will also inherit this value.
			\param Enable or disable input events.
		*/
		virtual void SetEventsEnabled(bool state);

		//! Are input events enabled.
		/*!
			\return True if input events are enabled, false otherwise.
		*/
		bool GetEventsEnabled()const;

		//! Does this item have input focus.
		/*!
			\return True if this object has input focus, false otherwise.
		*/
		bool GetHasFocus()const;

		//! Is the mouse currently hovering over this object.
		/*!
			\return True if the mouse is currently hovering over this object, false otherwise.
		*/
		bool GetMouseHover()const;

		//! Gets the original resting destination.
		/*!
			Used for saving a components region. This removes any parent scroll movement from the position.
			\param out The destination in its initial state.
		*/
		void GetResetDestinationRegion(MashGUIRect &out)const;

		//! Called by the GUI manager when a style change occurs.
		/*!
			This is implimented by each component so textures and fonts can be changed
			to match the new style.

			\param style New style.
		*/
		virtual void OnStyleChange(MashGUIStyle *style) = 0;

		//! Updates the absolute region.
		/*!
			This is called when needed and should never need to be called manually.
		*/
		void UpdateRegion();

		//! Called when an object is being deleted.
		void OnDelete();

		// Returns the active view in the inherited class.
		/*!
			The dimentions of the view should not be altered.
			The main use for this is easy iterating over children. Only views support children,
			however windows contain a view and therefore 'in a way' support children. So for windows
			and views, the view is returned. With tabs, only the active tab will be returned.

			\return Active view.
		*/
		virtual MashGUIView* GetView(){return 0;}

		//! Searches this element and all its children.
		/*!
			\param name Object to search for.
			\param searchChildren If this is true then the childrens children etc.. are also checked.
			\return Found object. NULL if no object was found with the given name.
		*/
		virtual MashGUIComponent* GetElementByName(const MashStringc &name, bool searchChildren = true);

		//! Gets the front most component that intersects a position.
		/*
			\param screenPos Abs position to query.
			\param disregardFocusState if true then CanHaveFocus and RenderEnabled are igorned.
			\return Front most intersecting object.
		*/
		virtual MashGUIComponent* GetClosestIntersectingChild(const mash::MashVector2 &screenPos, bool disregardFocusState = false);

		//! Gets the front most view that intersects a position.
		/*!
			\param screenPosition Abs position to query.
			\return Front most intersecting parentable object.
		*/
		virtual MashGUIView* GetClosestParentableObject(const mash::MashVector2 &screenPosition);

		//! Internal use only.
		/*!
			Called when the mouse enters or exits this components region.
			\param screenPos Mouse position.
			\param mouseHover Hover state.
		*/
		virtual void _SetMouseHover(const mash::MashVector2 &screenPos, bool mouseHover);

		//! Internal use only.
		/*!
			Called when a new parent is set.
			\param Mash parent.
		*/
		void _SetParent(MashGUIView *parent);
		
		//! Calculates the initial region with no scrollbar value added.
		/*!
			\param out Initial region.
		*/
		virtual void _GetResetDestinationRegion(MashGUIRect &out)const{}

		//! Called when focus lock is activated.
		/*!
			\param lockFocus Set the state of focus lock.
		*/
		void _SetLockFocusWhenActivated(bool lockFocus);

		void _SetAlwaysOnTop(bool enable);

		//! Gives this item focus.
		/*!
			This should never be called directly. Instead call MashGUIManager::SetFocusedElement().
			\param hasFocus Enables or disables focus.
		*/
		virtual void _SetHasFocus(bool hasFocus);
	};

	inline bool MashGUIComponent::GetAlwaysOnTop()const
	{
		return m_alwaysOnTop;
	}

	inline void MashGUIComponent::SetDrawDebugBounds(bool state)
	{
		m_drawDebugBounds = state;
	}

	inline void MashGUIComponent::_SetLockFocusWhenActivated(bool lockFocus)
	{
		m_lockFocusWhenActivated = lockFocus;
	}

	inline bool MashGUIComponent::LockFocusWhenActivated()const
	{
		return m_lockFocusWhenActivated;
	}

	inline void MashGUIComponent::SetEventsEnabled(bool state)
	{
		m_eventsEnabled = state;
	}

	inline bool MashGUIComponent::GetEventsEnabled()const
	{
		return m_eventsEnabled;
	}

	inline const MashStringc& MashGUIComponent::GetName()const
	{
		return m_elementName;
	}

	inline eCULL MashGUIComponent::GetClipState()const
	{
		return m_cullState;
	}

	inline bool MashGUIComponent::GetMouseHover()const
	{
		return m_mouseHover;
	}

	inline void MashGUIComponent::SetCanHaveFocus(bool bValue)
	{
		m_canHaveFocus = bValue;
	}

	inline bool MashGUIComponent::GetCanHaveFocus()const
	{
		return m_canHaveFocus;
	}

	inline bool MashGUIComponent::GetHasFocus()const
	{
		return m_hasFocus;
	}

	inline void MashGUIComponent::SetClippingEnabled(bool bClip)
	{
		m_clippingEnabled = bClip;
	}

	inline bool MashGUIComponent::GetClippingEnabled()const
	{
		return m_clippingEnabled;
	}

	inline const MashGUIRect& MashGUIComponent::GetDestinationRegion()const
	{
		return m_destinationRegion;
	}

	inline const mash::MashRectangle2& MashGUIComponent::GetAbsoluteRegion()const
	{
		return m_absoluteRegion;
	}

	inline const mash::MashRectangle2& MashGUIComponent::GetAbsoluteClippedRegion()const
	{
		return m_absoluteClippedRegion;
	}

	inline bool MashGUIComponent::GetRenderEnabled()const
	{
		return m_renderEnabled;
	}
}

#endif