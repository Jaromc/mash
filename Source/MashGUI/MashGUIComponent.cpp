//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashGUIComponent.h"
#include "MashGUIManager.h"
#include "MashInputManager.h"
#include "MashEventTypes.h"

namespace mash
{
	MashGUIComponent::MashGUIComponent(
			MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):
		m_GUIManager(pGUIManager),
		m_renderEnabled(true),m_parent(0),
		m_clippingEnabled(true), m_cullState(aCULL_VISIBLE),
		m_hasFocus(false), m_canHaveFocus(true),
		m_mouseHover(false), m_inputManager(pInputManager), m_eventsEnabled(true),
		m_lockFocusWhenActivated(false), m_drawDebugBounds(false),
		m_positionChangeX(0), m_positionChangeY(0), m_updateFlags(aGUI_UPDATE_REGION),
		m_lastParentPosition(0.0f, 0.0f), m_alwaysOnTop(false)
	{
		m_destinationRegion = destination;

		if (pParent)
			pParent->OnAddChildFromConstructor(this);
		else
			UpdateRegion();
	}

	MashGUIComponent::~MashGUIComponent()
	{
		m_GUIManager->OnDestroyElement(this);
	}

	void MashGUIComponent::OnAddChildFromConstructor(MashGUIComponent *component)
	{
		component->m_parent = this;
		component->UpdateRegion();
	}

	void MashGUIComponent::_SetHasFocus(bool bHasFocus)
	{
		if (bHasFocus && !m_hasFocus && GetCanHaveFocus())
		{
			m_hasFocus = true;
			OnFocusGained();

			sGUIEvent newGUIMsg;
			
			newGUIMsg.GUIEvent = aGUIEVENT_INPUTFOCUS;
			newGUIMsg.component = this;
			ImmediateBroadcast(newGUIMsg);			
		}
		else if (!bHasFocus && m_hasFocus && GetCanHaveFocus())
		{
			m_hasFocus = false;
			OnLostFocus();

			sGUIEvent newGUIMsg;
			
			newGUIMsg.GUIEvent = aGUIEVENT_LOST_INPUTFOCUS;
			newGUIMsg.component = this;
			ImmediateBroadcast(newGUIMsg);
		}
		
		if (m_hasFocus)
			SendToFront();
	}

	void MashGUIComponent::_SetMouseHover(const mash::MashVector2 &vScreenPos, bool bMouseHover)
	{
		if (bMouseHover && !m_mouseHover && GetCanHaveFocus())
		{
			m_mouseHover = true;

			sGUIEvent newGUIMsg;
			
			newGUIMsg.GUIEvent = aGUIEVENT_MOUSE_ENTER;
			newGUIMsg.component = this;
			ImmediateBroadcast(newGUIMsg);

			OnMouseEnter(vScreenPos);
		}
		else if (!bMouseHover && m_mouseHover && GetCanHaveFocus())
		{
			m_mouseHover = false;

			sGUIEvent newGUIMsg;
			
			newGUIMsg.GUIEvent = aGUIEVENT_MOUSE_EXIT;
			newGUIMsg.component = this;
			ImmediateBroadcast(newGUIMsg);

			OnMouseExit(vScreenPos);
		}
	}

