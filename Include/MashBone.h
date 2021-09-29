//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_BONE_SCENE_NODE_H_
#define _MASH_BONE_SCENE_NODE_H_

#include "MashSceneNode.h"

namespace mash
{
    /*!
        Bones are used for skinned animation of entities and can be added to MashSkin.
    */
	class MashBone : public MashSceneNode
	{
	public:
		MashBone(MashSceneManager *manager,
			MashSceneNode *parent,
			const int8 *boneName):MashSceneNode(parent, manager, boneName){}

		virtual ~MashBone(){}

		//! Create a new instance of this node.
		/*!
			\param parent New parent.
			\param name New name.
		*/
		virtual MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name) = 0;

		//! Returns the final skinning matrix.
		/*!
			This matrix is the amount of movement that has occured from the bind pose.
			So if it hasn't moved from its bind pose then this matrix will be an
			identity matrix.

			This method will only be valid after a cull pass. It uses the interpolated
			render matrix to compute the final skinning offset.
		*/
		virtual void GetWorldSkinningOffset(MashMatrix4 &out) = 0;

		//! Sets the world bind pose.
		/*!
            This is the original position of this node in world space.
         
			This matrix is used to determine if any movement has occured from
			the original bind pose to the current world transform.
			The resulting difference is used for skinning.

			\param worldBind World bind pose matrix.
			\param isInverse Is the matrix being passed in the inverse world bind.
		*/
		virtual void SetWorldBindPose(const MashMatrix4 &worldBind, bool isInverse) = 0;

		//! Gets the world bind pose.
		/*!
			\return World bind pose.
		*/
		virtual const MashMatrix4& GetInverseWorldBindPose()const = 0;

		//! Local bind pose.
		/*!
			Called at load time only to determine a bones initial state in
			local space. This is usually the same as the nodes initial position.

			\param translation Translation.
			\param rotation Rotation.
			\param scale Scale.
		*/
		virtual void SetLocalBindPose(const mash::MashVector3 &translation, 
			const MashQuaternion &rotation, 
			const MashVector3 &scale) = 0;

		//! Gets the local bounds.
		/*!
			\return Local bounds.
		*/
		virtual const MashAABB& GetLocalBoundingBox()const = 0;

		//! Gets the bind pose position in local space.
		/*!
			\return Bind pose position in local space.
		*/
		virtual const MashVector3& GetLocalBindPosition()const = 0;

		//! Gets the bind pose scale in local space.
		/*!
			\return Bind pose scale in local space.
		*/
		virtual const MashVector3& GetLocalBindScale()const = 0;

		//! Gets the bind pose rotation in local space.
		/*!
			\return Bind pose rotation in local space.
		*/
		virtual const MashQuaternion& GetLocalBindRotation()const = 0;

		//! Gets a bones unique id.
		/*!
			Used when cloning skins.
			Each bone is given a unique id when created.

			These values are given at runtime and should not be stored or used for save files.
		*/
		virtual uint32 GetBoneSceneId()const = 0;
		
		//! Gets the id of the bone this bone was cloned from.
		/*!
			If this bone was cloned, then this will be the id of the source bone.
			Otherwise -1 is returned.

			These values are given at runtime and should not be stored or used for save files.
		*/
		virtual uint32 GetSourceBoneSceneId()const = 0;
	};
}

#endif