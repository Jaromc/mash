//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _CMASH_ANIMATION_MIXER_H_
#define _CMASH_ANIMATION_MIXER_H_

#include "MashAnimationMixer.h"
#include <map>
#include "MashList.h"
#include "MashAnimationController.h"

namespace mash
{
	class MashControllerManager;

	class CMashAnimationMixer : public MashAnimationMixer
	{
	public:
		//! Controller update types.
		/*!
			All controllers will advance their time every
			update however it may be desirable to delay
			advancing the keys. For instance, skin controllers
			don't need their keys advanced unless it is being 
			rendered. So we delay it until we know it will
			be visible.
		*/
		enum eCONTROLLER_UPDATE
		{
			/*!
				Updates a controller before culling. This is necessary
				for node based animation, as the position needs to be
				updated before culling.
			*/
			IMMEDIATE,
			//! Delays advancing the keys until culling has been done.
			ON_DRAW
		};
	protected:

		//! Holds previous update data for fast lookups.
		struct sAnimationKeyCache
		{
			int32 iStart;
			int32 iEnd;

			sAnimationKeyCache():iStart(0), iEnd(0){}
		};

		//! Holds information about a single animation set.
		struct sAnimationSet
		{
			MashStringc animationName;

			MashArray<MashKeyController*> keyControllers;
			MashArray<sAnimationEvent> callbackFrames;

			//! Key cache for each animation in the set.
			MashArray<sAnimationKeyCache> keyCache;
			//! Wrap mode. 
			eANIMATION_WRAP_MODE eWrapMode;
			//! Blend mode.
			eANIMATION_BLEND_MODE eBlendMode;
			//! Blend weight.
			f32 fWeight;
			//! Animation speed.
			f32 fSpeed;
			//! Current frame.
			int32 iFrame;
			//! Store the last frame.
			/*!
				This is handy for making sure we dont miss frames
				inbetween updates (due to framerates). The callback 
				system for example needs to know the frames we have 
				passed so we don't miss any callbacks along the way.
			*/
			int32 iLastFrame;
			/*!
				This is true if the animation is playing (the controller
				has full control over the current frame).
			*/
			bool bPlay;
			/*!
				Only valid if bPlay is true. Makes the frames go
				in reverse.
			*/
			bool bReverse;
			//! Used when playing so we can track the time passed.
			int32 iStartFrame;
			uint32 iTrack;

			int32 frameLength;

			sAnimationSet():animationName(""),
				fWeight(0.0f),
				fSpeed(1.0f),
				iFrame(0),
				iLastFrame(0),
				iStartFrame(0),
				//bEnabled(false),
				bPlay(false),
				iTrack(0),
				frameLength(0),
				bReverse(false),
				eWrapMode(aWRAP_PLAYONCE),
				eBlendMode(aBLEND_BLEND){}

			~sAnimationSet()
			{
				MashArray<MashKeyController*>::Iterator iter = keyControllers.Begin();
				MashArray<MashKeyController*>::Iterator iterEnd = keyControllers.End();
				for(; iter != iterEnd; ++iter)
				{
					(*iter)->Drop();
				}

				keyControllers.Clear();
			}
		};

		//! Track sorter 
		struct sTrackSort
		{
			bool operator()(const sAnimationSet *a, const sAnimationSet *b)const
			{
				return a->iTrack > b->iTrack;
			}
		};

		struct sStaticData
		{
			/*
				These are stored as static because the same memory can be reused
				across all instances.
			*/
			MashArray<sAnimationSet*> layer;
			MashArray<sAnimationSet*> addtiveAnimations;
			uint32 refCounter;

			sStaticData():refCounter(1){}
		};
	private:
		//! Updated every frame to track the time passed.
		int32 m_iCurrentFrame;
	protected:
		static sStaticData *m_staticData;
		//! Values used for aniamtion blending.
		sAnimationSet *m_pTransitionTo;
		bool m_bTransitionAcvtive;
		bool m_bAffectAllTracks;
		//! Transition time set by the user. 
		f32 m_fTotalTransitionTime;
		//! Time that has passed since the transition started.
		f32 m_fCurrentTransitionTime;
		///////////////////////////////////

		//! Current time since controller was added to a scene.
		f64 m_dTime;
		//! Frames per second the animations should be played back at.
		int32 m_iFramesPerSecond;

