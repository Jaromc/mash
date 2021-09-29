//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ANIMATION_KEY_H_
#define _MASH_ANIMATION_KEY_H_

#include "MashVector3.h"
#include "MashQuaternion.h"

namespace mash
{
	/*!
		Keys used for animation key sets.
		All keys types must derive from sMashAnimationKey.
	*/
	struct sMashAnimationKey
	{
		uint32 frame;
	};

	struct sMashAnimationKeyTransform : public sMashAnimationKey
	{
		MashVector3 positionKey;
		MashVector3 scaleKey;
		MashQuaternion rotationKey;
	};
}

#endif