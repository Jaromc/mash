//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_KEY_SET_H_
#define _MASH_KEY_SET_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashTypes.h"
#include "MashAnimationKey.h"
#include "MashArray.h"
#include "MashLog.h"

namespace mash
{
	class MashControllerManager;
    
    /*!
        Base class for all animation key types.
    */
	class MashKeySetInterface : public MashReferenceCounter
	{
	public:
		MashKeySetInterface():MashReferenceCounter(){}
		virtual ~MashKeySetInterface(){}

        //! Gets a keys frame.
        /*!
            \param key Key.
            \return Keys frame.
        */
		virtual uint32 GetFrameFromKey(uint32 key)const = 0;
        
        //! Gets the keys count.
        /*!
            \return Keys sets key count.
        */
		virtual uint32 GetKeyCount()const = 0;

        //! Gets the keys that surround a frame.
        /*!
            Example, This set may contain 4 keys with frame 0, 10, 15, 20.
            If the frame passed in was 5 then this function would return
            keys 0 and 1.
         
            \param frame Frame to query.
            \param fromKeyOut Key < frame key.
            \param toKeyOut key > frame key.
        */
		virtual void GetFrameBounds(uint32 frame, uint32 &fromKeyOut, uint32 &toKeyOut)const = 0;
        
        //! Same as GetFrameBounds but you can pass in a hint.
        /*!
            \param frame Frame to query.
            \param playingInReverse Hint as to which way to search.
            \param lastFromKey Hint as to where it was last frame.
            \param lastToKey Hint as to where it was last frame.
            \param fromKeyOut Key < frame key. This can be used as the hint next frame.
            \param toKeyOut key > frame key. This can be used as the hint next frame.
        */
		virtual void GetFrameBoundsCached(uint32 frame, bool playingInReverse, 
			uint32 lastFromKey, uint32 lastToKey,
			uint32 &fromKeyOut, uint32 &toKeyOut)const = 0;

        //! Internal method for chopping the key set into clips.
        /*!
            \param clip Clips to chop this set into.
            \param out Chopped arrays.
            \return Ok on success. Failed otherwise.
        */
		virtual eMASH_STATUS Chop(const sAnimationClip &clip, MashKeySetInterface **out)const = 0;

        //! Gets the key type.
        /*!
            \return Key type.
        */
		virtual eANIM_KEY_TYPE GetKeyType()const = 0;

		//! Gets a new empty set of the derived type.
		virtual MashKeySetInterface* CreateNewSet()const = 0;
	};

    /*!
        Intermediate class for all key sets. This implements
        the common functions for all key sets.
    */
	template<class T>
	class MashKeySet : public MashKeySetInterface
	{
	protected:
		MashArray<T> m_keyList;

		struct sCompareKeyFramesPred
		{
			bool operator()(const T &lhs, const T &rhs)const
			{
				return lhs.frame < rhs.frame;
			}
		};

		/*
			This converts the search function to look for greater than
			or equal to.
		*/
		struct sCompareKeyFramesGreaterPred
		{
			bool operator()(const T &lhs, const T &rhs)const
			{
				return lhs.frame > rhs.frame;
			}
		};
	public:
		MashKeySet():MashKeySetInterface(){}
		virtual ~MashKeySet(){}

		const T* GetKey(uint32 key)const
		{
			return &m_keyList[key];
		}

		const MashArray<T>& GetKeyArray()const
		{
			return m_keyList;
		}

		uint32 GetFrameFromKey(uint32 key)const
		{
			return m_keyList[key].frame;
		}

		uint32 GetKeyCount()const
		{
			return m_keyList.Size();
		}

		void GetFrameBounds(uint32 frame, uint32 &fromKeyOut, uint32 &toKeyOut)const
		{
			fromKeyOut = 0;
			toKeyOut = 0;

			T keyToFind;
			keyToFind.frame = frame;

			typename MashArray<T>::ConstIterator foundFrameIter = mash::algorithms::LowerBoundPred<typename MashArray<T>::ConstIterator, T, sCompareKeyFramesPred, algorithms::sPassThrough<bool> >(m_keyList.Begin(), m_keyList.End(), keyToFind, sCompareKeyFramesPred(), algorithms::sPassThrough<bool>());
			if (foundFrameIter != m_keyList.End())
			{
				fromKeyOut = m_keyList.Begin().Distance(foundFrameIter);
				++foundFrameIter;
				if (foundFrameIter != m_keyList.End())
					toKeyOut = fromKeyOut + 1;
				else
					toKeyOut = fromKeyOut;
			}
		}

		eMASH_STATUS Chop(const sAnimationClip &clip, MashKeySetInterface **out)const
		{
			*out = 0;
			int32 startKey = -1;
			int32 endKey = -1;
			uint32 frame = 0;

			/*
				Get bounding keys around the clip
			*/
			T keyToFind;
			keyToFind.frame = clip.start;

			typename MashArray<T>::ConstIterator startFrameIter = mash::algorithms::LowerBoundPred<typename MashArray<T>::ConstIterator, T, sCompareKeyFramesPred, algorithms::sPassThrough<bool> >(m_keyList.Begin(), m_keyList.End(), keyToFind, sCompareKeyFramesPred(), algorithms::sPassThrough<bool>());
			if (startFrameIter != m_keyList.End())
				startKey = m_keyList.Begin().Distance(startFrameIter);

			keyToFind.frame = clip.end;
			typename MashArray<T>::ConstIterator endFrameIter = mash::algorithms::LowerBoundPred<typename MashArray<T>::ConstIterator, T, sCompareKeyFramesGreaterPred, algorithms::sPassThrough<bool> >(m_keyList.Begin(), m_keyList.End(), keyToFind, sCompareKeyFramesGreaterPred(), algorithms::sPassThrough<bool>());
			if (endFrameIter != m_keyList.End())
				endKey = m_keyList.Begin().Distance(endFrameIter);

			if (startKey != -1 && endKey != -1)
			{
				if (endKey > startKey)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Animation clip start frame must be less than the end frame.", 
						"MashTransformationKeySet::Chop");

					return aMASH_FAILED;
				}

