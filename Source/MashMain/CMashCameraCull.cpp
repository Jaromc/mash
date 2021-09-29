//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashCameraCull.h"
#include "MashSceneManager.h"
#include "MashAABB.h"
#include "MashCamera.h"
#include "MashLight.h"
#include "MashRenderable.h"
#include "MashTimer.h"
#include "MashDevice.h"
namespace mash
{
	CMashCameraCull::CMashCameraCull(MashSceneManager *pSceneManager):MashCullTechnique(),m_pSceneManager(pSceneManager)
	{
	}

	CMashCameraCull::~CMashCameraCull()
	{
		m_pSceneManager = 0;
	}

	bool CMashCameraCull::CullSceneRenderable(MashRenderable *renderable)
	{
		return false;
	}

	void CMashCameraCull::_CullScene(MashSceneNode *scene)
	{
		bool passedCull = false;

		/*
			children are rendered even if the parent is not visible
		*/
        if (scene == m_activeCamera)//this has been previously passed in the scene manager
        {
            passedCull = true;
        }
        else
        {
            if (aNODETYPE_LIGHT & scene->GetNodeType())
            {
                mash::MashLight *pLight = (mash::MashLight*)scene;
                
                if (pLight->IsLightEnabled())
                {
                    
                    /*
                     Only consider lights with a range that extends within the view frustum.
                     
                     Light culling is really only used during deferred rendering to stop
                     lights outside the view frustum from generating unnecessary shadows
                     and render passes.
                     */	
                    f32 lightRange = pLight->GetLightData()->range;
                    MashVector3 lightPos = pLight->GetWorldTransformState().translation;
                    mash::MashAABB lightRangeBounds(MashVector3(lightPos.x - lightRange, lightPos.y - lightRange, lightPos.z - lightRange),
                                                  MashVector3(lightPos.x + lightRange, lightPos.y + lightRange, lightPos.z + lightRange));
                    
                    if (!m_activeCamera->IsCulled(lightRangeBounds))
                        scene->OnCullPass();
                    
                    if (!m_activeCamera->IsCulled(scene->GetTotalBoundingBox()))
                        passedCull = true;
                }
            }
            
            if (!passedCull)
            {
                //first test if only the object is visible. In this case it will be rendered.
                if (!m_activeCamera->IsCulled(scene->GetWorldBoundingBox()))
                {
                    scene->OnCullPass();
                    passedCull = true;
                    
                    if (scene->IsVisible() && scene->ContainsRenderables())
                        scene->AddRenderablesToRenderQueue(aRENDER_STAGE_SCENE, CullSceneRenderable);
                }
                //if its not visible then maybe its children are.
                else if (!m_activeCamera->IsCulled(scene->GetTotalBoundingBox()))
                {
                    passedCull = true;
                }
            }
        }

		if (passedCull)
		{
			MashList<mash::MashSceneNode*>::ConstIterator iter = scene->GetChildren().Begin();
			MashList<mash::MashSceneNode*>::ConstIterator end = scene->GetChildren().End();
			for(; iter != end; ++iter)
			{
				_CullScene(*iter);
			}
		}
	}

	void CMashCameraCull::CullScene(MashSceneNode *scene)
	{
		m_activeCamera = m_pSceneManager->GetActiveCamera();

		if (!m_activeCamera)
			return;

		_CullScene(scene);
	}
}