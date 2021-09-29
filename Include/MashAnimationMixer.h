//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ANIMATION_MIXER_H_
#define _MASH_ANIMATION_MIXER_H_

#include "MashDataTypes.h"
#include "MashEnum.h"
#include "MashReferenceCounter.h"
#include "MashEventTypes.h"
#include "MashArray.h"
#include "MashString.h"

namespace mash
{
    class MashSceneNode;
	class MashKeyController;

	/*!
		An animation mixer is the main interface a user will manipulate animations for a node
		or a set of nodes. Mixers can be created from MashControllerManager::CreateMixer().

		Mixers hold and advance animation data held in key controllers. They are most commonly used 
		for node movement.

		Mixers may affect 1 node or many nodes. For example, an entity may be skinned by using many bones that
		each contain animation buffers. Each of these buffers describe that bones movement during different 
		animation cycles such as walk, run, jump, etc... All those buffers can be add to a single mixer
		that will animate all those nodes foward at the same time, this will then make an entity look like its
		walking or running.
     
        To blend from one animation to another use Transition(), this is the easiest way to use
        a mixer. 
     
        For more advance usage animations must have a weight set using SetWeight(). Weighting
        blends animations to form the final animation state. If no weighting is set then no change will
        be visible. The current frame can then be set using SetFrame() or automatically advanced using
        Play().
	*/
	class MashAnimationMixer : public MashReferenceCounter
	{
	public:
		//! Constructor.
		MashAnimationMixer():MashReferenceCounter(){};
		
		//! Destructor.
		virtual ~MashAnimationMixer(){}

		//! Adds a controller to the mixer.
		/*!
			\param animationName Animation name such as walk, run, jump, etc..
			\param controller Controller.
			\return Ok if everything was fine, failed otherwise.
		*/
		virtual eMASH_STATUS AddController(const int8 *animationName, MashKeyController *controller) = 0;

		//! Adds the handler for this controller.
		/*!
			The controller grabs an instance of the handler so you may like
			to Drop the handler after this has been called to avoid memory
			leaks.

			If this class already has a handler set then the old one will 
			be dropped.

			\param pHandler controller to add.
		*/
		virtual void SetCallbackHandler(MashAnimationEventFunctor callback) = 0;

		//! A trigger will send a message to the callback handler when a particular frame is reached.
		/*!
			\param animation Animation name.
			\param frame Frame to fire the event.
			\param userData This value will be passed to the callback handler so this event can be identified.
		*/
		virtual void SetCallbackTrigger(const int8 *animation, int32 frame, int32 userData) = 0;

		/*!
			\param sName Animation set to remove.
		*/
		virtual void RemoveAnimationSet(const int8 *name) = 0;

		//! Transition from the current animation set to a new one.
		/*!
			Use this for smooth transitioning to a new animation set. It will fade out currently active
			animation sets and fade in the new animation set.
			This method will adjust the weight, current frame, and play status of the transitioning
			animations.
			This is the recommended way of playing animations. For more advanced functionality the
			Play() function can be used.

			\param name Animation set to transition to.
			\param transitionLength The time over which the transition will occur.
			\param affectAllTracks Should all tracks fade out, or only animation sets on
					the same track as the one being faded in.
		*/
		virtual void Transition(const int8 *name, f32 transitionLength = 0.4f, bool affectAllTracks = false) = 0; 
		
