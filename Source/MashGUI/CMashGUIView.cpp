//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIView.h"
#include "MashGUICustomRender.h"
#include "MashGUIManager.h"

namespace mash
{
	CMashGUIView::CMashGUIView(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUIView(pGUIManager, pInputManager, pParent, destination),
			m_verticalScrollBar(0), m_horizontalScrollBar(0), m_renderBackground(true),
			m_hscrollEnabled(true), m_vscrollEnabled(true), m_updateScrollbars(true), m_styleElement(styleElement),
			m_customRenderer(0)
	{
		m_verticalScrollBar = m_GUIManager->_GetGUIFactory()->CreateScrollBarView(true, this);
		m_verticalScrollBar->SetRenderEnabled(false);
		m_verticalScrollBar->RegisterReceiver(aGUIEVENT_SB_VALUE_CHANGE, MashGUIEventFunctor(&CMashGUIView::OnVerticalSBValueChange, this));

		m_horizontalScrollBar = m_GUIManager->_GetGUIFactory()->CreateScrollBarView(false, this);
		m_horizontalScrollBar->SetRenderEnabled(false);
		m_horizontalScrollBar->RegisterReceiver(aGUIEVENT_SB_VALUE_CHANGE, MashGUIEventFunctor(&CMashGUIView::OnHorizontalSBValueChange, this));
	}

	CMashGUIView::~CMashGUIView()
	{
		DetachAllChildren();

		if (m_verticalScrollBar)
		{
			m_verticalScrollBar->Detach();
			m_verticalScrollBar = 0;
		}

		if (m_horizontalScrollBar)
		{
			m_horizontalScrollBar->Detach();
			m_horizontalScrollBar = 0;
		}

		if (m_customRenderer)
		{
			m_customRenderer->Drop();
			m_customRenderer = 0;
		}
	}

	void CMashGUIView::OnChildAlwaysOnTop(MashGUIComponent *component)
	{
		m_alwaysOnTopStack.PushBack(component);
		component->SendToFront();
	}

	void CMashGUIView::OnRemoveChildAlwaysOnTop(MashGUIComponent *component)
	{
		m_alwaysOnTopStack.Erase(component);
	}

	void CMashGUIView::SetCustomRenderer(MashGUICustomRender *customRenderer)
	{
		if (m_customRenderer == customRenderer)
			return;

		if (m_customRenderer)
		{
			m_customRenderer->Drop();
			m_customRenderer = 0;
		}

		m_customRenderer = customRenderer;

		if (m_customRenderer)
			m_customRenderer->Grab();
	}

	void CMashGUIView::OnStyleChange(MashGUIStyle *style)
	{
		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			(*iter)->OnStyleChange(style);
		}
	}

