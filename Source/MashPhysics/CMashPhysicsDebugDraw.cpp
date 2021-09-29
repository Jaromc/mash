//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashPhysicsDebugDraw.h"

namespace mash
{
	void CMashPhysicsDebugDraw::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		MashVertexColour::sMashVertexColour verts[2];
		verts[0].position.x = from.getX();
		verts[0].position.y = from.getY();
		verts[0].position.z = from.getZ();
		sMashColour4 aeroColour(color.getX(), color.getY(), color.getZ(), 1.0f);
		verts[0].colour = aeroColour.ToColour();

		verts[1].position.x = to.getX();
		verts[1].position.y = to.getY();
		verts[1].position.z = to.getZ();
		aeroColour.Set(color.getX(), color.getY(), color.getZ(), 1.0f);
		verts[1].colour = aeroColour.ToColour();

		m_sceneManager->DrawLines(verts, 2);
	}
}