				MashKeySet<T> *newKeySet = (MashKeySet<T>*)CreateNewSet();
				if (!newKeySet)
					return aMASH_FAILED;

				int32 frameAdjustmentTime = -m_keyList[startKey].frame;
				if (newKeySet->AddKeyList(&m_keyList[startKey], endKey - startKey) == aMASH_FAILED)
				{
					newKeySet->Drop();
					return aMASH_FAILED;
				}

				/*
					The new animations time is adjusted so that it starts at 0
				*/
				newKeySet->AdjustAllKeyFrameTimes(frameAdjustmentTime);

				*out = newKeySet;
			}

			return aMASH_OK;
		}

		/*
			This function uses the result of the last lookup to predict the key locations.
			If the cached data doesn't work then a full search is performed (This will usually only
			happen in the first call).
		*/
		void GetFrameBoundsCached(uint32 frame, bool playingInReverse, uint32 lastFromKey, uint32 lastToKey,
				uint32 &fromKeyOut, uint32 &toKeyOut)const
		{
			fromKeyOut = 0;
			toKeyOut = 0;

			const uint32 keyCount = m_keyList.Size();

			if (keyCount < 2)
				return;

			if (!playingInReverse)
			{
				//first check cached result
				if ((m_keyList[lastFromKey].frame <= frame) && (m_keyList[lastToKey].frame >= frame))
				{
					fromKeyOut = lastFromKey;
					toKeyOut = lastToKey;
					return;
				}

				//Before checking the next set, make sure we haven't hit the end of the keys.
				int32 nextPredKey = lastToKey + 1;
				if (nextPredKey >= keyCount)
				{
					fromKeyOut = 0;
					toKeyOut = 1;
					return;
				}

				//try the next set along
				if ((m_keyList[lastToKey].frame <= frame) && (m_keyList[nextPredKey].frame >= frame))
				{
					fromKeyOut = lastToKey;
					toKeyOut = nextPredKey;
					return;
				}
			}
			else
			{
				//first check cached result
				if ((m_keyList[lastFromKey].frame >= frame) && (m_keyList[lastToKey].frame <= frame))
				{
					fromKeyOut = lastFromKey;
					toKeyOut = lastToKey;
					return;
				}

				//Before checking the next set, make sure we haven't hit the end of the keys.
				int32 nextPredKey = lastToKey - 1;
				if (nextPredKey < 0)
				{
					fromKeyOut = keyCount-1;
					toKeyOut = math::Max<int32>(0, fromKeyOut-1);
					return;
				}

				//try the next set along
				if ((m_keyList[lastToKey].frame >= frame) && (m_keyList[nextPredKey].frame <= frame))
				{
					fromKeyOut = lastToKey;
					toKeyOut = nextPredKey;
					return;
				}
			}

			//cant find it so search the whole array
			GetFrameBounds(frame, fromKeyOut, toKeyOut);
		}

		void AdjustAllKeyFrameTimes(int32 frameTimeAdjustment)
		{
			const uint32 keyCount = m_keyList.Size();
			for(uint32 i = 0; i < keyCount; ++i)
			{
				m_keyList[i].frame += frameTimeAdjustment;
			}
		}
		
		/*!
			All keys MUST be in ASSENDING order.
		*/
		eMASH_STATUS AddKeyList(const T *keys, uint32 keyCount)
		{
			if (keys && keyCount)
			{
				m_keyList.Assign(keys, keyCount);
				m_keyList.ShrinkToFit();
			}

			return aMASH_OK;
		}

		/*!
			Adding keys in assending order will assure fastest insetion.
			This function makes sure all inserted keys end up in assending order.
		*/
		eMASH_STATUS AddKey(const T &key)
		{
			/*
				If the last key is less than the new key then we simply push back
				the new key. Otherwise we need to find the location to insert the new key.
			*/
			if (m_keyList.Empty() || (key.frame > m_keyList.Back().frame))
			{
				m_keyList.PushBack(key);
				return aMASH_OK;
			}

			typename MashArray<T>::Iterator upperBoundIter = mash::algorithms::LowerBoundPred<typename MashArray<T>::Iterator, T, sCompareKeyFramesGreaterPred, algorithms::sPassThrough<bool> >(m_keyList.Begin(), m_keyList.End(), key, sCompareKeyFramesGreaterPred(), algorithms::sPassThrough<bool>());
			if (upperBoundIter != m_keyList.End())
			{
				//make sure frames numbers are different
				if (upperBoundIter->frame == key.frame)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Animation keys in the same set cannot share frame numbers.", 
						"MashTransformationKeySet::AddKey");

					return aMASH_FAILED;
				}
			}

			m_keyList.Insert(upperBoundIter, key);

			return aMASH_OK;
		}
	};

    /*!
        Keys used for transformation.
    */
	class MashTransformationKeySet : public MashKeySet<sMashAnimationKeyTransform>
	{
	public:
		MashTransformationKeySet():MashKeySet<sMashAnimationKeyTransform>(){}
		virtual ~MashTransformationKeySet(){}

		MashKeySetInterface* CreateNewSet()const
		{
			return MASH_NEW_COMMON MashTransformationKeySet();
		}

		eANIM_KEY_TYPE GetKeyType()const {return aANIM_KEY_TRANSFORM;}
	};
}

#endif