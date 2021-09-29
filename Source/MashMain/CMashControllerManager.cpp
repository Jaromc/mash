//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashControllerManager.h"
#include "CMashAnimationBuffer.h"
#include "CMashTransformController.h"
#include "CMashAnimationMixer.h"
#include "MashSceneNode.h"
#include "MashLog.h"

namespace mash
{
	CMashControllerManager::CMashControllerManager()
	{

	}

	CMashControllerManager::~CMashControllerManager()
	{
		std::set<MashAnimationMixer*>::iterator mixerIter = m_animationMixers.begin();
		std::set<MashAnimationMixer*>::iterator mixerEndIter = m_animationMixers.end();
		for(; mixerIter != mixerEndIter; ++mixerIter)
		{
			(*mixerIter)->Drop();
		}

		m_animationMixers.clear();
	}

	MashTransformationController* CMashControllerManager::CreateTransformController(MashTransformationKeySet *keys, MashSceneNode *node)
	{
		if (!keys)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to create transform controller. No keys pointer given.", 
				"CMashControllerManager::ChopAnimationBuffers");

			return 0;
		}

		if (!node)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to create transform controller. No scene node pointer given.", 
				"CMashControllerManager::ChopAnimationBuffers");

			return 0;
		}

		MashTransformationController *newController = MASH_NEW_COMMON CMashTransformationController(keys, node);
		return newController;
	}

	MashTransformationKeySet* CMashControllerManager::CreateTransformationKeySet()
	{
		MashTransformationKeySet *keySet = MASH_NEW_COMMON MashTransformationKeySet();

		return keySet;
	}

	MashAnimationMixer* CMashControllerManager::CreateMixer()
	{
		MashAnimationMixer *newMixer = MASH_NEW_COMMON CMashAnimationMixer(this);
		m_animationMixers.insert(newMixer);
		return newMixer;
	}

	void CMashControllerManager::AddAnimationsToMixer(MashAnimationMixer *mixer, MashSceneNode *node)
	{
		if (node->GetAnimationBuffer())
		{
			std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator iter = node->GetAnimationBuffer()->GetAnimationKeySets().begin();
			std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator end = node->GetAnimationBuffer()->GetAnimationKeySets().end();
			for(; iter != end; ++iter)
			{
				const uint32 controllerCount = iter->second.Size();
				for(uint32 i = 0; i < controllerCount; ++i)
				{
					/*
						No need to validate controller and key types here as this
						was done in the node class
					*/
					MashKeyController *newController = 0;

					switch(iter->second[i].controllerType)
					{
					case aCONTROLLER_TRANSFORMATION:
						{
							newController = CreateTransformController((MashTransformationKeySet*)iter->second[i].keySet, node);
							break;
						}
					default:
						{
							MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
									"Invalid controller type.",
									"CMashControllerManager:AddAnimationsToMixer");
						}
					}

					if (newController)
					{
						mixer->AddController(iter->first.GetCString(), newController);
						newController->Drop();//the mixer now owns it
					}
				}
			}
		}
	}

	eMASH_STATUS CMashControllerManager::ChopAnimationBuffers(MashSceneNode *node, const MashArray<sAnimationClip> &clips, bool processChildren)
	{
		if (node->GetAnimationBuffer())
		{
			MashAnimationBuffer *newBuffer = 0;
			if (node->GetAnimationBuffer()->ChopBuffer(this, clips, &newBuffer) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
								"Failed to chop animation buffer.",
								"CMashControllerManager:ChopAnimationBuffers");	

				return aMASH_FAILED;
			}

			node->RemoveAnimationBuffer();

			if (newBuffer)
				node->SetAnimationBuffer(newBuffer);
		}

		if (processChildren == true)
		{
			MashList<MashSceneNode*>::ConstIterator iter = node->GetChildren().Begin();
			MashList<MashSceneNode*>::ConstIterator end = node->GetChildren().End();
			for(; iter != end; ++iter)
			{
				if (ChopAnimationBuffers(*iter, clips, processChildren) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to chop animation buffers.", 
						"CMashControllerManager::ChopAnimationBuffers");

					return aMASH_FAILED;
				}
			}
		}
		
		return aMASH_OK;
	}

	void CMashControllerManager::ProcessNodeTreeAndAddToMixer(MashAnimationMixer *mixer, MashSceneNode *root)
	{
		AddAnimationsToMixer(mixer, root);

		MashList<MashSceneNode*>::ConstIterator iter = root->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator end = root->GetChildren().End();
		for(; iter != end; ++iter)
		{
			ProcessNodeTreeAndAddToMixer(mixer, *iter);
		}
	}

	MashAnimationMixer* CMashControllerManager::CreateMixer(MashSceneNode *root, bool processChildren)
	{
		MashAnimationMixer *newMixer = 0;
		if (!processChildren)
		{
			newMixer = CreateMixer();
			AddAnimationsToMixer(newMixer, root);
			root->SetAnimationMixer(newMixer);
		}
		else
		{
			newMixer = CreateMixer();
			ProcessNodeTreeAndAddToMixer(newMixer, root);
			root->SetAnimationMixer(newMixer);
		}

		//the root node now owns the mixer
		if (root)
			newMixer->Drop();

		return newMixer;
	}

	MashAnimationBuffer* CMashControllerManager::CreateAnimationBuffer()
	{
		return MASH_NEW_COMMON CMashAnimationBuffer();
	}

	void CMashControllerManager::Update(f32 dt)
	{
		std::set<MashAnimationMixer*>::iterator mixerIter = m_animationMixers.begin();
		std::set<MashAnimationMixer*>::iterator mixerEndIter = m_animationMixers.end();
		for(; mixerIter != mixerEndIter; ++mixerIter)
		{
			(*mixerIter)->_ForceAdvanceTime(dt);
			(*mixerIter)->_ForceAdvanceAnimation();
		}
	}

	void CMashControllerManager::_RemoveAnimationMixer(MashAnimationMixer *controller)
	{
		m_animationMixers.erase(controller);
	}
}