//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_ANIMATION_BUFFER_H_
#define _C_MASH_ANIMATION_BUFFER_H_

#include "MashAnimationBuffer.h"
#include "MashTypes.h"

namespace mash
{
	class CMashAnimationBuffer : public MashAnimationBuffer
	{
	private:
		std::map<MashStringc, MashArray<sController> > m_animations;
	public:
		CMashAnimationBuffer(){}
		~CMashAnimationBuffer();

		eMASH_STATUS AddAnimationKeySet(const MashStringc &animationName, eMASH_CONTROLLER_TYPE destinationType, MashKeySetInterface *keySet);
		const std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >& GetAnimationKeySets()const;
		eMASH_STATUS ChopBuffer(MashControllerManager *manager, const MashArray<sAnimationClip> &clips, MashAnimationBuffer **out)const;
		void SetAnimationName(const MashStringc &from, const MashStringc &to);

		void GetAnimationFrameBounds(const MashStringc &animationName, uint32 &startFrame, uint32 &endFrame)const;
	};
	
	inline const std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >& CMashAnimationBuffer::GetAnimationKeySets()const
	{
		return m_animations;	
	}
}

#endif