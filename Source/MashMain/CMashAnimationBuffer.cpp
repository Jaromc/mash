//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "CMashAnimationBuffer.h"
#include "MashKeySet.h"
#include "MashHelper.h"
#include "MashControllerManager.h"
#include "MashLog.h"

namespace mash
{
	CMashAnimationBuffer::~CMashAnimationBuffer()
	{
		std::map<MashStringc, MashArray<sController> >::const_iterator animationIter = m_animations.begin();
		std::map<MashStringc, MashArray<sController> >::const_iterator animationIterEnd = m_animations.end();
		for(; animationIter != animationIterEnd; ++animationIter)
		{
			MashArray<sController>::ConstIterator controllerIter = animationIter->second.Begin();
			MashArray<sController>::ConstIterator controllerIterEnd = animationIter->second.End();
			for(; controllerIter != controllerIterEnd; ++controllerIter)
			{
				controllerIter->keySet->Drop();
			}
		}

		m_animations.clear();
	}

	eMASH_STATUS CMashAnimationBuffer::AddAnimationKeySet(const MashStringc &animationName, eMASH_CONTROLLER_TYPE destinationType, MashKeySetInterface *keySet)
	{
		if (!keySet)
			return aMASH_FAILED;

		if (mash::helpers::ValidateAnimationKeyTypeToController(destinationType, keySet->GetKeyType()) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
								  "Controller type is not compatable with key set.", 
								  "CMashAnimationBuffer::AddAnimationKeySet");

			return aMASH_FAILED;
		}

		m_animations[animationName].PushBack(sController(keySet, destinationType));
		keySet->Grab();

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationBuffer::ChopBuffer(MashControllerManager *manager, const MashArray<sAnimationClip> &clips, MashAnimationBuffer **out)const
	{
		*out = manager->CreateAnimationBuffer();

		std::map<MashStringc, MashArray<sController> >::const_iterator animationIter = m_animations.begin();
		std::map<MashStringc, MashArray<sController> >::const_iterator animationIterEnd = m_animations.end();
		for(; animationIter != animationIterEnd; ++animationIter)
		{
			MashArray<sController>::ConstIterator controllerIter = animationIter->second.Begin();
			MashArray<sController>::ConstIterator controllerIterEnd = animationIter->second.End();
			for(; controllerIter != controllerIterEnd; ++controllerIter)
			{
				MashArray<sAnimationClip>::ConstIterator clipIter = clips.Begin();
				MashArray<sAnimationClip>::ConstIterator clipIterEnd = clips.End();
				for(; clipIter != clipIterEnd; ++clipIter)
				{
					MashKeySetInterface *newInterface = 0;
					if (controllerIter->keySet->Chop(*clipIter, &newInterface) == aMASH_FAILED)
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
									  "Failed to chop animation buffer.", 
									  "CMashAnimationBuffer::ChopBuffer");

						(*out)->Drop();
						*out = 0;

						return aMASH_FAILED;
					}
					
					if (newInterface)
						(*out)->AddAnimationKeySet(clipIter->name.GetCString(), controllerIter->controllerType, newInterface);
				}

			}
		}

		return aMASH_OK;
	}

	void CMashAnimationBuffer::SetAnimationName(const MashStringc &from, const MashStringc &to)
	{
		std::map<MashStringc, MashArray<sController> >::iterator iter = m_animations.find(from);
		if (iter != m_animations.end())
		{
			m_animations.insert(std::make_pair(to, iter->second));
			m_animations.erase(iter);
		}
	}

	void CMashAnimationBuffer::GetAnimationFrameBounds(const MashStringc &animationName, uint32 &startFrame, uint32 &endFrame)const
	{
		startFrame = mash::math::MaxUInt32();
		endFrame = 0;

		std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator iter = m_animations.find(animationName);
		if (iter != m_animations.end())
		{
			uint32 controllerCount = iter->second.Size();
			for(uint32 con = 0; con < controllerCount; ++con)
			{
				MashKeySetInterface *keyInterface = iter->second[con].keySet;
				if (keyInterface->GetKeyCount() > 0)
				{
					uint32 tempStart = keyInterface->GetFrameFromKey(0);
					uint32 tempEnd = keyInterface->GetFrameFromKey(keyInterface->GetKeyCount() - 1);
					if (tempStart < startFrame)
						startFrame = tempStart;
					if (tempEnd > endFrame)
						endFrame = tempEnd;
				}
			}
		}
	}
}