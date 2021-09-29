//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_EVENET_DISPATCH_H_
#define _MASH_GUI_EVENET_DISPATCH_H_

#include "MashFunctor.h"
#include "MashGUITypes.h"
#include "MashArray.h"
#include <map>

namespace mash
{
	/*!
		This is the message hub for each GUI component. GUI components derive from this class
		to give them event driven capabilities.

		It is very similar to the event dispatch in the main engine except for this class
		uses a map to store receivers based on the message they wish to receive.
	*/
	class MashGUIEventDispatch
	{
	private:
		struct sReceiver
		{
			uint32 id;
			MashGUIEventFunctor callback;

			sReceiver():id(0), callback(){}
			sReceiver(uint32 _id, MashGUIEventFunctor &_callback):id(_id), callback(_callback){}
		};
	protected:
		std::map<uint32, MashArray<sReceiver> > m_receivers;
		uint32 m_receiverIdCounter;
		bool m_isMute;
		static bool m_globalMute;
	public:
		MashGUIEventDispatch();
		~MashGUIEventDispatch();

		//! Is this dispatch muted.
		/*!
			\return Is this dispatch muted.
		*/
		bool IsMute()const;

		//! Sets the mute state.
		/*!
			This will stop all messages from being sent to the receivers.
			\param isMute Mute state.
		*/
		void SetMute(bool isMute);

		//! Broadcasts an event to all receivers.
		/*!
			\param eventData Data to send.
		*/
		void ImmediateBroadcast(const sGUIEvent &eventData);
		
		//! Registers receivers for this object.
		/*!
			\param eventType event to register for.
			\param callback Callback to send any messages to for the given event.
			\return Id that can be used for disconnection.
		*/
		uint32 RegisterReceiver(uint32 eventType, MashGUIEventFunctor callback);

		//! Removes a receiver from this object.
		/*!
			\param handle The Id of the receiver to remove. This is from RegisterReceiver().
		*/
		void RemoveReceiver(uint32 handle);

		//! Removes all receivers from this object.
		void RemoveAllRecivers();

		//! Static object that mutes all meesages for ALL objects.
		/*!
			\param val Mute state.
		*/
		static void SetGlobalMute(bool val);
	};

	inline void MashGUIEventDispatch::SetGlobalMute(bool val)
	{
		m_globalMute = val;
	}

	inline bool MashGUIEventDispatch::IsMute()const
	{
		return m_isMute;
	}

	inline void MashGUIEventDispatch::SetMute(bool isMute)
	{
		m_isMute = isMute;
	}
}

#endif