	void CMashGUIView::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		MashGUIComponent::SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		
		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			(*iter)->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		}
	}
    
	void CMashGUIView::OnVerticalSBValueChange(const sGUIEvent &eventData)
	{
		//moving the children will cause them to msg this scroll bar. So we just restore the value when done.
		bool oldUpdateState = m_updateScrollbars;

		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			if ((*iter) != m_verticalScrollBar &&
				(*iter) != m_horizontalScrollBar)
			{
				(*iter)->AddPosition(0, -eventData.value);//SetDestinationRegion(dest);
			}
		}

		m_updateScrollbars = oldUpdateState;

		/*
			If an element has hidden and the user sets the slider back to zero this
			check will allow the slider to dissapear if its no longer needed
		*/
		if (m_verticalScrollBar->GetSliderValue() == 0.0f)
			m_updateScrollbars = true;
	}

	void CMashGUIView::OnHorizontalSBValueChange(const sGUIEvent &eventData)
	{
		//moving the children will cause them to msg this scroll bar. So we just restore the value when done.
		bool oldUpdateState = m_updateScrollbars;

		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			if ((*iter) != m_verticalScrollBar &&
				(*iter) != m_horizontalScrollBar)
			{
				(*iter)->AddPosition(-eventData.value, 0);
			}
		}

		m_updateScrollbars = oldUpdateState;

		if (m_verticalScrollBar->GetSliderValue() == 0.0f)
			m_updateScrollbars = true;
	}

	void CMashGUIView::OnHorizontalSBRelease(const sGUIEvent &eventData)
	{
		m_updateScrollbars = true;
	}

	f32 CMashGUIView::GetVerticalScrollAmount()const
	{
		return m_verticalScrollBar->GetSliderValue();
	}

	f32 CMashGUIView::GetHorizontalScrollAmount()const
	{
		return m_horizontalScrollBar->GetSliderValue();
	}
    
    bool CMashGUIView::IsHorizontalScrollInUse()const
    {
        return (m_horizontalScrollBar && m_hscrollEnabled && m_horizontalScrollBar->GetRenderEnabled());
    }
    
    bool CMashGUIView::IsVerticalScrollInUse()const
    {
        return (m_verticalScrollBar && m_vscrollEnabled && m_verticalScrollBar->GetRenderEnabled());
    }

	void CMashGUIView::SetRenderBackgroundState(bool state)
	{
		m_renderBackground = state;
	}

	void CMashGUIView::SetVerticalScrollState(bool state)
	{
		if (state != m_vscrollEnabled)
			m_updateScrollbars = true;

		m_vscrollEnabled = state;
	}

	void CMashGUIView::SetHorizontalScrollState(bool state)
	{
		if (state != m_hscrollEnabled)
			m_updateScrollbars = true;

		m_hscrollEnabled = state;
	}

	void CMashGUIView::_SendChildToFront(MashGUIComponent *pChild)
	{
		if (m_children.Front() != pChild)
		{
			MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
			MashList<MashGUIComponent*>::Iterator end = m_children.End();
			for(; iter != end; ++iter)
			{
				if ((*iter) == pChild)
				{
					m_children.Erase(iter);
					m_children.Insert(m_children.Begin(), pChild);

					if (pChild != m_verticalScrollBar && pChild != m_horizontalScrollBar)
					{
						//scroll bars are always on top
						_SendChildToFront(m_verticalScrollBar);
						_SendChildToFront(m_horizontalScrollBar);
					}

					break;
				}
			}
		}
	}

	void CMashGUIView::_SendChildToBack(MashGUIComponent *pChild)
	{
		if (m_children.Back() != pChild)
		{
			MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
			MashList<MashGUIComponent*>::Iterator end = m_children.End();
			for(; iter != end; ++iter)
			{
				if ((*iter) == pChild)
				{
					m_children.Erase(iter);
					m_children.Insert(m_children.End(), pChild);
					break;
				}
			}
		}
	}

	void CMashGUIView::SendChildToFront(MashGUIComponent *pChild)
	{
		_SendChildToFront(pChild);

		if (!m_alwaysOnTopStack.Empty())
		{
			MashList<MashGUIComponent*>::Iterator topIter = m_alwaysOnTopStack.Begin();
			MashList<MashGUIComponent*>::Iterator topEnd = m_alwaysOnTopStack.End();
			for(; topIter != topEnd; ++topIter)
			{
				//avoid recursion
				if (pChild != (*topIter))
					_SendChildToFront(*topIter);
			}
		}
	}

	void CMashGUIView::SendChildToBack(MashGUIComponent *pChild)
	{
		_SendChildToBack(pChild);
	}

	MashGUIComponent* CMashGUIView::GetElementByName(const MashStringc &name, bool searchChildren)
	{
		MashGUIComponent *element = MashGUIComponent::GetElementByName(name, searchChildren);
		if (element)
			return element;

		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			if (searchChildren)
			{
				MashGUIComponent *element = (*iter)->GetElementByName(name, searchChildren);
				if (element)
					return element;
			}
			else
			{
				if ((*iter)->GetName() == name)
					return *iter;
			}
		}

		return 0;
	}

	void CMashGUIView::OnAddChildFromConstructor(MashGUIComponent *component)
	{
		_AddChild(component);
		//drops the extra copy so that the parent becomes the only owner
		component->Drop();
	}

	void CMashGUIView::AddChild(MashGUIComponent *pChild)
	{
		_AddChild(pChild);
	}

	MashGUIComponent* CMashGUIView::GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren)
	{
		//test children first
		if (bTestAllChildren || (GetRenderEnabled() && GetCanHaveFocus() && (m_cullState != aCULL_CULLED)))
		{
			MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
			MashList<MashGUIComponent*>::Iterator end = m_children.End();
			for(; iter != end; ++iter)
			{
				MashGUIComponent *pIntersects = (*iter)->GetClosestIntersectingChild(vScreenPos, bTestAllChildren);

				if (pIntersects)
					return pIntersects;
			}
			
			//now test this rect
			MashGUIComponent *pIntersects = MashGUIComponent::GetClosestIntersectingChild(vScreenPos, bTestAllChildren);
			if (pIntersects)
				return pIntersects;
		}

		return 0;
	}

	MashGUIView* CMashGUIView::GetClosestParentableObject(const mash::MashVector2 &screenPosition)
	{
		if (GetRenderEnabled() && GetCanHaveFocus())
		{
			MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
			MashList<MashGUIComponent*>::Iterator end = m_children.End();
			for(; iter != end; ++iter)
			{
				if (m_horizontalScrollBar == *iter)
					continue;
				if (m_verticalScrollBar == *iter)
					continue;

				MashGUIView *pIntersects = (*iter)->GetClosestParentableObject(screenPosition);

				if (pIntersects)
					return pIntersects;
			}
			
			//now test this rect
			MashGUIView *pIntersects = MashGUIComponent::GetClosestParentableObject(screenPosition);
			if (pIntersects)
				return pIntersects;
		}

		return 0;
	}
    
	void CMashGUIView::DetachAllChildren()
	{
		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		while(iter != m_children.End())
		{
			if ((*iter != m_verticalScrollBar) && (*iter != m_horizontalScrollBar))
			{
				MashList<MashGUIComponent*>::Iterator tempIter = iter;
				++tempIter;
				(*iter)->Detach();
				iter = tempIter;
			}
			else
			{
				++iter;
			}
		}

		m_alwaysOnTopStack.Clear();
	}

	bool CMashGUIView::DetachChild(MashGUIComponent *pChild)
	{
		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			if (pChild == *iter)
			{
				pChild->_SetParent(0);
				m_children.Erase(iter);

				if (pChild->GetAlwaysOnTop())
					m_alwaysOnTopStack.Erase(pChild);

				//update the scroll bar position
				m_updateScrollbars = true;

				return pChild->Drop();
			}
		}

		return false;
	}

	void CMashGUIView::_GetResetDestinationRegion(MashGUIRect &out)const
	{
		if (m_verticalScrollBar->GetRenderEnabled())
		{
			out.bottom.offset += m_verticalScrollBar->GetSliderValue();
			out.top.offset += m_verticalScrollBar->GetSliderValue();
		}

		if (m_horizontalScrollBar->GetRenderEnabled())
		{
			out.left.offset += m_horizontalScrollBar->GetSliderValue();
			out.right.offset += m_horizontalScrollBar->GetSliderValue();
		}

		if (m_parent && m_parent->GetView() && (m_parent->GetView() != this))
			m_parent->GetView()->_GetResetDestinationRegion(out);
	}

	void CMashGUIView::OnChildRegionChange(MashGUIComponent *component)
	{
		if (component != m_verticalScrollBar && component != m_horizontalScrollBar)
			m_updateScrollbars = true;
	}

	void CMashGUIView::UpdateScrollbars()
	{
		/*
			If there are only two children, then they must be the scroll bars.
			In which case we disable the scrollbars.
		*/
		if (m_children.Size() == 2)
		{
			m_verticalScrollBar->ResetSlider();
			m_verticalScrollBar->SetRenderEnabled(false);

			m_horizontalScrollBar->ResetSlider();
			m_horizontalScrollBar->SetRenderEnabled(false);

			return;
		}

		//update the scroll bar position
		mash::MashRectangle2 totalRegion = m_absoluteRegion;

		if (m_vscrollEnabled)
			totalRegion.right -= m_verticalScrollBar->GetAbsoluteClippedRegion().right - m_verticalScrollBar->GetAbsoluteClippedRegion().left;
		if (m_hscrollEnabled)
			totalRegion.bottom -= m_horizontalScrollBar->GetAbsoluteClippedRegion().bottom - m_horizontalScrollBar->GetAbsoluteClippedRegion().top;

		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			//merge all objects except the scrollbars
			if ((m_verticalScrollBar != *iter) && 
				(m_horizontalScrollBar != *iter) &&
				((*iter)->GetRenderEnabled()))
			{
				totalRegion.MergeGUI((*iter)->GetAbsoluteRegion());
			}
		}
		

		/*
			Clamp to top left. Movement of the scrollbars should only be
			when moving right or down
		*/
		totalRegion.left = m_absoluteRegion.left;
		totalRegion.top = m_absoluteRegion.top;

		//cache some data
		/*
			The current scrollbar value is added to the range so that the bar stays visible
			until it has been moved back to the top. Otherwise if the view is currently big enough to view everything,
			the child objects at the top will not be visible and we will not have a way to slide back up.
		*/
		f32 vnewRange = (totalRegion.bottom - totalRegion.top) + m_verticalScrollBar->GetSliderValue();
		f32 vdestRange = m_absoluteRegion.bottom - m_absoluteRegion.top;
		bool vupdateMaxSliderVal = false;
		if (m_vscrollEnabled)
		{
			if (vnewRange > vdestRange)
			{
				if (m_absoluteRegion.top <= (totalRegion.top + m_verticalScrollBar->GetSliderValue()))
				{
					m_verticalScrollBar->SetRenderEnabled(true);
					vupdateMaxSliderVal = true;
				}
				else if (m_verticalScrollBar->GetSliderValue() == 0.0f)
				{
					m_verticalScrollBar->SetRenderEnabled(false);
				}
			}
			else
			{
				m_verticalScrollBar->SetRenderEnabled(false);
			}
		}
		else
		{
			m_verticalScrollBar->SetRenderEnabled(false);
		}

		f32 hnewRange = (totalRegion.right - totalRegion.left) + m_horizontalScrollBar->GetSliderValue();
		f32 hdestRange = m_absoluteRegion.right - m_absoluteRegion.left;
		bool hupdateMaxSliderVal = false;
		if (m_hscrollEnabled)
		{
			if (hnewRange > hdestRange)
			{
				if (m_absoluteRegion.left <= (totalRegion.left + m_horizontalScrollBar->GetSliderValue()))
				{
					m_horizontalScrollBar->SetRenderEnabled(true);
					hupdateMaxSliderVal = true;
				}
				else if (m_horizontalScrollBar->GetSliderValue() == 0.0f)
				{
					m_horizontalScrollBar->SetRenderEnabled(false);
				}
			}
			else
			{
				m_horizontalScrollBar->SetRenderEnabled(false);
			}
		}
		else
		{
			m_horizontalScrollBar->SetRenderEnabled(false);
		}

		//this bit was delayed cause we first needed to determine each scrollbars state
		if (vupdateMaxSliderVal)
		{
			/*
				Reduce the sliding range if the other scroll bar is enabled
			*/
			if (m_horizontalScrollBar->GetRenderEnabled())
				vdestRange -= m_horizontalScrollBar->GetAbsoluteClippedRegion().bottom - m_horizontalScrollBar->GetAbsoluteClippedRegion().top;

			m_verticalScrollBar->SetSliderMaxValue(vnewRange - vdestRange);
		}

		if (hupdateMaxSliderVal)
		{
			if (m_verticalScrollBar->GetRenderEnabled())
				hdestRange -= m_verticalScrollBar->GetAbsoluteClippedRegion().right - m_verticalScrollBar->GetAbsoluteClippedRegion().left;

			m_horizontalScrollBar->SetSliderMaxValue(hnewRange - hdestRange);
		}

		//if both bars are enabled then we need to resize them so they dont overlap
		if (m_verticalScrollBar->GetRenderEnabled() && m_horizontalScrollBar->GetRenderEnabled())
		{
			m_verticalScrollBar->SetDualScrollEnabled(true);
			m_horizontalScrollBar->SetDualScrollEnabled(true);
		}
		else
		{
			m_verticalScrollBar->SetDualScrollEnabled(false);
			m_horizontalScrollBar->SetDualScrollEnabled(false);
		}

		m_updateScrollbars = false;
	}

	bool CMashGUIView::_AddChild(MashGUIComponent *pChild)
	{
		if (pChild == this)
			return false;

		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			if (pChild == *iter)
				return false;
		}

		pChild->Grab();
		pChild->Detach();
		pChild->_SetParent(this);
		pChild->UpdateRegion();
		m_children.PushFront(pChild);

		if (pChild->GetAlwaysOnTop())
			m_alwaysOnTopStack.PushBack(pChild);

		//make the child inherit the alpha override
		pChild->SetOverrideTransparency(m_overrideTransparency.enableOverrideTransparency,
			m_overrideTransparency.alphaValue,
			m_overrideTransparency.affectFontAlpha,
			m_overrideTransparency.alphaMaskThreshold);

		if (m_verticalScrollBar && m_vscrollEnabled)
		{
			//make sure the scollbars are on top
			m_verticalScrollBar->SendToFront();
		}

		if (m_horizontalScrollBar && m_hscrollEnabled)
		{
			//make sure the scollbars are on top
			m_horizontalScrollBar->SendToFront();
		}

		//update the scroll bar position
		m_updateScrollbars = true;
		return true;
	}

	void CMashGUIView::SetEventsEnabled(bool state)
	{
		MashGUIComponent::SetEventsEnabled(state);

		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			(*iter)->SetEventsEnabled(state);
		}
	}

	void CMashGUIView::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (m_updateScrollbars)
			positionChangeOnly = false;

		/*
			In order for everything to play nicely, we need to reset all children back to their
			original position, reset the scollbars, update all regions, then move the slider
			back to its last position.
		*/
		MashList<MashGUIComponent*>::Iterator iter = m_children.Begin();
		MashList<MashGUIComponent*>::Iterator end = m_children.End();
		for(; iter != end; ++iter)
		{
			if ((*iter) == m_verticalScrollBar)
				m_verticalScrollBar->UpdateRegion();
			else if ((*iter) == m_horizontalScrollBar)
				m_horizontalScrollBar->UpdateRegion();
			else
			{
				(*iter)->AddPosition(m_horizontalScrollBar->GetSliderValue(), m_verticalScrollBar->GetSliderValue());
			}
		}

		f32 verticalVal = m_verticalScrollBar->GetSliderValue();
		f32 horizontalVal = m_horizontalScrollBar->GetSliderValue();
		m_verticalScrollBar->ResetSlider();
		m_horizontalScrollBar->ResetSlider();

		if (!positionChangeOnly)
			UpdateScrollbars();

		m_verticalScrollBar->MoveSlider(verticalVal);
		m_horizontalScrollBar->MoveSlider(horizontalVal);

		m_updateScrollbars = false;
	}
    
    void CMashGUIView::OnEvent(const sInputEvent &eventData)
    {
		if (eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE)
        {
            switch(eventData.action)
            {
                case aMOUSEEVENT_AXISZ:
                {
					//only auto scroll if the horizontal bar is not in focus
					if (m_horizontalScrollBar && m_verticalScrollBar->GetRenderEnabled() && !m_horizontalScrollBar->GetHasFocus())
					{
						if (m_verticalScrollBar && m_verticalScrollBar->GetRenderEnabled() && !m_verticalScrollBar->GetHasFocus())
							m_verticalScrollBar->OnEvent(eventData);
					}

                    break;
                }
            }
        }
    }

	void CMashGUIView::Draw()
	{
		MashGUIComponent::Draw();

		if (m_updateScrollbars)
		{
			UpdateScrollbars();
		}

		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *viewSkin = (MashGUISkin*)activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND);
			
			/*
				We removed the const from GUI skin so we can edit the boarder value.
				Views need to render the board after all its children so that it
				appears on top.
			*/
			bool oldDrawBoarderVal = false;
			
			if (viewSkin)
			{
				oldDrawBoarderVal = viewSkin->renderBoarder;
				viewSkin->renderBoarder = false;
			}
			
			if (m_renderBackground)
				m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, viewSkin, m_overrideTransparency);

			if (viewSkin)
				viewSkin->renderBoarder = oldDrawBoarderVal;

			if (m_customRenderer)
				m_customRenderer->Draw(this);

			if (!m_children.Empty())
			{
				MashList<MashGUIComponent*>::Iterator iterBegin = m_children.Begin();
				for(MashList<MashGUIComponent*>::Iterator iter = m_children.BackIterator();;--iter)
				{
					(*iter)->Draw();

					if (iter == iterBegin)
						break;
				}
			}

			if (m_renderBackground && viewSkin && viewSkin->renderBoarder)
			{
				sMashColour borderColour = viewSkin->borderColour;
				if (m_overrideTransparency.enableOverrideTransparency)
					borderColour.SetAlpha(m_overrideTransparency.alphaValue);

				m_GUIManager->DrawBorder(m_absoluteClippedRegion, borderColour);
			}

			/*
				Buffers need to be flushed at this point to avoid overlaps.
			*/
			m_GUIManager->FlushBuffers();
		}
	}

}