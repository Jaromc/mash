//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIViewport.h"
#include "MashDevice.h"

namespace mash
{
	CMashGUIViewport::CMashGUIViewport(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			mash::MashVideo *renderer):MashGUIViewport(pGUIManager, pInputManager, pParent, destination), m_renderer(renderer)
	{
		m_originalViewport = m_renderer->GetViewport();
	}

	CMashGUIViewport::~CMashGUIViewport()
	{
	}

	void CMashGUIViewport::SetInputEventCallback(const MashInputEventFunctor &callback)
	{
		m_inputEventCallback = callback;
	}

	void CMashGUIViewport::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if ((eventData.eventType == sInputEvent::aEVENTTYPE_KEYBOARD) || 
				(eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE) ||
				(eventData.eventType == sInputEvent::aEVENTTYPE_JOYSTICK) ||
				(eventData.eventType == sInputEvent::aEVENTTYPE_CONTROLLER_CONNECT) ||
				(eventData.eventType == sInputEvent::aEVENTTYPE_CONTROLLER_DISCONNECT))
			{
				//forward messages
				m_inputEventCallback.Call(eventData);
			}
		}
	}

	void CMashGUIViewport::OnFocusGained()
	{
		SendToBack();
	}

	void CMashGUIViewport::GetViewportWidthHeight(f32 &x, f32 &y)const
	{
		x = GetAbsoluteRegion().right - GetAbsoluteRegion().left;
		y = GetAbsoluteRegion().bottom - GetAbsoluteRegion().top;
	}

	void CMashGUIViewport::GetViewport(sMashViewPort &out)
	{
		out.x = GetAbsoluteRegion().left;
		out.y = GetAbsoluteRegion().top;
		out.width = GetAbsoluteRegion().right - GetAbsoluteRegion().left;
		out.height = GetAbsoluteRegion().bottom - GetAbsoluteRegion().top;
		out.minZ = 0.0f;
		out.maxZ = 1.0f;
	}

	void CMashGUIViewport::SetViewport()
	{
		m_originalViewport = m_renderer->GetViewport();

		sMashViewPort viewPort;
		viewPort.x = GetAbsoluteRegion().left;
		viewPort.y = GetAbsoluteRegion().top;
		viewPort.width = GetAbsoluteRegion().right - GetAbsoluteRegion().left;
		viewPort.height = GetAbsoluteRegion().bottom - GetAbsoluteRegion().top;
		viewPort.minZ = 0.0f;
		viewPort.maxZ = 1.0f;

		m_renderer->SetViewport(viewPort);
	}

	void CMashGUIViewport::RestoreOriginalViewport() 
	{
		m_renderer->SetViewport(m_originalViewport);
	}

	void CMashGUIViewport::Draw()
	{
		MashGUIComponent::Draw();
	}
}