		//! Play a particular animation set.
		/*!
			If you will only be playing one blended animation at a time then use Transition() for simplicity.

			Play automatically advances an animation sets frame. In order to see the animation playing
			you will need to set the weight of the animation to a value > 0.

			\param name Animation set to play.
			\param stopAllTracks Stops and resets all animations
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS Play(const int8 *name, bool stopAndResetAllTracks = false) = 0;

		//! Stop an animation set from playing.
		/*!
			\param name Animation set to stop playing.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS Stop(const int8 *name, bool resetBackStart = false) = 0;

		//! Stop all animation sets from playing.
		/*!
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS StopAll(bool resetBackStart = false) = 0;

		//! Only useful if Play is true. Set reverse play.
		/*!
			This function has no effect if wrapping mode is equal to aWRAP_BOUNCE.

			\param name Animation set to stop playing.
			\param reversePlayback Set reverse to true or false.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetReverse(const int8 *name, bool reversePlayback) = 0;

		//! Only useful if Play is true. Set playback speed.
		/*!
			\param name Animation set name.
			\param speed Speed of the animation. 1 == normal speed.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetSpeed(const int8 *name, f32 speed) = 0;

		//! Only useful if Play is false. Sets the current frame.
		/*!
			This can be used for procedural animation, for example, playing
			a lean animation based on user input.

			\param name Animation set to affect.
			\param frame Current frame.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetFrame(const int8 *name, int32 frame) = 0;

		//! Manually sets the blend weight.
		/*!
			Only valid if an animations blend mode is set to aBLEND_BLEND.
			This value will be overwritten if the Transition() function is
			used.

			Setting the weight affects blending when two or more animations
			are playing. Weight will have no affect with only 1 animation
			playing.

			\param name Animation set to affect.
			\param weight Blend weight.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetWeight(const int8 *name, f32 weight) = 0;

		//! Sets the wrap mode.
		/*!
			\param name Animation set to affect.
			\param mode Wrap mode.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetWrapMode(const int8 *name, eANIMATION_WRAP_MODE mode) = 0;

		//! Sets the blend mode.
		/*!
			\param name Animation set to affect.
			\param mode Blend mode.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetBlendMode(const int8 *name, eANIMATION_BLEND_MODE mode) = 0;

		//! Set an animations sets track.
		/*!
			Giving an animation set a higher track number will give
			it priority when blending animations. 
			It can also be used to determine which animations are affected
			by certain methods, for example, transitions.
			There is no limit on the track numbers.

			\param name Animation set to affect.
			\param track Track ID.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS SetTrack(const int8 *name, uint32 track) = 0;

		//! Returns true if the animation is playing.
		/*!
			\param name Animation set name.
			\return True if the animation set is playing. False otherwise.
		*/
		virtual bool GetIsPlaying(const int8 *name)const = 0;

		//! Returns the current frame.
		/*!
			\param name Animation set name.
			\return Current frame.
		*/
		virtual int32 GetFrame(const int8 *name)const = 0;

		//! Returns the speed.
		/*!
			\param name Animation set name.
			\return Animation speed.
		*/
		virtual f32 GetSpeed(const int8 *name)const = 0;

		//! Returns the weight.
		/*!
			\param name Animation set name.
			\return Animation weight.
		*/
		virtual f32 GetWeight(const int8 *name)const = 0;

		//! Returns the wrap mode.
		/*!
			\param name Animation set name.
			\return Wrap mode.
		*/
		virtual eANIMATION_WRAP_MODE GetWrapMode(const int8 *name)const = 0;

		//! Returns the wrap mode.
		/*!
			\param name Animation set name.
			\return Wrap mode.
		*/
		virtual eANIMATION_BLEND_MODE GetBlendMode(const int8 *name)const = 0;

		//! Returns the track ID.
		/*!
			\param name Animation set name.
			\return Track ID.
		*/
		virtual uint32 GetTrack(const int8 *name)const = 0;

		//! Set the frame rate for the animation sets.
		/*!
			Common FPS settings are 24, 25, and 30. You should set this
			number to be the same as in the modeling package used to
			created the animation.

			\param framesPerSecond FPS.
		*/
		virtual void SetFrameRate(int32 framesPerSecond) = 0;

		//! Gets the frame rate for the animation sets.
		/*!
			\return Frames per second.
		*/
		virtual int32 GetFrameRate()const = 0;

		//! Returns the number of animation sets in this controller.
		/*!
			\return Animation set count.
		*/
		virtual uint32 GetAnimationCount()const = 0;

		//! Returns the names of each animation set.
		/*!
			\namesOut Animation set names.
		*/
		virtual void GetAnimationNames(MashArray<MashStringc> &namesOut)const = 0;

		//! Utility function. Gathers all the scene nodes affected by this mixer.
		/*!
			\param sceneNodesOut This array will be filled with the affected nodes.
		*/
		virtual void GetAffectedSceneNodes(MashArray<MashSceneNode*> &sceneNodesOut) = 0;

		//! Gets the frame length of an animation.
		/*!
			This is different to key length.

			\param name Animation name.
			\return Frame length of the animation.
		*/
		virtual int32 GetFrameLength(const int8 *name)const = 0;

		//! Sets the animation back to its first key.
		/*!
			\param name Animation name.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS ResetBackToStart(const int8 *name) = 0;

		//! Internal use only. Advances the timers.
		/*!
			This will be called by AdvanceTimeIfNeeded().

			This is called BEFORE _ForceAdvanceAnimation().
			\param dt Time elapsed since the last update.
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS _ForceAdvanceTime(f32 dt) = 0;
        
		//! Internal use only. Advances the frames.
		/*!
			This will be called by AdvanceAnimationIfNeeded().

			This is called AFTER _ForceAdvanceAnimation().
			\return OK if no errors occured. FAILED otherwise.
		*/
		virtual eMASH_STATUS _ForceAdvanceAnimation() = 0;
	};
}

#endif