//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LIGHT_H_
#define _MASH_LIGHT_H_

#include "MashSceneNode.h"

namespace mash
{
	class MashTexture;

    /*!
        Description of a light source in a scene.
     
        Lighting in a deferred scene is calculated based on the lights in
        a scene graph when culled. All lights may generate shadows.
     
        In forward rendered scenes, SetForwardRenderedLight() must be called to register a light
        as a light source. This is done to support runtime shader generation.
        Only the main light source may generate shadows in forward rendered scenes.
     
        Shadows may be enabled/disabled using SetShadowsEnabled()
    */
	class MashLight : public MashSceneNode
	{
	public:
		MashLight(MashSceneNode *parent,
			MashSceneManager *manager,
				const MashStringc &userName):MashSceneNode(parent, manager, userName){}

		virtual ~MashLight(){} 

        //! Sets the lights type.
        /*!
            \param type Light type.
        */
		virtual void SetLightType(eLIGHTTYPE type) = 0;
        
        //! Sets default parameters for directional lights.
        /*!
            Does not set the light type.
         
            \param dir Light direction.
        */
		virtual void SetDefaultDirectionalLightSettings(const MashVector3 &dir) = 0;
        
        //! Sets default parameters for spot lights.
        /*!
            Does not set the light type.
         
            \param dir Light direction.
         */
		virtual void SetDefaultSpotLightSettings(const MashVector3 &dir) = 0;
        
        //! Sets default parameters for point lights.
        /*!
            Does not set the light type.
         */
		virtual void SetDefaultPointLightSettings() = 0;

        //! Diffuse colour and intensity.
        /*!
            \param diffuse Diffuse light colour and intensity.
        */
		virtual void SetDiffuse(const sMashColour4 &diffuse) = 0;
        
        //! Specular colour and intensity.
        /*!
            \param specular Specular light colour and intensity.
         */
		virtual void SetSpecular(const sMashColour4 &specular) = 0;
        
        //! Ambient colour and intensity.
        /*!
            \param ambient Ambient light colour and intensity.
         */
		virtual void SetAmbient(const sMashColour4 &ambient) = 0;
        
        //! Light direction.
        /*!
            \param direction Light direction.
        */
		//virtual void SetDirection(const MashVector3 &direction) = 0;

        //! Lights inner cone. For spot lights only.
        /*!
            \param angleInRadians Angle in radians.
        */
		virtual void SetInnerCone(f32 angleInRadians) = 0;
        
        //! Lights outer cone. For spot lights only.
        /*!
            \param angleInRadians Angle in radians.
        */
		virtual void SetOuterCone(f32 angleInRadians) = 0;
        
        //! Light range. For Spot and point lights only.
        /*!
            \param range Light range.
        */
		virtual void SetRange(f32 range) = 0;
        
        //! Falloff. For Spot and point lights only.
        /*!
            \param falloff Affects how the light fades out over a distance.
        */
		virtual void SetFalloff(f32 falloff) = 0;
        
        //! Attenuation. For spot and point lights only.
        /*!
            \param attenuation0 Attenuation 0.
            \param attenuation1 Attenuation 1.
            \param attenuation2 Attenuation 2.
        */
		virtual void SetAttenuation(f32 attenuation0, f32 attenuation1, f32 attenuation2) = 0;
        
        //! Lighting type.
        /*!
            \return Lighting type.
        */
		virtual eLIGHTTYPE GetLightType()const = 0;
        
        //! AABB range test.
        /*!
            Tests whether an AABB in world space is within range of this light.
            \param aabb AABB to test.
            \return True is it's in range. False otherwise.
        */
		virtual bool IsAABBInRange(const MashAABB &aabb)const = 0;
        
        //! Internal light data.
        /*!
            Mainly used for shaders.
            \return Light data.
        */
		virtual const sMashLight* GetLightData()const = 0;

		//! Enables/disables shadows for this light.
        /*!
            In deferred renderers, all lights may cast shadows.
            In forward renderers, only the main light may cast shadows.
         
            This function should not be called often as performance will
            be affected.
         
            \param enable Enables/Disables shadows for this light.
        */
		virtual void SetShadowsEnabled(bool enable) = 0;
		
        //! Are shadows enabled for this light.
        /*!
            \return Are shadows enabled for this light.
        */
		virtual bool IsShadowsEnabled()const = 0;

        //! Registers this light as a forward rendered light.
        /*!
            Lights must be registered in forward rendered scenes to support
            runtime shader generation.
         
            This should be called at load times only. Runtime use will affect performance.
         
            Note that a light may be regestered as a forward rendered light but still
            be used in deferred scenes.
        */
		virtual void SetLightRendererType(eLIGHT_RENDERER_TYPE rendererType, bool main = true) = 0;

		//! Forward rendered main light
		/*!
			\return True of this is the main forward rendered light.
		*/
		virtual bool IsMainForwardRenderedLight()const = 0;

		//! Forward rendered light
		/*!
			\return True if this is a forward rendered light.
		*/
		virtual bool IsForwardRenderedLight()const = 0;
        
        //! Returns the light renderer type.
        virtual eLIGHT_RENDERER_TYPE GetLightRendererType()const = 0;

		//! Is this light enabled.
        /*!
            \return Is this light on or off.
        */
		virtual bool IsLightEnabled()const = 0;
        
        //! Enables/disables this light.
        /*!
            Lights can be flicked on or off at any time.
            \param enable Enable/disable this light.
        */
		virtual void SetEnableLight(bool enable) = 0;
	};
}

#endif