		//! Holds the animation sets. Sorted differently for speed.
		MashList<sAnimationSet*> m_animationSetsByLayer;
		std::map<MashStringc, sAnimationSet*> m_animationSetsByName;

		//! The controller manager.
		MashControllerManager *m_pControllerManager;
		//! Callback handler for this controller.
		//MashAnimationCallbackHandler *m_pCallbackHandler;
		MashAnimationEventFunctor m_callbackHandler;

		//! Internal use only. Updates the frame number for the parameter.
		void _UpdateFrameNumber(sAnimationSet *pSet);
		//! Internal use only. 
		/*!
			Resets the controllers timers when the time has
			reached their limits.
		*/
		virtual void _ResetTime();

		eMASH_STATUS BlendTracks(MashArray<sAnimationSet*> &sets, f32 iTotalBlend);

		void FlushTrackList(MashArray<sAnimationSet*> &layer, f32 &fRemainingWeight);
		void _ResetAnimationBackToStart(sAnimationSet *set);
		void _Stop(sAnimationSet *set, bool resetBackStart);

	public:
		//! Constructor.
		CMashAnimationMixer(MashControllerManager *pControllerManager/*, int32 id*/);
		
		//! Destructor.
		virtual ~CMashAnimationMixer();

		eMASH_STATUS AddController(const int8 *animationName, MashKeyController *controller);

		//! Adds the handler for this controller.
		/*!
			The controller grabs an instance of the handler so you may like
			to Drop the handler after this has been called to avoid memory
			leaks.

			If this class already has a handler set then the old one will 
			be dropped.

			\param pHandler controller to add.
		*/
		virtual void SetCallbackHandler(MashAnimationEventFunctor callback);
		virtual void SetCallbackTrigger(const int8 *animation, int32 frame, int32 userData);

		/*
			Removes an instance of an animation set from this controller.
			The set will be deleted if there are no more references to it.
			Returns true if the set was deleted from memory. 
		*/
		//! Remove an animation set by name and Drops the instance.
		/*!
			\param sName Animation set to remove.
		*/
		virtual void RemoveAnimationSet(const int8 *sName);

		//! Transition from the current animation set to a new one.
		/*!
			Use this for smooth transitioning to a new animation set. It will fade out currently active
			animation sets and fade in the new animation set.
			This method will adjust the weight, current frame, and play status of the transitioning
			animations.

			\param sName Animation set to transition to.
			\param fTransitionLength The time over which the transition will occur.
			\param bAffectAllTracks Should all tracks fade out, or only animation sets on
					the same track as the one being faded in.
		*/
		virtual void Transition(const int8 *sName, f32 fTransitionLength = 0.4f, bool bAffectAllTracks = false); 
		
		//! Play a particular animation set.
		/*!
			Automatically advances an animation sets frame.
			This method does not affect weighting.

			\param sName Animation set to play.
			\param bStopAllTracks Stops and resets all animations
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS Play(const int8 *sName, bool bStopAndResetAllTracks = false);

		//! Stop an animation set from playing.
		/*!
			\param sName Animation set to stop playing.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS Stop(const int8 *sName, bool resetBackStart = false);

		//! Stop all animation sets from playing.
		/*!
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS StopAll(bool resetBackStart = false);

		//! Only useful if Play is true. Set reverse play.
		/*!
			This function has no effect if wrapping mode is equal to aWRAP_BOUNCE.
			\param sName Animation set to stop playing.
			\param bReversePlayback Set reverse to true or false.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetReverse(const int8 *sName, bool bReversePlayback);

		//! Only useful if Play is true. Set playback speed.
		/*!
			\param sName Animation set to stop playing.
			\param bReversePlayback Set reverse to true or false.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetSpeed(const int8 *sName, f32 fSpeed);

		//! Only useful if Play is false. Sets the current frame.
		/*!
			This can be used for procedural animation, for example, playing
			a lean animation based on user input.

			\param sName Animation set to affect.
			\param iFrame Current frame.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetFrame(const int8 *sName, int32 iFrame);
		/*
			Setting the weight affects blending when two or more animations
			are playing. Weight will have no affect with only 1 animation
			playing.
			Weight will automatically be set when using transitioning functions
		*/
		//! Manually sets the blend weight.
		/*!
			Only valid if an animations blend mode is set to aBLEND_BLEND.
			This value will be overwritten if the Transition() function is
			used.

			\param sName Animation set to affect.
			\param fWeight Blend weight.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetWeight(const int8 *sName, f32 fWeight);

		//! Sets the wrap mode.
		/*!
			\param sName Animation set to affect.
			\param mode Wrap mode.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetWrapMode(const int8 *sName, eANIMATION_WRAP_MODE mode);

		//! Sets the blend mode.
		/*!
			\param sName Animation set to affect.
			\param mode Blend mode.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetBlendMode(const int8 *sName, eANIMATION_BLEND_MODE mode);

		//! Set an animations sets track.
		/*!
			Giving an animation set a higher track number will give
			it priority when blending animations. 
			It can also be used to determine which animations are affected
			by certain methods, for example, transitions.
			There is no limit on the track numbers.

			\param sName Animation set to affect.
			\param iTrack Track ID.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetTrack(const int8 *sName, uint32 iTrack);

		//! Returns true if the animation is playing.
		/*!
			\param Animation set to test.
			\return True if the animation set is playing. False otherwise.
		*/
		virtual bool GetIsPlaying(const int8 *sName)const;

