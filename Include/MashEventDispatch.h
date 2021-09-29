//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_EVENT_DISPATCH_H_
#define _MASH_EVENT_DISPATCH_H_

#include "MashDataTypes.h"
#include "MashArray.h"
#include "MashFunctor.h"

namespace mash
{
    /*!
        Derived from class for event handling.
    */
	template<class TEventType>
	class MashEventDispatch
	{
	public:
		typedef MashFunctor<TEventType> MashEventFunctor;
	private:
		struct sReceiver
		{
		public:
			uint32 id;
			MashEventFunctor callback;

			sReceiver():id(0), callback(){}
			sReceiver(uint32 _id, MashEventFunctor &_callback):id(_id), callback(_callback){}
			sReceiver(const sReceiver &copy):id(copy.id), callback(copy.callback){}
		};

		/*
			Its expected that receivers will not be removed often/or at 
			all at run time. They are kept in an array for better
			efficiency
		*/
		MashArray<sReceiver> m_receivers;
		uint32 m_receiverIdCounter;
		bool m_isMute;
	public:
		MashEventDispatch():m_isMute(false), m_receiverIdCounter(0){}
		~MashEventDispatch(){}

        //! Sends a message to all receivers.
		void ImmediateBroadcast(TEventType &eventData)
		{
			if (!m_isMute && !m_receivers.Empty())
			{
				typename MashArray<sReceiver>::Iterator recIter = m_receivers.Begin();
				typename MashArray<sReceiver>::Iterator recEnd = m_receivers.End();
				for(; recIter != recEnd; ++recIter)
					recIter->callback.Call(eventData);
			}
		}

        //! Returns true if this dispatch is not currently passing messages on to its revceivers.
		bool IsMute()const
		{
			return m_isMute;
		}
        
        //! Stops this dispatch from passing on messages to its receivers.
		void SetMute(bool isMute)
		{
			m_isMute = isMute;
		}

		//! Registers a receiver that will receive messages from this dispatch.
        /*!
             \return An id that can be used for disconnection.
        */
		uint32 RegisterReceiver(MashEventFunctor callback)
		{
			m_receivers.PushBack(sReceiver(m_receiverIdCounter++, callback));
			return m_receivers.Back().id;
		}
        
        //! Removes a receiver from this dispatch. The handle came from RegisterReceiver().
		void RemoveReceiver(uint32 handle)
		{
			typename MashArray<sReceiver>::Iterator recIter = m_receivers.Begin();
			typename MashArray<sReceiver>::Iterator recEnd = m_receivers.End();
			for(; recIter != recEnd; ++recIter)
			{
				if (recIter->id == handle)
				{
					m_receivers.Erase(recIter);
					return;
				}
			}
		}
        
        //! Removes all receivers.
		void RemoveAllRecivers()
		{
			m_receivers.Clear();
		}
	};
}

#endif