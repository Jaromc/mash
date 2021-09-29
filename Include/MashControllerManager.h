//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_CONTROLLER_MANAGER_H_
#define _MASH_CONTROLLER_MANAGER_H_

#include "MashReferenceCounter.h"
#include "MashArray.h"
#include "MashString.h"

namespace mash
{
	class MashTransformationKeySet;
	class MashTransformationController;
	class MashSceneNode;
	class MashAnimationBuffer;
	class MashAnimationMixer;
    struct sAnimationClip;

    /*!
        Manager for animation objects.
    */
	class MashControllerManager : public MashReferenceCounter
	{
	public:
		MashControllerManager():MashReferenceCounter(){}

		//! Chops a buffer into smaller buffers.
		/*!
			Transforms one large animation buffer into smaller buffers, such as walk, run, jump, etc..
			This must be used before an animation mixer uses it otherwise the mixer will need
			to be recreated.

			This is best used if the node you pass in is the parent of all nodes affected by that same
			animations. For example, The root node contains all the bones thats affect an entity.

			\param root Node that contains a animation buffer.
			\param clips These describe how to chop this buffer into new smaller buffers.
			\param processChildren Set to true if you want the children to be processed too.
			\return Ok if everything was fine, failed otherwise.
		*/
		virtual eMASH_STATUS ChopAnimationBuffers(MashSceneNode *root, const MashArray<sAnimationClip> &clips, bool processChildren = true) = 0;

		//! Creates a new animation buffer.
		/*!
			\return Empty animation buffer.
		*/
		virtual MashAnimationBuffer* CreateAnimationBuffer() = 0;

		//! Creates a transform controller for a node.
		/*!
			The returned controller will affect only the node passed in.

			\param keys A set of keys that describe a nodes animation.
			\param node The node that will be affected by the key sets and the controller created.
			\return A new transform controller. This can then be added to an animation mixer.
		*/
		virtual MashTransformationController* CreateTransformController(MashTransformationKeySet *keys, MashSceneNode *node) = 0;

		//! Creates an empty key set.
		/*!
			\return An empty key set.
		*/
		virtual MashTransformationKeySet* CreateTransformationKeySet() = 0;
		
		//! Create an empty animation mixer.
		/*!
			An animation mixer is the main interface a user will manipulate animations for a node
			or a set of nodes. See MashAnimationMixer for more info.

			\return An empty animation mixer.
		*/
		virtual MashAnimationMixer* CreateMixer() = 0;

		//! Creates a mixer from animation buffer data held in a node graph.
		/*!
			An animation mixer is the main interface a user will manipulate animations for a node
			or a set of nodes. See MashAnimationMixer for more info.

			The mixer will be added to the root node. The returned maixer should not be dropped.

			\param root Node to start processing.
			\param processChildren Should all the children be processed too.
			\return An animation mixer that controlls all the nodes in the graph.
		*/
		virtual MashAnimationMixer* CreateMixer(MashSceneNode *root, bool processChildren = true) = 0;

		//! Adds a node graphs animation buffers to a mixer.
		/*!
			\param mixer Mixer to add animations to.
			\param root A node with an animation buffer. The buffer will be added to the mixer. Children are not processed.
		*/
		virtual void AddAnimationsToMixer(MashAnimationMixer *mixer, MashSceneNode *root) = 0;

		//! Updates all the controllers.
		/*!
			This is called automatically by the engine when updated. So the user should never
			need to call this.

			\param dt Time elapsed since last call.
		*/
		virtual void Update(f32 dt) = 0;

		//! Internal use only. Called to remove a mixer.
		/*!
			Should not be called directly. To delete a controller call MashReferenceCounter::Drop()
			\param controller Controller to remove.
		*/
		virtual void _RemoveAnimationMixer(MashAnimationMixer *controller) = 0;
	};
}

#endif