//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashAnimationMixer.h"
#include "MashControllerManager.h"
#include <set>
#include "MashAnimationController.h"
#include "MashKeySet.h"
#include "MashLog.h"

namespace mash
{
	CMashAnimationMixer::sStaticData *CMashAnimationMixer::m_staticData = 0;

	CMashAnimationMixer::CMashAnimationMixer(MashControllerManager *pControllerManager/*, int32 id*/):MashAnimationMixer(),
		m_pControllerManager(pControllerManager),
		m_dTime(0.0),
		m_iCurrentFrame(0),
		m_pTransitionTo(0),
		m_bTransitionAcvtive(false),
		m_bAffectAllTracks(false),
		m_fTotalTransitionTime(0.0f),
		m_fCurrentTransitionTime(0.0f)
	{
		SetFrameRate(30);

		if (!m_staticData)
			m_staticData = MASH_NEW_T_COMMON(sStaticData);
		else
			++m_staticData->refCounter;
	}
    
	CMashAnimationMixer::~CMashAnimationMixer()
	{
		MashList<sAnimationSet*>::Iterator animSetIter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator animSetIterEnd = m_animationSetsByLayer.End();
		for(; animSetIter != animSetIterEnd; ++animSetIter)
		{
			MASH_DELETE_T(sAnimationSet, *animSetIter);
		}

		m_animationSetsByLayer.Clear();
		m_animationSetsByName.clear();

		m_pControllerManager->_RemoveAnimationMixer(this);

		if (m_staticData)
		{
			--m_staticData->refCounter;
			if (m_staticData->refCounter == 0)
			{
				MASH_DELETE_T(sStaticData, m_staticData);
				m_staticData = 0;
			}
		}
	}

	void CMashAnimationMixer::_UpdateFrameNumber(sAnimationSet *pSet)
	{
		const int32 iTotalFrameCount = pSet->frameLength;

		/*
			Only advance the animation if its playing.
		*/
		if (pSet->bPlay && !pSet->bReverse)
			pSet->iFrame = (m_iCurrentFrame - pSet->iStartFrame) * pSet->fSpeed;
		else if (pSet->bPlay && pSet->bReverse)
			pSet->iFrame = (iTotalFrameCount - (m_iCurrentFrame - pSet->iStartFrame)) * pSet->fSpeed;

		if (pSet->iFrame > iTotalFrameCount)
		{
			if (pSet->eWrapMode == aWRAP_PLAYONCE)
			{
				pSet->iFrame = 0;
				pSet->bPlay = false;
				pSet->fWeight = 0.0f;

				/*
					TODO : When this case is reached we should run all the controllers
					to be at frame 0 otherwise CPU speed changes could cause animations
					to appear stopped half way though.
				*/
			}
			else if (pSet->eWrapMode == aWRAP_LOOP)
			{
				//set it to equal the difference
				pSet->iFrame  = (pSet->iFrame % iTotalFrameCount) - 1;
				pSet->iStartFrame = m_iCurrentFrame - pSet->iFrame;
			}
			else if (pSet->eWrapMode == aWRAP_BOUNCE)
			{
				int32 iDifference = pSet->iFrame % iTotalFrameCount;
				pSet->iFrame = iTotalFrameCount - iDifference;
				pSet->iStartFrame = m_iCurrentFrame - iDifference;
				pSet->bReverse = true;
			}
			else
			{
				pSet->iFrame = iTotalFrameCount;//clamped
			}
		}
		else if (pSet->iFrame < 0)
		{
			if (pSet->eWrapMode == aWRAP_PLAYONCE)
			{
				pSet->iFrame = 0;
				pSet->bPlay = false;
				pSet->fWeight = 0.0f;
			}
			else if (pSet->eWrapMode == aWRAP_LOOP)
			{
				//set it to equal the difference
				int32 f = ((pSet->iFrame * -1) - 1) % iTotalFrameCount;//In a perfect world this would be zero
				pSet->iFrame = iTotalFrameCount - f;
				pSet->iStartFrame = m_iCurrentFrame - f;
			}
			else if (pSet->eWrapMode == aWRAP_BOUNCE)
			{
				//set it to equal the difference
				pSet->iFrame = (pSet->iFrame * -1) % iTotalFrameCount;
				pSet->iStartFrame = m_iCurrentFrame - pSet->iFrame;
				pSet->bReverse = false;
			}
			else
				pSet->iFrame = 0;
		}
	}

