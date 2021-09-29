//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_BONE_SCENE_NODE_H_
#define _C_MASH_BONE_SCENE_NODE_H_

#include "MashBone.h"

namespace mash
{
	class CMashBoneSceneNode : public MashBone
	{
	protected:
		MashMatrix4 m_mInvOffset; //used to calculate the difference from the bind pose to the current transformation

		mash::MashAABB m_boundingBox;

		//TODO : Rename these so they dont conflict with node data
		mash::MashVector3 m_translation;
		mash::MashQuaternion m_rotation;
		mash::MashVector3 m_scale;

		//used when cloning skins
		uint32 m_boneSceneId;
		uint32 m_sourceBoneSceneId;

	public:
		CMashBoneSceneNode(MashSceneManager *pManager,
			MashSceneNode *parent,
			/*MashSkeleton *skeletonOwner,*/
			//uint32 boneID, 
			const int8 *boneName,
			uint32 boneCreationId);

		virtual ~CMashBoneSceneNode();

		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		void GetWorldSkinningOffset(mash::MashMatrix4 &out);
		
		//! Sets the world bind pose (for animations).
		/*!
			This matrix is used to determine if any movement has occured from
			the original bind pose to the current world transform.
			The resulting difference is used for skinning.

			\param worldBind World bind pose matrix.
		*/
		void SetWorldBindPose(const mash::MashMatrix4 &worldBind, bool isInverse);

		//! Gets the world bind pose (for animations).
		/*!
			\return World bind pose.
		*/
		const mash::MashMatrix4& GetInverseWorldBindPose()const;

		void SetLocalBindPose(const mash::MashVector3 &translation, 
			const mash::MashQuaternion &rotation, 
			const mash::MashVector3 &scale);

		const mash::MashAABB& GetLocalBoundingBox()const;
		const mash::MashVector3& GetLocalBindPosition()const;
		const mash::MashVector3& GetLocalBindScale()const;
		const mash::MashQuaternion& GetLocalBindRotation()const;

		uint32 GetNodeType()const;
		uint32 GetBoneSceneId()const;
		uint32 GetSourceBoneSceneId()const;
	};

	inline uint32 CMashBoneSceneNode::GetSourceBoneSceneId()const
	{
		return m_sourceBoneSceneId;
	}

	inline uint32 CMashBoneSceneNode::GetBoneSceneId()const
	{
		return m_boneSceneId;
	}

	inline const mash::MashVector3& CMashBoneSceneNode::GetLocalBindPosition()const
	{
		return m_translation;
	}

	inline const mash::MashVector3& CMashBoneSceneNode::GetLocalBindScale()const
	{
		return m_scale;
	}

	inline const mash::MashQuaternion& CMashBoneSceneNode::GetLocalBindRotation()const
	{
		return m_rotation;
	}

	inline const mash::MashMatrix4& CMashBoneSceneNode::GetInverseWorldBindPose()const
	{
		return m_mInvOffset;
	}

	inline const mash::MashAABB& CMashBoneSceneNode::GetLocalBoundingBox()const
	{
		return m_boundingBox;
	}

	inline uint32 CMashBoneSceneNode::GetNodeType()const
	{
		return aNODETYPE_BONE;
	}
}

#endif