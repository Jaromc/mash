//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_EVENT_TYPES_H_
#define _MASH_EVENT_TYPES_H_

#include "MashEventDispatch.h"

namespace mash
{
    class MashAnimationMixer;
    
	struct sInputEvent
	{
		enum eEVENT_TYPE
		{
			aEVENTTYPE_KEYBOARD,
			aEVENTTYPE_MOUSE,
			aEVENTTYPE_JOYSTICK,
			aEVENTTYPE_CONTROLLER_CONNECT,
			aEVENTTYPE_CONTROLLER_DISCONNECT,
			aEVENTTYPE_CONTROLLER_CONTEXT_CHANGE
		};

		eEVENT_TYPE eventType;

		uint16 controllerID;
		int8 character;//ASCII value
		int8 isPressed;//1 if pressed. 0 otherwise.

		eINPUT_EVENT action;
		//this value is valid if its been mapped
		uint32 actionID;//user defined action < aINPUTMAP_MAX
		f32 value;//axis value.
	};

	struct sAnimationEvent
	{
		MashAnimationMixer *mixer;
		const int8 *animationName;
		int32 frame;
		int32 userData;
	};

	struct sLogEvent
	{
		int32 level;//type MashLog::eERROR_LEVEL
		const int8 *msg;
	};

	typedef MashFunctor<const sInputEvent> MashInputEventFunctor;
	typedef MashFunctor<const sAnimationEvent> MashAnimationEventFunctor;
	typedef MashFunctor<const sLogEvent> MashLogEventFunctor;
}

#endif