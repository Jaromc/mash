//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_CONTROLLER_MANAGER_H_
#define _C_MASH_CONTROLLER_MANAGER_H_

#include "MashControllerManager.h"
#include <queue>
#include <set>
namespace mash
{
	/*
		This is needed because multiple entities may transform based on
		a single controller, and it would be silly to update the same
		controller multiple times. So controllers are keeped here, in one place,
		to facilitate fast set/resetting of updated flags for each controller.
	*/
	class CMashControllerManager : public MashControllerManager
	{
	private:
		bool IsElementValid(int32 iHandle);

		std::set<MashAnimationMixer*> m_animationMixers;

		/*
			If mixer == NULL then a new mixer is created and set for the root and each of its
			children. Otherwise all animations are stored in the mixer passed in.
		*/
		void ProcessNodeTreeAndAddToMixer(MashAnimationMixer *mixer, MashSceneNode *root);
	public:
		CMashControllerManager();
		~CMashControllerManager();

		eMASH_STATUS ChopAnimationBuffers(MashSceneNode *root, const MashArray<sAnimationClip> &clips, bool processChildren = true);
		MashAnimationBuffer* CreateAnimationBuffer();

		MashTransformationController* CreateTransformController(MashTransformationKeySet *keys, MashSceneNode *node);
		MashTransformationKeySet* CreateTransformationKeySet();
		MashAnimationMixer* CreateMixer();
		MashAnimationMixer* CreateMixer(MashSceneNode *root, bool processChildren = true);

		void Update(f32 dt);

		//helper function
		void AddAnimationsToMixer(MashAnimationMixer *mixer, MashSceneNode *root);

		//called by mixers when deleted
		void _RemoveAnimationMixer(MashAnimationMixer *pController);
	};
}

#endif