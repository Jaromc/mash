//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_CAMERA_CULL
#define _C_MASH_CAMERA_CULL

#include "MashCullTechnique.h"

namespace mash
{
	class MashSceneManager;

	class MashCamera;

	class CMashCameraCull : public MashCullTechnique
	{
	private:
		MashSceneManager *m_pSceneManager;
		MashCamera *m_activeCamera;

		static bool CullSceneRenderable(MashRenderable *renderable);

		void _CullScene(MashSceneNode *scene);
	public:
		CMashCameraCull(MashSceneManager *m_pSceneManager);
		virtual ~CMashCameraCull();

		void CullScene(MashSceneNode *pScene);
	};
}

#endif