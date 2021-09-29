//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashBoneSceneNode.h"
#include "MashSceneManager.h"
#include "MashLog.h"

namespace mash
{
	CMashBoneSceneNode::CMashBoneSceneNode(MashSceneManager *pManager,
			MashSceneNode *parent,
			const int8 *boneName,
			uint32 boneSceneId):MashBone(pManager, parent, boneName),
				m_mInvOffset(),
				m_boundingBox(mash::MashVector3(-0.5f, -0.5f, -0.5f), mash::MashVector3(0.5f, 0.5f, 0.5f)),
				m_boneSceneId(boneSceneId), m_sourceBoneSceneId(-1)
				
	{
		m_mInvOffset.Invert();
	}

	CMashBoneSceneNode::~CMashBoneSceneNode()
	{
	}

	MashSceneNode* CMashBoneSceneNode::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		CMashBoneSceneNode *newBone = (CMashBoneSceneNode*)m_sceneManager->AddBone(parent, name);

		if (!newBone)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to create bone instance pointer.", 
						"CMashBoneSceneNode::_CreateInstance");

			return 0;
		}

		newBone->InstanceMembers(this);
		newBone->m_sourceBoneSceneId = m_boneSceneId;
		newBone->m_boundingBox = m_boundingBox;
		newBone->m_mInvOffset = m_mInvOffset;
		newBone->m_boundingBox = m_boundingBox;
		newBone->m_translation = m_translation;
		newBone->m_rotation = m_rotation;
		newBone->m_scale = m_scale;

		return newBone;
	}

	void CMashBoneSceneNode::SetLocalBindPose(const mash::MashVector3 &translation, 
			const mash::MashQuaternion &rotation, 
			const mash::MashVector3 &scale)
	{
		m_translation = translation;
		m_rotation = rotation;
		m_scale = scale;
	}

	void CMashBoneSceneNode::SetWorldBindPose(const MashMatrix4 &mOffset, bool isInverse)
	{
		m_mInvOffset = mOffset;

		if (!isInverse)
			m_mInvOffset.Invert();
	}

	void CMashBoneSceneNode::GetWorldSkinningOffset(mash::MashMatrix4 &out)
	{
		out = m_mInvOffset * GetRenderTransformation();
	}
}