	eMASH_STATUS CMashAnimationMixer::AddController(const int8 *animationName, MashKeyController *controller)
	{
		if (!controller || (controller->GetKeySet()->GetKeyCount() == 0))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Invalid controller pointer or the controller contained no keys.", 
						"CMashAnimationMixer::AddController");

			return aMASH_FAILED;
		}

		MashStringc animationNameBuffer = "";
		if (animationName)
			animationNameBuffer = animationName;

		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(animationNameBuffer);
		if (iter == m_animationSetsByName.end())
		{
			sAnimationSet *pNewSet = MASH_NEW_T_COMMON(sAnimationSet)();
			pNewSet->animationName = animationNameBuffer;
			iter = m_animationSetsByName.insert(std::make_pair(animationNameBuffer, pNewSet)).first;
			m_animationSetsByLayer.PushBack(pNewSet);
			m_animationSetsByLayer.Sort(sTrackSort());
		}

		iter->second->keyControllers.PushBack(controller);
		controller->Grab();

		const uint32 keyCount = controller->GetKeySet()->GetKeyCount();
		//set up the key cache for predictive lookups.
		//There is one element per animation.
		iter->second->keyCache.PushBack(sAnimationKeyCache());

		//update animation frame length
		const uint32 frameLength = controller->GetKeySet()->GetFrameFromKey(keyCount - 1);
		if (frameLength > iter->second->frameLength)
			iter->second->frameLength = frameLength;

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::SetReverse(const int8 *sName, bool bReversePlayback)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		if (iter->second->bReverse == bReversePlayback)
			return aMASH_OK;

		//bounces uses the reverse value internally to bounce the animation. So it cant be set by the user in this case.
		if (iter->second->eWrapMode == aWRAP_BOUNCE)
			return aMASH_OK;

		iter->second->bReverse = bReversePlayback;

		/*
			This adjusts the start frame time so that the animation continues
			smoothly in the opposite direction. For example, when making a character
			walk forwards and backwards.
		*/
		if (iter->second->bReverse)
			iter->second->iStartFrame = m_iCurrentFrame - (iter->second->frameLength - iter->second->iFrame);
		else
			iter->second->iStartFrame = m_iCurrentFrame - iter->second->iFrame;

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::Play(const int8 *sName, bool bStopAndResetAllTracks)
	{
		std::map<MashStringc, sAnimationSet*>::iterator playIter = m_animationSetsByName.find(sName);
		if (playIter == m_animationSetsByName.end())
			return aMASH_FAILED;

		/*
			Dont continue If this animation is already playing
		*/
		if (playIter->second->bPlay)
			return aMASH_OK;

		playIter->second->bPlay = true;
		playIter->second->iStartFrame = m_iCurrentFrame;

		MashList<sAnimationSet*>::Iterator iter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator end = m_animationSetsByLayer.End();
		for(; iter != end; ++iter)
		{
			if (*iter == playIter->second)
				continue;

			else if (bStopAndResetAllTracks || (m_pTransitionTo && ((*iter)->iTrack == m_pTransitionTo->iTrack)))
			{
				_Stop((*iter), true);
			}
		}
		return aMASH_OK;
	}

	void CMashAnimationMixer::_ResetAnimationBackToStart(sAnimationSet *set)
	{
		//advance the controllers (and transforms) back to the first key
		uint32 controllerCount = set->keyControllers.Size();
		for(uint32 i = 0; i < controllerCount; ++i)
		{
			MashKeyController *currentController = set->keyControllers[i];
			const MashKeySetInterface *currentKeySet = currentController->GetKeySet();
			const uint32 keyCount = currentKeySet->GetKeyCount();
			if (keyCount == 0)
				continue;

			currentController->AnimationStart();
			currentController->AnimateToKey(0);
			currentController->AnimationEnd(set->eBlendMode, 1.0f);
		}

		set->iFrame = 0;
	}

	eMASH_STATUS CMashAnimationMixer::ResetBackToStart(const int8 *sName)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		_ResetAnimationBackToStart(iter->second);

		return aMASH_OK;
	}

	void CMashAnimationMixer::_Stop(sAnimationSet *set, bool resetBackStart)
	{
		if (set->bPlay)
			set->bPlay = false;
		
		if (resetBackStart)
			_ResetAnimationBackToStart(set);
	}

	eMASH_STATUS CMashAnimationMixer::Stop(const int8 *sName, bool resetBackStart)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		_Stop(iter->second, resetBackStart);

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::StopAll(bool resetBackStart)
	{
		MashList<sAnimationSet*>::Iterator iter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator end = m_animationSetsByLayer.End();
		for(; iter != end; ++iter)
		{
			if ((*iter)->bPlay)
			{
				(*iter)->bPlay = false;
			}
			
			if (resetBackStart)
				_ResetAnimationBackToStart((*iter));
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::SetFrame(const int8 *sName, int32 iFrame)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		iter->second->iFrame = math::Clamp<int32>(0, iter->second->frameLength, iFrame);
		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::SetSpeed(const int8 *sName, f32 fSpeed)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		iter->second->fSpeed = fSpeed;

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::SetWeight(const int8 *sName, f32 fWeight)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		iter->second->fWeight = math::Clamp<f32>(0.0f, 1.0f, fWeight);
		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::SetWrapMode(const int8 *sName, eANIMATION_WRAP_MODE mode)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		iter->second->eWrapMode = mode;
		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::SetBlendMode(const int8 *sName, eANIMATION_BLEND_MODE mode)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		iter->second->eBlendMode = mode;
		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::SetTrack(const int8 *sName, uint32 iTrack)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(sName);
		if (iter == m_animationSetsByName.end())
			return aMASH_FAILED;

		iter->second->iTrack = iTrack;

		//re-sort the track list
		m_animationSetsByLayer.Sort(sTrackSort());

		return aMASH_OK;
	}

	bool CMashAnimationMixer::GetIsPlaying(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->bPlay;

		return false;
	}

	int32 CMashAnimationMixer::GetFrame(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->iFrame;

		return 0;
	}

	f32 CMashAnimationMixer::GetSpeed(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->fSpeed;

		return 0.0f;
	}

	int32 CMashAnimationMixer::GetFrameLength(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->frameLength;

		return 0;
	}

	f32 CMashAnimationMixer::GetWeight(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->fWeight;

		return 0.0f;
	}

	eANIMATION_WRAP_MODE CMashAnimationMixer::GetWrapMode(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->eWrapMode;

		return aWRAP_PLAYONCE;
	}

	eANIMATION_BLEND_MODE CMashAnimationMixer::GetBlendMode(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->eBlendMode;

		return aBLEND_BLEND;
	}

	uint32 CMashAnimationMixer::GetTrack(const int8 *sName)const
	{
		std::map<MashStringc, sAnimationSet*>::const_iterator iter = m_animationSetsByName.find(sName);
		if (iter != m_animationSetsByName.end())
			return iter->second->iTrack;

		return 0;
	}

	void CMashAnimationMixer::_ResetTime()
	{
		m_dTime = 0.0;
		m_iCurrentFrame = 0;

		/*
			We need to loop through any active animations
			and reset the start timers
		*/
		MashList<sAnimationSet*>::Iterator iter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator end = m_animationSetsByLayer.End();
		for(; iter != end; ++iter)
			(*iter)->iStartFrame = 0;
	}

	uint32 CMashAnimationMixer::GetAnimationCount()const
	{
		return m_animationSetsByLayer.Size();//m_animationSetCount;
	}

	void CMashAnimationMixer::GetAnimationNames(MashArray<MashStringc> &namesOut)const
	{
		MashList<sAnimationSet*>::ConstIterator setIter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::ConstIterator setIterEnd = m_animationSetsByLayer.End();
		for(; setIter != setIterEnd; ++setIter)
			namesOut.PushBack((*setIter)->animationName);
	}

	void CMashAnimationMixer::Transition(const int8 *sName, f32 fTransitionLength, bool bAffectAllTracks)
	{
		std::map<MashStringc, sAnimationSet*>::iterator mapNodeIter = m_animationSetsByName.find(sName);
		if (mapNodeIter == m_animationSetsByName.end())
			return;

		/*
			Dont repeat a recent similar call
		*/
		if (m_bTransitionAcvtive && (m_pTransitionTo == mapNodeIter->second))
			return;

		//dont perform a transition on an animation thats already playing
		if (mapNodeIter->second->bPlay == true && (mapNodeIter->second->fWeight == 1.0f))
			return;

		//dont allow transitioning into additive animations.
		if (mapNodeIter->second->eBlendMode == aBLEND_ADDITIVE)
			return;

		m_pTransitionTo = mapNodeIter->second;

		m_pTransitionTo->bPlay = true;

		/*
			Only reset this animations data if its weight is equal to zero. Anything else may mean
			it has recently been used in a transition, and reseting it would result is a hard reset
			back to the beginning
		*/
		if (m_pTransitionTo->fWeight < 0.0001f)
		{
			m_pTransitionTo->iFrame = 0;
			m_pTransitionTo->fWeight = 0.0f;
			m_pTransitionTo->iStartFrame = m_iCurrentFrame;
		}

		m_bTransitionAcvtive = true;
		m_bAffectAllTracks = bAffectAllTracks;
		m_fTotalTransitionTime = fTransitionLength;
		m_fCurrentTransitionTime = 0.0f;
	}

	void CMashAnimationMixer::SetFrameRate(int32 iFramesPerSecond)
	{
		m_iFramesPerSecond = math::Max<int32> (1, iFramesPerSecond);
	}

	void CMashAnimationMixer::RemoveAnimationSet(const int8 *sName)
	{
		std::map<MashStringc, sAnimationSet*>::iterator mapNodeIter = m_animationSetsByName.find(sName);
		if (mapNodeIter == m_animationSetsByName.end())
			return;

		m_animationSetsByName.erase(mapNodeIter);

		MashList<sAnimationSet*>::Iterator iter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator end = m_animationSetsByLayer.End();
		for(; iter != end; ++iter)
		{
			if (mapNodeIter->second == *iter)
			{
				m_animationSetsByLayer.Erase(iter);
				break;
			}
		}
	}

	void CMashAnimationMixer::SetCallbackTrigger(const int8 *animation, int32 frame, int32 userData)
	{
		std::map<MashStringc, sAnimationSet*>::iterator iter = m_animationSetsByName.find(animation);
		if (iter == m_animationSetsByName.end())
			return;

		sAnimationEvent newEvent;
		newEvent.mixer = this;
		newEvent.animationName = 0;//save setting this string until its time to send to avoid any memory issues.
		newEvent.frame = frame;
		newEvent.userData = userData;

		iter->second->callbackFrames.PushBack(newEvent);
	}

	void CMashAnimationMixer::SetCallbackHandler(MashAnimationEventFunctor callback)
	{
		m_callbackHandler = callback;
	}

	void CMashAnimationMixer::FlushTrackList(MashArray<sAnimationSet*> &layer, f32 &fRemainingWeight)
	{
		f32 fScale = 1.0f;
		f32 fTotalTrackWeight = 0.0f;

		const int32 iLayerSize = layer.Size();
		for(int32 i = 0; i < iLayerSize; ++i)
			fTotalTrackWeight += layer[i]->fWeight;

		/*
			If the total sum of all anim weights on this track is greater than one,
			then we need to scale each weight based on the total
		*/
		if (fTotalTrackWeight > 1.0f)
		{
			fScale = 1.0f / fTotalTrackWeight;

			//track weight is now 1 because everything will be scaled down
			//to be within the range 0 - 1
			fTotalTrackWeight = 1.0f;
		}

		if (fTotalTrackWeight > fRemainingWeight)
		{
			//scale down somemore if there is not enough weight remaining
			fScale *= fRemainingWeight;
		}

		BlendTracks(layer, fScale);

		fRemainingWeight -= fTotalTrackWeight;
	}

	eMASH_STATUS CMashAnimationMixer::BlendTracks(MashArray<sAnimationSet*> &sets,//MashArray<sTrackDesc*> &tracks, 
			f32 fTotalBlend)
	{
		const uint32 iNumSets = sets.Size();

		for(uint32 iCurrentSet = 0; iCurrentSet < iNumSets; ++iCurrentSet)
		{
			const uint32 controllerCount = sets[iCurrentSet]->keyControllers.Size();

			uint32 boundedFrameNum = sets[iCurrentSet]->iFrame;

			for(uint32 i = 0; i < controllerCount; ++i)
			{
				MashKeyController *currentController = sets[iCurrentSet]->keyControllers[i];
				const MashKeySetInterface *currentKeySet = currentController->GetKeySet();

				const uint32 keyCount = currentKeySet->GetKeyCount();
				if (keyCount == 0)
					continue;

				currentController->AnimationStart();

				/*
					Only update this animation if we are within its bounds. Remeber that during an animation cycle such
					a "walk", a bone may only move for a brief moment then hold its place for the rest of the walk cycle.
				*/
				//if (boundedFrameNum < currentKeySet->GetFrameFromKey(keyCount - 1))
				{
					uint32 iFrameStartKey = 0;
					uint32 iFrameEndKey = 0;
					sAnimationKeyCache *pKeyCache = &sets[iCurrentSet]->keyCache[i];
					//GetCachedKeys(sets[iCurrentSet], i, &pCache);

					currentKeySet->GetFrameBoundsCached(boundedFrameNum,
						sets[iCurrentSet]->bReverse,
						pKeyCache->iStart,
						pKeyCache->iEnd,
						iFrameStartKey,
						iFrameEndKey);

					pKeyCache->iStart = iFrameStartKey;
					pKeyCache->iEnd = iFrameEndKey;

					const f32 fMinTime = currentKeySet->GetFrameFromKey(iFrameStartKey);
					const f32 fMaxTime = currentKeySet->GetFrameFromKey(iFrameEndKey);

					f32 fDenom = (fMaxTime - fMinTime);
					f32 u = 0.0f;
					if (fDenom != 0.0f)
					{
						u = ((f32)boundedFrameNum - fMinTime) / fDenom;
						currentController->AnimateForward(iFrameStartKey, iFrameEndKey, u);

						/*
							This is just different code that checks if interpolation is needed.
							It doesn't seem to perform any better.
						*/
						/*if (u != 0.0f)
							currentController->AnimateForward(iFrameStartKey, iFrameEndKey, u);
						else if (u == 0.0f)
							currentController->AnimateToKey(iFrameStartKey);
						else
							currentController->AnimateToKey(iFrameEndKey);*/
					}
					else
						currentController->AnimateToKey(iFrameStartKey);
				}

				currentController->AnimationEnd(sets[iCurrentSet]->eBlendMode, sets[iCurrentSet]->fWeight * fTotalBlend);
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::_ForceAdvanceAnimation()
	{
		m_staticData->layer.Clear();
		m_staticData->addtiveAnimations.Clear();
		f32 fRemainingWeight = 1.0f;

		MashList<sAnimationSet*>::Iterator iter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator end = m_animationSetsByLayer.End();
		for(; iter != end; ++iter)
		{
			/*
				Only blend tracks that have weight AND there is overall weight remaining (Additive animations
				are not affected by overall weight)
			*/
			if (((*iter)->fWeight > 0.0f) &&
				!((*iter)->eBlendMode != aBLEND_ADDITIVE && (fRemainingWeight <= 0.0f)))
			{
				if ((*iter)->eBlendMode == aBLEND_ADDITIVE)
				{
					m_staticData->addtiveAnimations.PushBack(*iter);
					continue;
				}

				if (m_staticData->layer.Empty() || (m_staticData->layer.Back()->iTrack == (*iter)->iTrack))
				{
					m_staticData->layer.PushBack(*iter);
				}
				else
				{
					FlushTrackList(m_staticData->layer, fRemainingWeight);
					m_staticData->layer.Clear();

					//add new object
					m_staticData->layer.PushBack(*iter);
				}
			}
		}

		//flush the final list
		if (!m_staticData->layer.Empty())
		{
			FlushTrackList(m_staticData->layer, fRemainingWeight);
		}

		//additive animations are calculated last.
		/*
			We check the remaining weight because it should be less than 1 if some
			blended animation has played. Without blended animation additive animations
			don't work correctly.
		*/
		if ((fRemainingWeight < 1.0f) && !m_staticData->addtiveAnimations.Empty())
		{
			BlendTracks(m_staticData->addtiveAnimations, 1.0f);
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashAnimationMixer::_ForceAdvanceTime(f32 dt)
	{
		if (m_animationSetsByLayer.Empty())
			return aMASH_OK;

		m_dTime += dt;

		if (m_iCurrentFrame > mash::math::MaxInt32())
			_ResetTime();

		m_iCurrentFrame = m_dTime * m_iFramesPerSecond;

		MashList<sAnimationSet*>::Iterator iter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator end = m_animationSetsByLayer.End();
		for(; iter != end; ++iter)
		{
			//only check active animations
			if ((*iter)->fWeight > 0.0f)
			{
				(*iter)->iLastFrame = (*iter)->iFrame;

				//advance the animation time
				_UpdateFrameNumber(*iter);

				/*
					Handle callbacks.
					Grab all callbacks within a time range so we dont skip any.
					Only collect frames if the animation has been advanced since its last update
				*/
				const int32 iFrameDifference = (*iter)->iFrame - (*iter)->iLastFrame;
				if (m_callbackHandler.IsValid() && (iFrameDifference != 0))
				{
					//code is slightly different for reverse play
					if ((*iter)->bReverse)
					{
						const uint32 callbackCount = (*iter)->callbackFrames.Size();
						for(uint32 i = 0; i < callbackCount; ++i)
						{
							sAnimationEvent &callbackData = (*iter)->callbackFrames[i];

							if ((callbackData.frame >= (*iter)->iFrame) && (callbackData.frame <= (*iter)->iLastFrame))
							{
								callbackData.animationName = (*iter)->animationName.GetCString();
								m_callbackHandler.Call(callbackData);
							}
						}
					}
					else
					{
						const uint32 callbackCount = (*iter)->callbackFrames.Size();
						for(uint32 i = 0; i < callbackCount; ++i)
						{
							sAnimationEvent &callbackData = (*iter)->callbackFrames[i];

							if ((callbackData.frame >= (*iter)->iLastFrame) && (callbackData.frame <= (*iter)->iFrame))
							{
								callbackData.animationName = (*iter)->animationName.GetCString();
								m_callbackHandler.Call(callbackData);
							}
						}
					}
				}
			}
		}
        
		if (m_bTransitionAcvtive)
		{
			f32 u = 1.0f;

			//watch for divide by zero.
			if (m_fTotalTransitionTime > 0.0f)
				u = dt / m_fTotalTransitionTime;

			MashList<sAnimationSet*>::Iterator iter = m_animationSetsByLayer.Begin();
			MashList<sAnimationSet*>::Iterator end = m_animationSetsByLayer.End();
			for(; iter != end; ++iter)
			{
				if (*iter == m_pTransitionTo)
				{
					(*iter)->fWeight = math::Clamp<f32>(0.0f, 1.0f, (*iter)->fWeight + u);				}
				else if (m_bAffectAllTracks || ((*iter)->iTrack == m_pTransitionTo->iTrack))
				{
					(*iter)->fWeight = math::Clamp<f32>(0.0f, 1.0f, (*iter)->fWeight - u);

					if ((*iter)->fWeight < 0.00001f)
					{
						(*iter)->bPlay = false;
						(*iter)->iFrame = 0;
						(*iter)->fWeight = 0.0f;
					}
				}
			}

			if (m_fCurrentTransitionTime >= m_fTotalTransitionTime)
			{
				m_bTransitionAcvtive = false;
			}

			/*
				Doing this late means the transition calculations will occur 1
				more frame than it needs to, however, it also assures that 
				all animations being faded out are disabled.
			*/
			m_fCurrentTransitionTime += dt;
		}

		return aMASH_OK;
		
	}

	void CMashAnimationMixer::GetAffectedSceneNodes(MashArray<mash::MashSceneNode*> &sceneNodesOut)
	{
		std::set<mash::MashSceneNode*> sceneNodeSet;
		std::set<mash::MashSceneNode*>::iterator nodeSearchIter;
		MashList<sAnimationSet*>::Iterator setIter = m_animationSetsByLayer.Begin();
		MashList<sAnimationSet*>::Iterator setIterEnd = m_animationSetsByLayer.End();
		for(; setIter != setIterEnd; ++setIter)
		{
			MashArray<MashKeyController*>::Iterator controllerIter = (*setIter)->keyControllers.Begin();
			MashArray<MashKeyController*>::Iterator controllerIterEnd = (*setIter)->keyControllers.End();
			for(; controllerIter != controllerIterEnd; ++controllerIter)
			{
				mash::MashSceneNode *node = (*controllerIter)->GetOwner();
				nodeSearchIter = sceneNodeSet.find(node);
				if (nodeSearchIter == sceneNodeSet.end())
				{
					sceneNodesOut.PushBack(node);
					sceneNodeSet.insert(node);
				}
			}
		}
	}
}