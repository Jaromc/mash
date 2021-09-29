//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PHYSICS_MOTION_STATE_H_
#define _C_MASH_PHYSICS_MOTION_STATE_H_

#include "LinearMath/btMotionState.h"
#include "MashSceneNode.h"
#include "MashMatrix4.h"
namespace mash
{
	class CMashPhysicsMotionState : public btMotionState
	{
	private:
		
		mash::MashSceneNode *m_node;
		btTransform m_offset;

		mash::MashMatrix4 m_orig;
	public:
		CMashPhysicsMotionState(mash::MashSceneNode *node, const btTransform &offset):btMotionState(), m_node(node), m_offset(offset)
		{
			m_orig = m_node->GetUpdatedWorldTransformState().ToMatrix();
		}

		~CMashPhysicsMotionState(){}

		void getWorldTransform(btTransform& worldTrans ) const
		{
			mash::MashVector3 pos = m_node->GetUpdatedWorldTransformState().translation;
			worldTrans.getOrigin().setValue(pos.x, pos.y, pos.z);

			mash::MashQuaternion rot = m_node->GetUpdatedWorldTransformState().orientation;
			btQuaternion bulletRot(rot.x, rot.y, rot.z, rot.w);
			worldTrans.getBasis().setRotation(bulletRot);
		}

		void ConvertMatrix(const btTransform &trn, 
			mash::MashVector3 &posOut,
			mash::MashVector3 &scaleOut,
			mash::MashQuaternion &rotOut) 
		{
		   btVector3 R = trn.getBasis().getColumn(0);
		   btVector3 U = trn.getBasis().getColumn(1);
		   btVector3 L = trn.getBasis().getColumn(2);
		   btVector3 P = trn.getOrigin();

		   mash::MashVector3 vR, vU, vL, vP;
		   vR.x = R.x();vR.y = R.y();vR.z = R.z();
		   vU.x = U.x();vU.y = U.y();vU.z = U.z();
		   vL.x = L.x();vL.y = L.y();vL.z = L.z();
		   vP.x = P.x();vP.y = P.y();vP.z = P.z();

		   mash::MashMatrix4 rot, trans, matOutput;
		   rot.m11 = vR.x;rot.m12 = vR.y;rot.m13 = vR.z;rot.m14 = 0.f;
		   rot.m21 = vU.x;rot.m22 = vU.y;rot.m23 = vU.z;rot.m24 = 0.f;
		   rot.m31 = vL.x;rot.m32 = vL.y;rot.m33 = vL.z;rot.m34 = 0.f;
		   rot.m41 = 0.0f;rot.m42 = 0.0f;rot.m43 = 0.0f;rot.m44 = 1.f;

		   posOut = vP;
		   scaleOut = mash::MashVector3(1.0f, 1.0f, 1.0f);
		   rot.ToQuaternion(rotOut);
		}

		void setWorldTransform(const btTransform& worldTrans)
		{
			mash::MashVector3 pos;
			mash::MashVector3 scale;
			mash::MashQuaternion rot;
			
			ConvertMatrix(worldTrans, pos, scale, rot);
            m_node->SetPosition(pos);
            m_node->SetOrientation(rot);
		}
	};
}

#endif