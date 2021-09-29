//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashPhysicsCollisionShape.h"
#include "MashMemory.h"

namespace mash
{
	CMashPhysicsCollisionShape::~CMashPhysicsCollisionShape()
	{
		if (m_collisionShape)
		{
			MASH_DELETE_T(btCollisionShape, m_collisionShape);
			m_collisionShape = 0;
		}

		if (m_triangleArray)
		{
			MASH_DELETE_T(btTriangleIndexVertexArray, m_triangleArray);
			m_triangleArray = 0;
		}
        
		for(uint32 i = 0; i < m_triangleBuffers.Size(); ++i)
			m_triangleBuffers[i]->Drop();

		m_triangleBuffers.Clear();
	}

	void CMashPhysicsCollisionShape::SetTriangleArray(btTriangleIndexVertexArray *triArray)
	{
		if (m_triangleArray)
			MASH_DELETE_T(btTriangleIndexVertexArray, m_triangleArray);

		m_triangleArray = triArray;
	}

	void CMashPhysicsCollisionShape::SetTriangleBuffers(const MashArray<MashTriangleBuffer*> &usedTriangleBuffers)
	{
		uint32 triangleBufferCount = usedTriangleBuffers.Size();
		for(uint32 i = 0; i < triangleBufferCount; ++i)
			usedTriangleBuffers[i]->Grab();

		triangleBufferCount = m_triangleBuffers.Size();
		for(uint32 i = 0; i < triangleBufferCount; ++i)
			m_triangleBuffers[i]->Drop();

		m_triangleBuffers = usedTriangleBuffers;
	}
}