		//! Returns the current frame.
		/*!
			\param Animation set to test.
			\return Current frame.
		*/
		virtual int32 GetFrame(const int8 *sName)const;

		//! Returns the speed.
		/*!
			\param Animation set to test.
			\return Animation speed.
		*/
		virtual f32 GetSpeed(const int8 *sName)const;

		//! Returns the weight.
		/*!
			\param Animation set to test.
			\return Animation weight.
		*/
		virtual f32 GetWeight(const int8 *sName)const;

		//! Returns the wrap mode.
		/*!
			\param Animation set to test.
			\return Wrap mode.
		*/
		virtual eANIMATION_WRAP_MODE GetWrapMode(const int8 *sName)const;

		//! Returns the wrap mode.
		/*!
			\param Animation set to test.
			\return Wrap mode.
		*/
		virtual eANIMATION_BLEND_MODE GetBlendMode(const int8 *sName)const;

		//! Returns the track ID.
		/*!
			\param Animation set to test.
			\return Track ID.
		*/
		virtual uint32 GetTrack(const int8 *sName)const;

		//! Set the frame rate for the animation sets.
		/*!
			Common FPS settings are 24, 25, and 30. You should set this
			number to be the same as in the modeling package used to
			created the animation.

			\param iFramesPerSecond FPS.
		*/
		virtual void SetFrameRate(int32 iFramesPerSecond);

		//! Gets the frame rate for the animation sets.
		/*!
			\return Frames per second.
		*/
		virtual int32 GetFrameRate()const;

		//! Returns an animation set.
		/*!
			\param sAnimationSetName Animation set to retrieve.
			\return Animation set.
		*/
		//virtual MashAnimationSet* GetAnimationSetByName(const int8 *sAnimationSetName);

		//! Returns the number of animation sets in this controller.
		/*!
			\return Animation set count.
		*/
		virtual uint32 GetAnimationCount()const;

		//! Returns the names of each animation set.
		/*!
			\namesOut Animation set names.
		*/
		virtual void GetAnimationNames(MashArray<MashStringc> &namesOut)const;

		//! Returns the update type of this controller.
		/*!
			\return Update type.
		*/
		virtual eCONTROLLER_UPDATE GetUpdateType()const;

		//! Internal use only. Advances the timers.
		/*!
			This will be called by AdvanceTimeIfNeeded().

			This is called BEFORE ForceAdvanceAnimation().
			\param dt Time elapsed since the last update.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS _ForceAdvanceTime(f32 dt);
		//! Internal use only. Advances the frames.
		/*!
			This will be called by AdvanceAnimationIfNeeded().

			This is called AFTER ForceAdvanceAnimation().
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS _ForceAdvanceAnimation();

		//! Utility function. Gathers all the scene nodes affected by this mixer.
		/*!
			\param sceneNodesOut This array will be filled with the affected nodes.
		*/
		virtual void GetAffectedSceneNodes(MashArray<mash::MashSceneNode*> &sceneNodesOut);

		virtual int32 GetFrameLength(const int8 *sName)const;

		virtual eMASH_STATUS ResetBackToStart(const int8 *sName);
	};

	inline int32 CMashAnimationMixer::GetFrameRate()const
	{
		return m_iFramesPerSecond;
	}

	inline CMashAnimationMixer::eCONTROLLER_UPDATE CMashAnimationMixer::GetUpdateType()const
	{
		return IMMEDIATE;
	}
}

#endif