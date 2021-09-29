//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_RENDER_INFO_H_
#define _MASH_RENDER_INFO_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashTypes.h"

namespace mash
{
	class MashLight;
	class MashCamera;
	class MashTechniqueInstance;
	class MashVertex;
	class MashEffect;
	class MashShadowCaster;
	class MashParticleSystem;
	class MashSkin;
	class MashMatrix4;
	class MashTexture;

    /*!
        This is a blackboard for the current render state. Renderable objects write to
        this blackboard before being rendered so that GPU auto parameters can grab that data, 
        pack it if needed, then send it on to the GPU.
     
        Some data here is only valid depending on what is currently being rendered. For example,
        an Entity will not write particle data here. So using shader autos that are made only for particles
        in an entity shader may result in an error. 
     
        The only time a user may need to access this is when implimenting their own auto parameters.
    */
	class MashRenderInfo : public MashReferenceCounter
	{
	public:
		MashRenderInfo():MashReferenceCounter(){}
		virtual ~MashRenderInfo(){}
		
        //! Gets the bone palette.
        /*!
            The bone palette contains an array of matrices that describe each bones
            movement from its bind pose. Eg, if the bone is still in its bind pose
            then it will have an identity matrix.
            Array elements match the bone indices contained in each vertex of a skinned mesh.
         
            \return Bone palette.
        */
		virtual MashMatrix4* GetBonePalette()const = 0;
        
        //! Max number of elements the current bone array can hold.
		virtual uint32 GetMaximumBoneCount()const = 0;
        
        //! Last number of bones that were written to the bone array.
		virtual uint32 GetCurrentBoneCount()const = 0;
        
        //! Gets the current world transform.
		virtual const mash::MashMatrix4& GetWorldTransform()const = 0;
        
        //! Gets the current technique.
		virtual MashTechniqueInstance* GetTechnique()const = 0;
        
        //! Gets the current effect. This just comes from GetTechnique().
		virtual MashEffect* GetEffect()const = 0;

        //! Gets the current light.
		virtual MashLight* GetLight()const = 0;

        //! Gets the scene diffuse texture used in the deferred renderer. This is valid after rendering a deferred scene.
		virtual MashTexture* GetSceneDiffuseMap()const = 0;
        
        //! Gets the scene specular texture used in the deferred renderer. This is valid after rendering a deferred scene.
		virtual MashTexture* GetSceneSpecularMap()const = 0;
        
        //! Gets the scene normal texture used in the deferred renderer. This is valid after rendering a deferred scene. 
		virtual MashTexture* GetSceneNormalMap()const = 0;
        
        //! Gets the scene depth texture used in the deferred renderer. This is valid after rendering a deferred scene.
		virtual MashTexture* GetSceneDepthMap()const = 0;
        
        //! Gets the light specular texture used in the deferred renderer. This is valid after rendering a deferred scene.
        /*!
            This is the final combined specular texture after lighting has been calculated.
        */
		virtual MashTexture* GetSceneLightSpecMap()const = 0;
        
        //! Gets the scene light texture used in the deferred renderer. This is valid after rendering a deferred scene.
        /*!
            This is the final light map with lighting calculated.
        */
		virtual MashTexture* GetSceneLightMap()const = 0;
        
        //! Gets the current skin.
		virtual MashSkin* GetSkin()const = 0;
        
        //! Gets the active camera.
        virtual MashCamera* GetCamera()const = 0;
        
        //! Gets the current particle system.
		virtual const MashParticleSystem* GetParticleSystem()const = 0;
        
        //! Gets the current light buffer.
        /*!
            This is an array of sMashLight. For forward rendered scene this contains all the light data
            for all forward rendered lights. In deferred renderers this is usually only one element.
         
            \param sizeInBytesOut Total byte size of the light data array (active elements only).
            \return Light buffer array.
         */
		virtual void* GetLightBuffer(uint32 *sizeInBytesOut)const = 0;
        
        //! Gets the active shadow caster.
		virtual MashShadowCaster* GetShadowCaster()const = 0;

		//! Gets the active vertex declaration.
		virtual MashVertex* GetVertex()const = 0;
        
        //! Sets the current skin.
        virtual void SetSkin(MashSkin *skin) = 0;
        
        //! Sets the current list.
		virtual void SetLight(MashLight *light) = 0;

		//! Sets the current number of bones in the bone array.
        /*!
            This function will NOT resize the internal array for speed reasons.
            SetBonePaletteMinimumSize() must be called to do this.
        */
		virtual void SetCurrentBonePaletteSize(uint32 boneCount) = 0;
		
        //! Sets the maximum size of the bone array.
        /*!
            Used only during initialization to set the maximum size of the bone array.
            If this number is larger than whats already set then the array will
            be resized. The internal array will not change if this number is less then
            whats already set.
        */
		virtual void SetBonePaletteMinimumSize(uint32 boneCount) = 0;
	
        //! Sets the current world transform.
		virtual void SetWorldTransform(const MashMatrix4 &mWorld) = 0;

        //! Sets the scene diffuse map calculated in deferred renderer.
		virtual void SetSceneDiffuseMap(MashTexture *texture) = 0;
        
        //! Sets the scene specular map calculated in the deferred renderer.
		virtual void SetSceneSpecularMap(MashTexture *texture) = 0;
        
        //! Sets the scene normal map calculated in the deferred renderer.
		virtual void SetSceneNormalMap(MashTexture *texture) = 0;
        
        //! Sets the scene depth map calculated in the deferred renderer.
		virtual void SetSceneDepthMap(MashTexture *texture) = 0;
        
        //! Sets the final combined light map calculated in the deferred renderer.
		virtual void SetSceneLightMap(MashTexture *texture) = 0;
        
        //! Sets the final combined spec map calculated in the deferred renderer.
		virtual void SetSceneLightSpecMap(MashTexture *texture) = 0;

        //! Sets the active technique.
		virtual void SetTechnique(MashTechniqueInstance *technique) = 0;
	
        //! Sets the active camera.
		virtual void SetCamera(MashCamera *camera) = 0;

        //! Sets the current particle system.
		virtual void SetParticleSystem(MashParticleSystem *particleSystem) = 0;
        
        //! Sets the light buffer.
        /*!
            \param lightArray Array of light data.
            \param count Number of active elements in the array.
        */
		virtual void SetLightBuffer(const sMashLight *lightArray, uint32 count) = 0;

        //! Sets the active shadow caster.
		virtual void SetShadowCaster(MashShadowCaster *caster) = 0;

		//! Sets the active vertex declaration.
		virtual void SetVertex(MashVertex *vertexDecl) = 0;
	};
}

#endif