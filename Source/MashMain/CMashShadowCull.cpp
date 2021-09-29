//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashShadowCull.h"
#include "MashSceneManager.h"
#include "MashRenderable.h"
#include "MashTimer.h"
#include "MashDevice.h"
#include "MashSceneNode.h"
#include "MashMaterial.h"
#include "MashTechnique.h"
#include "MashTechniqueInstance.h"

namespace mash
{
	CMashShadowCull::CMashShadowCull(MashSceneManager *pSceneManager):MashCullTechnique(),m_pSceneManager(pSceneManager)
	{
	}

	CMashShadowCull::~CMashShadowCull()
	{
		m_pSceneManager = 0;
	}

	bool CMashShadowCull::CullShadowRenderable(MashRenderable *renderable)
	{
        MashMaterial *mat = renderable->GetMaterial();
		if (mat->IsValid() &&  mat->GetActiveTechnique()->GetTechnique()->ContainsValidShadowCaster())
			return false;

		return true;
	}

	void CMashShadowCull::_CullScene(MashSceneNode *scene)
	{
        bool passedCull = false;
        
        //unfourtunatly we can't do much culling with shadows because casters
        //may be far behind a camera.
        
		if (scene->IsVisible() && scene->ContainsRenderables())
        {
			if (scene->AddRenderablesToRenderQueue(aRENDER_STAGE_SHADOW, &CullShadowRenderable))
            {
                scene->OnCullPass();
                passedCull = true;
            }
        }

		MashList<mash::MashSceneNode*>::ConstIterator iter = scene->GetChildren().Begin();
		MashList<mash::MashSceneNode*>::ConstIterator end = scene->GetChildren().End();
		for(; iter != end; ++iter)
		{
			_CullScene(*iter);
		}
	}

	void CMashShadowCull::CullScene(MashSceneNode *scene)
	{
		_CullScene(scene);
	}
}