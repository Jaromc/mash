//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PHYSICS_DEBUG_DRAW_H_
#define _C_MASH_PHYSICS_DEBUG_DRAW_H_

#include "MashReferenceCounter.h"
#include "LinearMath/btIDebugDraw.h"
#include "MashSceneManager.h"

namespace mash
{
	class CMashPhysicsDebugDraw : public btIDebugDraw, public MashReferenceCounter
	{
	private:
		MashSceneManager *m_sceneManager;
		int32 m_debugMode;
	public:
		CMashPhysicsDebugDraw(MashSceneManager *sceneManager):MashReferenceCounter(),m_sceneManager(sceneManager), m_debugMode(0){}
		virtual ~CMashPhysicsDebugDraw(){}

		void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);

		void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int32 lifeTime,const btVector3& color){}
		void	reportErrorWarning(const int8* warningString){}
		void	draw3dText(const btVector3& location,const int8* textString){}
		void	setDebugMode(int32 debugMode){m_debugMode = debugMode;}
		int32		getDebugMode() const{return m_debugMode;}
	};
}

#endif