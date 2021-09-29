//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PHYSICS_COLLISION_SHAPE_H_
#define _C_MASH_PHYSICS_COLLISION_SHAPE_H_

#include "MashPhysicsCollisionShape.h"
#include "MashTriangleBuffer.h"
#include "MashArray.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h"

namespace mash
{
	class CMashPhysicsCollisionShape : public MashPhysicsCollisionShape
	{
	private:
		btCollisionShape *m_collisionShape;
		btTriangleIndexVertexArray *m_triangleArray;
		MashArray<MashTriangleBuffer*> m_triangleBuffers;
		bool m_isConcave;
	public:
		CMashPhysicsCollisionShape(btCollisionShape *collisionShape, bool isConcave):MashPhysicsCollisionShape(), m_collisionShape(collisionShape), m_isConcave(isConcave),
			m_triangleArray(0)
		{
			for(uint32 i = 0; i < m_triangleBuffers.Size(); ++i)
				m_triangleBuffers[i]->Grab();
		}

		~CMashPhysicsCollisionShape();

		btCollisionShape* GetBulletCollsionShape()const;

		/*
			Triangle buffers are grabbed here and dropped when this object is destroyed.

			TODO : Is it really needed to Grab() and store these?
		*/
		void SetTriangleBuffers(const MashArray<MashTriangleBuffer*> &usedTriangleBuffers);

		void SetTriangleArray(btTriangleIndexVertexArray *triArray);

		bool IsConcave()const;
	};

	inline bool CMashPhysicsCollisionShape::IsConcave()const
	{
		return m_isConcave;
	}

	inline btCollisionShape* CMashPhysicsCollisionShape::GetBulletCollsionShape()const
	{
		return m_collisionShape;
	}
}

#endif