	MashGUIComponent* MashGUIComponent::GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren)
	{
		bool bTest = bTestAllChildren || (GetRenderEnabled() && GetCanHaveFocus() && (m_cullState != aCULL_CULLED));
		if (bTest && GetAbsoluteClippedRegion().IntersectsGUI(vScreenPos))
		{
			return this;
		}

		return 0;
	}

	MashGUIView* MashGUIComponent::GetClosestParentableObject(const mash::MashVector2 &screenPosition)
	{
		bool bTest = GetRenderEnabled() && GetCanHaveFocus() && GetView();
		if (bTest && GetAbsoluteClippedRegion().IntersectsGUI(screenPosition))
		{
			return GetView();
		}

		return 0;
	}

	void MashGUIComponent::_SetParent(MashGUIView *pParent)
	{
		m_parent = pParent;
	}

	void MashGUIComponent::_SetAlwaysOnTop(bool enable)
	{
		if (m_alwaysOnTop != enable)
		{
			m_alwaysOnTop = enable;

			if (m_parent)
			{
				if (m_alwaysOnTop)
					m_parent->OnChildAlwaysOnTop(this);
				else
					m_parent->OnRemoveChildAlwaysOnTop(this);
			}
		}
		
	}

	void MashGUIComponent::OnDelete()
	{
		m_GUIManager->_DestroyElement(this);
	}

	bool MashGUIComponent::Detach()
	{
		if (m_parent && m_parent->GetView())
		{
			MashGUIView *p = m_parent->GetView();
			m_parent = 0;
			return p->DetachChild(this);
		}

		return false;
	}

	void MashGUIComponent::Destroy()
	{
		if (!Detach())
			m_GUIManager->_DestroyElement(this);
	}
    
	void MashGUIComponent::SetRenderEnabled(bool bEnable)
	{
		if (m_renderEnabled != bEnable)
		{
			m_renderEnabled = bEnable;

			if (!m_renderEnabled)
				m_GUIManager->OnHideElement(this);
			else
				m_GUIManager->OnShowElement(this);
			
			if (m_parent)
			{
				//notify the parent so it can resize if needed
				m_parent->OnChildRegionChange(this);
			}
				
		}
	}

	mash::MashRectangle2 MashGUIComponent::GetAbsoluteVirtualRegion()const
	{
		if (!m_parent || !m_parent->GetView())
			return m_absoluteRegion;

		mash::MashRectangle2 virtualRegion;
		m_destinationRegion.GetAbsoluteValue(m_parent->GetAbsoluteRegion(), virtualRegion);
		MashGUIView *viewParent = m_parent->GetView();
		virtualRegion.left += viewParent->GetHorizontalScrollAmount();
		virtualRegion.right += viewParent->GetHorizontalScrollAmount();
		virtualRegion.top += viewParent->GetVerticalScrollAmount();
		virtualRegion.bottom += viewParent->GetVerticalScrollAmount();

		return virtualRegion;
	}

	mash::MashRectangle2 MashGUIComponent::GetLocalVirtualRegion()const
	{
		if (!m_parent || !m_parent->GetView())
			return m_absoluteRegion;

		mash::MashRectangle2 virtualRegion;
		m_destinationRegion.GetLocalValue(m_parent->GetAbsoluteRegion(), virtualRegion);
		MashGUIView *viewParent = m_parent->GetView();
		virtualRegion.left += viewParent->GetHorizontalScrollAmount();
		virtualRegion.right += viewParent->GetHorizontalScrollAmount();
		virtualRegion.top += viewParent->GetVerticalScrollAmount();
		virtualRegion.bottom += viewParent->GetVerticalScrollAmount();

		return virtualRegion;
	}

	void MashGUIComponent::AddPosition(f32 x, f32 y)
	{
		m_destinationRegion.left.offset += x;
		m_destinationRegion.right.offset += x;
		m_destinationRegion.bottom.offset += y;
		m_destinationRegion.top.offset += y;

		UpdateRegion();
	}

	void MashGUIComponent::GetResetDestinationRegion(MashGUIRect &out)const
	{
		out = m_destinationRegion;
		if (m_parent && m_parent->GetView())
			m_parent->_GetResetDestinationRegion(out);
	}

	void MashGUIComponent::SetName(const MashStringc &name)
	{
		m_elementName = name;
	}

	void MashGUIComponent::SetDestinationRegion(const MashGUIRect &rect)
	{
		m_destinationRegion = rect;

		UpdateRegion();
	}

	MashGUIComponent* MashGUIComponent::GetElementByName(const MashStringc &name, bool searchChildren)
	{
		if (GetName() == name)
			return this;

		return 0;
	}

	void MashGUIComponent::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		m_overrideTransparency.enableOverrideTransparency = state;
		m_overrideTransparency.alphaValue = alpha;
		m_overrideTransparency.affectFontAlpha = affectFont;
		m_overrideTransparency.alphaMaskThreshold = alphaMaskThreshold;
	}

	/*
		A parents resize method should call this. Not OnResize()
	*/
	void MashGUIComponent::UpdateRegion()
	{
		f32 oldAbsTop = m_absoluteRegion.top;
		f32 oldAbsLeft = m_absoluteRegion.left;

		if (m_parent)
			m_destinationRegion.GetAbsoluteValue(m_parent->GetAbsoluteRegion(), m_absoluteRegion);
		else
			m_destinationRegion.GetOffsetValue(m_absoluteRegion);

		mash::MashRectangle2 newAbsClippedRegion = m_absoluteRegion;
		m_cullState = aCULL_VISIBLE;
		
		bool sameParentPosition = true;
		if (m_parent)
		{
			m_cullState = newAbsClippedRegion.ClipGUI(m_parent->GetAbsoluteClippedRegion());

			/*
				We check the parent pos to see if it has moved. If it hasn't then this object
				has either been dragged or moved by a scrollbar.
				If it has been determined that this object is not completley visible
				and the parent has not moved then we should rebuild this object, rather
				than just reposition it.
			*/
			f32 parentPosTop = m_parent->GetAbsoluteClippedRegion().top;
			f32 parentPosLeft = m_parent->GetAbsoluteClippedRegion().left;
			sameParentPosition = (parentPosTop == m_lastParentPosition.y) && (parentPosLeft == m_lastParentPosition.x);
			m_lastParentPosition.x = parentPosLeft;
			m_lastParentPosition.y = parentPosTop;
		}

		bool sameWidth = (newAbsClippedRegion.right - newAbsClippedRegion.left) == (m_absoluteClippedRegion.right - m_absoluteClippedRegion.left);
		bool sameHeight = (newAbsClippedRegion.bottom - newAbsClippedRegion.top) == (m_absoluteClippedRegion.bottom - m_absoluteClippedRegion.top);
		bool samePosition = (m_absoluteRegion.top == oldAbsTop) && (m_absoluteRegion.left == oldAbsLeft);

		if (samePosition && sameHeight && sameWidth)
			return;//no change

		if (!sameWidth || !sameHeight || (sameParentPosition && m_cullState != aCULL_VISIBLE))
		{
			m_absoluteClippedRegion = newAbsClippedRegion;

			if (!m_updateFlags && m_parent)
				m_parent->OnChildRegionChange(this);//This should notify to update the scollbar position only. Not resize.

			m_updateFlags = aGUI_UPDATE_REGION;//override any flag set
		}
		else if (!samePosition)
		{
			m_positionChangeX += m_absoluteRegion.left - oldAbsLeft;
			m_positionChangeY += m_absoluteRegion.top - oldAbsTop;

			m_absoluteClippedRegion = newAbsClippedRegion;

			if (!m_updateFlags && m_parent)
				m_parent->OnChildRegionChange(this);

			m_updateFlags |= aGUI_UPDATE_POSITION_ONLY;
		}
	}

	void MashGUIComponent::SendToFront()
	{
		if (m_parent && m_parent->GetView())
		{
			//send this item to the front of the parent
			m_parent->GetView()->SendChildToFront(this);
			//then send the parent to the front of its parent
			m_parent->SendToFront();
		}
	}

	void MashGUIComponent::SendToBack()
	{
		if (m_parent && m_parent->GetView())
		{
			//send this item to the front of the parent
			m_parent->GetView()->SendChildToBack(this);
		}
	}

	MashGUIComponent* MashGUIComponent::GetParent()const
	{
		return m_parent;
	}

	void MashGUIComponent::Draw()
	{
		if (m_updateFlags)
		{
			bool positionChangeOnly = false;

			if ((m_updateFlags ^ aGUI_UPDATE_POSITION_ONLY) == 0)
				positionChangeOnly = true;

			OnResize(positionChangeOnly, m_positionChangeX, m_positionChangeY);
			m_positionChangeX = 0;
			m_positionChangeY = 0;
			m_updateFlags = 0;
		}

		if (m_drawDebugBounds)
		{
			if (m_renderEnabled && (m_cullState != aCULL_CULLED))
				m_GUIManager->DrawBorder(m_absoluteClippedRegion, m_GUIManager->GetDebugDrawColour());
		}
	}

}