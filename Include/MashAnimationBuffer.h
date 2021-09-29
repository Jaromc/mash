//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ANIMATION_BUFFER_H_
#define _MASH_ANIMATION_BUFFER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashString.h"
#include "MashArray.h"
#include <map>

namespace mash
{
	class MashControllerManager;
	class MashKeySetInterface;
    struct sAnimationClip;
	
	/*!
		This class holds all the animation data for an animated object. This buffer can be assigned to a
        single node, or assigned to many nodes that have the same animation data, this helps to save on
        memory consumption. Animation data is then instanced out per object using an animation mixer.
     
		Animation buffers can be created from MashControllerManager::CreateAnimationBuffer() then should
        be assigned to the scene node(s) using it.
	*/
	class MashAnimationBuffer : public MashReferenceCounter
	{
	public:
		struct sController
		{
			/*
				This data is also shared with the animation mixers.
			*/
			MashKeySetInterface *keySet;
			eMASH_CONTROLLER_TYPE controllerType;

			sController():keySet(0), controllerType(aCONTROLLER_TRANSFORMATION){}
			sController(MashKeySetInterface *_keySet, eMASH_CONTROLLER_TYPE _controllerType):keySet(_keySet), controllerType(_controllerType){}
		};
	public:
		MashAnimationBuffer():MashReferenceCounter(){}
		virtual ~MashAnimationBuffer(){}

		//! Adds an animation key set.
		/*!
			An animation name is something like "walk", "run", etc...
			There may be many controllers per animation. For example lights may have
			a node transformation controller, and another that controls the light colours.

			\param animationName Animation name.
			\param destinationType Controller type.
			\param keySet Object that holds all the keys.
			\return Ok if everything was fine, failed otherwise.
		*/
		virtual eMASH_STATUS AddAnimationKeySet(const MashStringc &animationName, eMASH_CONTROLLER_TYPE destinationType, MashKeySetInterface *keySet) = 0;

		//! Returns all animations.
		/*!
			\return Animation map. This should not be modified.
		*/
		virtual const std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >& GetAnimationKeySets()const = 0;
		
		//! Chops a buffer into smaller buffers.
		/*!
			Transforms one large animation buffer into smaller buffers, such as walk, run, jump, etc..
			This must be used before an animation mixer uses it otherwise the mixer will need
			to be recreated.

			\param manager Controller manager.
			\param clips These describe how to chop this buffer into new smaller buffers.
			\param out New buffers will be placed here. This should be a NULL array.
		*/
		virtual eMASH_STATUS ChopBuffer(MashControllerManager *manager, const MashArray<sAnimationClip> &clips, MashAnimationBuffer **out)const = 0;

		//! Changes the name of an animation.
		/*!
			This must be used before an animation mixer uses it otherwise the mixer will need
			to be recreated.

			\param from Animation name to change.
			\param to New name.
		*/
		virtual void SetAnimationName(const MashStringc &from, const MashStringc &to) = 0;

		//! Gets the frame length of an animation.
		/*!
			Helper function. 
			Loops through each controller and find the frame bounds for an animation.

			\param animationName Animation to query.
			\param startFrame The start frame found will be placed here.
			\param endFrame The end frame found will be placed here.
		*/
		virtual void GetAnimationFrameBounds(const MashStringc &animationName, uint32 &startFrame, uint32 &endFrame)const = 0;
	};
}

#endif