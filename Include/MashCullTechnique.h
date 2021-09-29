//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_CULL_TECHNIQUE_H_
#define _MASH_CULL_TECHNIQUE_H_

#include "MashReferenceCounter.h"

namespace mash
{
	class MashSceneNode;
	class MashRenderable;

	/*!
		This class can be impliemented by the user to create custom culling techniques.

		Use MashSceneManager::SetCullTechnique() and MashSceneManager::SetCullTechniqueShadow() or create
		built in techniques with MashSceneManager::CreateCullTechnique()
	*/
	class MashCullTechnique : public MashReferenceCounter
	{
	public:
		/*!
			Scene nodes arn't necessarily the renderable as well (Eg, an entity is the scene node
			and it owns subEntities that are the renderables). So after high level node testing has
			been done (eg, frustum vs aabb) this can be passed into the MashSceneNode::AddRenderablesToRenderQueue
			so any further testing can be performed on the actual renderables.

			A simple implimentation of this simply returns false to render.

			\code
			bool CullSceneRenderable(MashRenderable *renderable)
			{
				return false;
			}
			\endcode
         
            A shadow cull function may look like:
            \code
            bool CullShadowCasterRnderable(MashRenderable *renderable)
            {
                if(renderable->MashTechnique->ContainsValidShadowCaster())
                    return false;
         
                return true;
            }
            \endcode
         
            ContainsValidShadowCaster

			\param renderable Renderable to perform further testing on.
			\return True to cull. False to render.
		*/
		typedef bool(*CullRenderableFunctPtr)(MashRenderable *renderable);
	public:
		MashCullTechnique():MashReferenceCounter(){}
		virtual ~MashCullTechnique(){}

		//! Culls a scene graph.
		/*!
			Any nodes that pass culling are added to the scene manager for rendering.
			If a node passes culling then it should also have MashSceneNode::OnCullPass()
			called so it's ready for rendering. MashSceneNode::OnCullPass() allows nodes
			to delay processing of things that are only needed if the renderables they
			own are be rendered. This should be followed by a call to
            MashSceneNode::AddRenderablesToRenderQueue(). Example,
         
            \code
            bool recurseChildren = false;
            if (!activeCamera->IsCulled(sceneNode->GetWorldBoundingBox()))
            {
                sceneNode->OnCullPass(interpolatedTime, frameCount);
                sceneNode->AddRenderablesToRenderQueue(aRENDER_STAGE_SCENE);
                recurseChildren = true;
            }
            else if (!activeCamera->IsCulled(sceneNode->GetTotalBoundingBox()))
            {
                recurseChildren = true;
            }
         
            if (recurseChildren)
            {
                for each child in sceneNode->children
                CullScene(child);
            }
            \endcode
         
            For shadow culling you would use aRENDER_STAGE_SHADOW in place of aRENDER_STAGE_SCENE
            as the parameter to MashSceneNode::AddRenderablesToRenderQueue().

			\param scene Scene to cull.
			\param interpolatedTime Interpolated frame time.
		*/
		virtual void CullScene(MashSceneNode *scene) = 0;
	};
}

#endif