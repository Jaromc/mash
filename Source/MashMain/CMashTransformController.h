//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_TRANSFORM_CONTROLLER_H_	
#define _CMASH_TRANSFORM_CONTROLLER_H_

#include "MashAnimationController.h"
#include "MashKeySet.h"
namespace mash
{
		
	class MashSceneNode;

	//one controller per scene node instance.
	class CMashTransformationController : public MashTransformationController
	{
	private:
		struct sTransformationData
		{
			sTransformationData():scale(1.0f, 1.0f, 1.0f),
				position(),rotation(){}

			mash::MashVector3 scale;
			mash::MashVector3 position;
			mash::MashQuaternion rotation;
		};
	private:
		mash::MashSceneNode *m_owner;//instance data
		MashTransformationKeySet *m_keySet;//shared among instances
		sTransformationData m_initialState;//bind pose
		sTransformationData m_transformBuffer;
	public:
		CMashTransformationController(MashTransformationKeySet *sharedKeys, mash::MashSceneNode *instance);
		~CMashTransformationController();

		const MashKeySetInterface* GetKeySet()const;
		void AnimationStart();
		void AnimateToKey(uint32 key);
		void AnimateForward(uint32 fromKey, uint32 toKey, f32 amount);
		void AnimationEnd(eANIMATION_BLEND_MODE blend, f32 blendAmount);
		eMASH_CONTROLLER_TYPE GetControllerType()const;
		MashSceneNode* GetOwner()const;
	};

	inline eMASH_CONTROLLER_TYPE CMashTransformationController::GetControllerType()const
	{
		return aCONTROLLER_TRANSFORMATION;
	}

	inline const MashKeySetInterface* CMashTransformationController::GetKeySet()const
	{
		return m_keySet;
	}
}

#endif