//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SCENE_NODE_CALLBACK_H_
#define _MASH_SCENE_NODE_CALLBACK_H_

#include "MashReferenceCounter.h"

namespace mash
{
	class MashSceneNode;

	/*!
        This can be implimented for custom purposes and added to a scene node using
        MashSceneNode::AddCallback().

		Each callback will have OnNodeUpdate() called automatically once per update.
    */
	class MashSceneNodeCallback : public MashReferenceCounter
	{
	public:
		MashSceneNodeCallback():MashReferenceCounter(){}
		virtual ~MashSceneNodeCallback(){}

        //! Called when this is attached to a node.
		/*!
			\param sceneNode Scene node that owns this.
		*/
		virtual void OnNodeAttach(MashSceneNode *sceneNode){};
        
		//! Called before the node has been updated.
        /*!
            \param sceneNode Scene node that owns this.
            \param dt Time passed since last update.
        */
		virtual void OnNodeUpdate(MashSceneNode *sceneNode, f32 dt) = 0;
		
        //! Called when this object is being removed from a scene node.
        /*!
			\param sceneNode Scene node that owns this.
		*/
		virtual void OnNodeDetach(MashSceneNode *sceneNode){};
	};
}

#endif