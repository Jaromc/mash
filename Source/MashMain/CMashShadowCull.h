//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SHADOW_CULL
#define _C_MASH_SHADOW_CULL

#include "MashCullTechnique.h"

namespace mash
{
	class MashSceneManager;

	class CMashShadowCull : public MashCullTechnique
	{
	private:
		MashSceneManager *m_pSceneManager;

		static bool CullShadowRenderable(MashRenderable *renderable);

		void _CullScene(MashSceneNode *scene);
	public:
		CMashShadowCull(MashSceneManager *m_pSceneManager);
		virtual ~CMashShadowCull();

		virtual void CullScene(MashSceneNode *pScene);
	};
}

#endif