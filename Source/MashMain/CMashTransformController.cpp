//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTransformController.h"
#include "MashSceneNode.h"
#include "MashBone.h"

namespace mash
{
	CMashTransformationController::CMashTransformationController(MashTransformationKeySet *sharedKeys, 
		mash::MashSceneNode *instance):MashTransformationController(), m_owner(instance), m_keySet(sharedKeys)
	{
		if (m_keySet)
			m_keySet->Grab();

		if (m_owner->GetNodeType() == aNODETYPE_BONE)
		{
			m_initialState.position = ((MashBone*)m_owner)->GetLocalBindPosition();
			m_initialState.rotation =((MashBone*)m_owner)->GetLocalBindRotation();
			m_initialState.scale = ((MashBone*)m_owner)->GetLocalBindScale();
		}
		else
		{
			m_initialState.position = m_owner->GetLocalTransformState().translation;
			m_initialState.rotation = m_owner->GetLocalTransformState().orientation;
			m_initialState.scale = m_owner->GetLocalTransformState().scale;
		}
	}

	CMashTransformationController::~CMashTransformationController()
	{
		if (m_keySet)
		{
			m_keySet->Drop();
			m_keySet = 0;
		}
	}

	MashSceneNode* CMashTransformationController::GetOwner()const
	{
		return m_owner;
	}

	void CMashTransformationController::AnimationStart()
	{
	}

	void CMashTransformationController::AnimateToKey(uint32 key)
	{
		const sMashAnimationKeyTransform *currentKey = m_keySet->GetKey(key);

		m_transformBuffer.position = currentKey->positionKey;
		m_transformBuffer.scale = currentKey->scaleKey;
		m_transformBuffer.rotation = currentKey->rotationKey;
	}

	void CMashTransformationController::AnimateForward(uint32 fromKey, uint32 toKey, f32 amount)
	{
		const sMashAnimationKeyTransform *startKey = m_keySet->GetKey(fromKey);
		const sMashAnimationKeyTransform *endKey = m_keySet->GetKey(toKey);

		m_transformBuffer.position = startKey->positionKey + ((endKey->positionKey - startKey->positionKey) * amount);
		m_transformBuffer.scale = startKey->scaleKey + ((endKey->scaleKey - startKey->scaleKey) * amount);
		m_transformBuffer.rotation.Slerp(startKey->rotationKey, endKey->rotationKey, amount);
	}

	void CMashTransformationController::AnimationEnd(eANIMATION_BLEND_MODE blend, f32 blendAmount)
	{
		const MashTransformState *currentTransform = &m_owner->GetLocalTransformState();

		if (blend == aBLEND_ADDITIVE)
		{
			m_transformBuffer.position = currentTransform->translation + (m_transformBuffer.position - m_initialState.position);
			m_transformBuffer.scale = currentTransform->scale + (m_transformBuffer.scale - m_initialState.scale);

			mash::MashQuaternion invRot = m_initialState.rotation;
			invRot.Conjugate();//TODO : Do this at init time
			m_transformBuffer.rotation = currentTransform->orientation * (invRot * m_transformBuffer.rotation);
		}
		else//blend
		{
			m_transformBuffer.position = currentTransform->translation + ((m_transformBuffer.position - currentTransform->translation) * blendAmount);
			m_transformBuffer.scale = currentTransform->scale + ((m_transformBuffer.scale - currentTransform->scale) * blendAmount);
			m_transformBuffer.rotation.Slerp(currentTransform->orientation,m_transformBuffer.rotation, blendAmount);
		}

		m_owner->SetPosition(m_transformBuffer.position);
		m_owner->SetScale(m_transformBuffer.scale);
		m_owner->SetOrientation(m_transformBuffer.rotation);
	}
}