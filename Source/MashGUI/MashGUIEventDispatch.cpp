//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashGUIEventDispatch.h"

namespace mash
{
	bool MashGUIEventDispatch::m_globalMute = false;

	MashGUIEventDispatch::MashGUIEventDispatch():m_isMute(false), m_receiverIdCounter(0)
	{
	}

	MashGUIEventDispatch::~MashGUIEventDispatch()
	{
		RemoveAllRecivers();
	}

	void MashGUIEventDispatch::ImmediateBroadcast(const sGUIEvent &eventData)
	{
		std::map<uint32, MashArray<sReceiver> >::iterator iter = m_receivers.find(eventData.GUIEvent);
		if (iter != m_receivers.end())
		{
			if (!m_globalMute && !m_isMute)
			{
				MashArray<sReceiver>::Iterator recIter = iter->second.Begin();
				MashArray<sReceiver>::Iterator recEnd = iter->second.End();
				for(; recIter != recEnd; ++recIter)
					recIter->callback.Call(eventData);
			}
		}
	}

	uint32 MashGUIEventDispatch::RegisterReceiver(uint32 eventType, MashGUIEventFunctor callback)
	{
		m_receivers[eventType].PushBack(sReceiver(m_receiverIdCounter++, callback));
		return m_receivers[eventType].Back().id;
	}

	void MashGUIEventDispatch::RemoveReceiver(uint32 handle)
	{
		/*
			this could be sped up by keeping a list of handles with event type and location info.
			But that shouldnt really be necessary as adding and removing should only be done
			during load times anyway.
		*/
		std::map<uint32, MashArray<sReceiver> >::iterator eventTypeIter = m_receivers.begin();
		std::map<uint32, MashArray<sReceiver> >::iterator eventTypeEnd = m_receivers.end();
		for(; eventTypeIter != eventTypeEnd; ++eventTypeIter)
		{
			MashArray<sReceiver>::Iterator recIter = eventTypeIter->second.Begin();
			MashArray<sReceiver>::Iterator recEnd = eventTypeIter->second.End();
			for(; recIter != recEnd; ++recIter)
			{
				if (recIter->id == handle)
				{
					eventTypeIter->second.Erase(recIter);
					return;
				}
			}
		}
	}

	void MashGUIEventDispatch::RemoveAllRecivers()
	{
		m_receivers.clear();
	}
}