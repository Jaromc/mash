//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SHADOW_CASTER_H_
#define _MASH_SHADOW_CASTER_H_

#include "MashReferenceCounter.h"
#include "MashRenderInfo.h"

namespace mash
{
	class MashAABB;
	class MashVector2;
	class MashTexture;
	class MashVideo;
	class MashSceneManager;
	class MashEffect;
	class MashLight;
	class MashCamera;
	class MashXMLReader;
	class MashXMLWriter;

	enum eSHADOW_SAMPLES
	{
		eSHADOW_SAMPLES_5 = 5,
		eSHADOW_SAMPLES_15 = 15,
		eSHADOW_SAMPLES_25 = 25,
		eSHADOW_SAMPLES_45 = 45
	};

	/*!
		This class is responsible for setting up shadow caster rendering and can be 
        used to relay shadow receiver information to shaders.

		One shadow caster is shared for all lights of a particular type within the scene manager.
        Built in shadow casters can be created from MashSceneManager.
     
        The two main functions to worry about when creating custom casters are OnInitialise() and
        OnLoad(), these will be explained below.
     
        Users can create their own custom shadow casters for each light and set them using
        MashSceneManager::SetShadowCaster(). You will need to create custom shader
        code for the custom caster. This code will be used to create runtime shadow effects.
        Use MashMaterialBuilder::SetCustomRuntimeIncludes() to set the custom shader code.
        Normally you would do this from OnLoad() which called when a caster is set within the
        scene manager.
        You will want to override the following effects depending on what light types you
        create casters for.
            aEFF_INC_DIRECTIONAL_SHADOW_CASTER_VERTEX
            aEFF_INC_DIRECTIONAL_SHADOW_CASTER_PIXEL
            aEFF_INC_SPOT_SHADOW_CASTER_VERTEX
            aEFF_INC_SPOT_SHADOW_CASTER_PIXEL
            aEFF_INC_POINT_SHADOW_CASTER_VERTEX
            aEFF_INC_POINT_SHADOW_CASTER_PIXEL
     
        Runtime shadow receiver code can be swapped using:
            aEFF_INC_DIRECTIONAL_SHADOW_RECEIVER
            aEFF_INC_SPOT_SHADOW_RECEIVER
            aEFF_INC_POINT_SHADOW_RECEIVER
     
        When creating custom casters, all initialisation should happen within OnInitialise().
        It is called at the end of MashGameLoop::Initialise() on all casters set within the scene
        manager. If a caster is set after MashGameLoop::Initialise() then Initialise() will be
        called when set within the scene manager. Note OnInitialise() will only be called once and
        OnLoad() will NOT be called before OnInitialise().
     
        Shadow map size, biasing, and other options are set from MashSceneManager and past
        on to the active casters via OnTextureResize(), OnBiasChange() etc...

		The following is a rough guide as to how shadows are handled in the renderer:

		for each light in scene
			set shadow caster for light type
			for each pass in shadow caster
				render scene to create shadow map (shadow caster stage)
			final render target += render scene again to create a lit scene using shadow map (shadow receiver stage)
        end for

	*/
	class MashShadowCaster : public MashReferenceCounter
	{
    private:
        bool m_isInitialised;
        bool m_isValid;
	public:
		MashShadowCaster():MashReferenceCounter(), m_isInitialised(false), m_isValid(false){}
		virtual ~MashShadowCaster(){}

        //! Called to write this casters data to file.
        /*!
            \param writer Valid XML writer.
        */
		virtual void Serialise(MashXMLWriter *writer) = 0;
        
        //! Called to read/load this casters data.
        /*!
            \param reader Valid XML reader.
        */
		virtual void Deserialise(MashXMLReader *reader) = 0;
        
        //! Gets this casters type.
        /*!
            User created casters must define their own type from a
            value greater than aSHADOW_CASTER_CUSTOM_RANGE.
        */
		virtual int32 GetShadowCasterType()const = 0;
        
        //! Gets the shadow map texture.
        /*!
            \param textureStateOut Returns the texture state.
            \return Shadow map texture.
        */
		virtual MashTexture* GetShadowMap(MashTextureState const  **textureStateOut = 0, uint32 textureIndex = 0) = 0;
		
		//! Gets the number of passes needed to complete the shadow map.
        /*!
            For example, point lights may need 6 passes. Most only need 1.
         
            \return Number of render passes.
        */
		virtual uint32 GetNumPasses()const = 0;

        //! Called before rendering objects to the shadow map.
		/*!
			This function sets up the shadow map render target and anything else
            needed for this caster.

			See OnPassSetup() to set common data before rending each pass.

			Note, no materials should be set here for rendering. The shadow material for each object are set
			automatically and setting any other material for rendering will cause that material to
			render incorrectly. If for example, a blur pass is needed then set the blur material in OnPassEnd()
			and render it from there.
         
            \param pass Current shadow map render pass.
            \param light Active shadow light.
            \param camera Active light.
            \param sceneAABB This is the bounds of the shadow scene.
            \return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS OnPass(uint32 pass,
			MashLight *light,
			const MashCamera *camera,
			const MashAABB &sceneAABB) = 0;

		//! Called per frame, per light, before OnPass().
		/*!
			This allows you to setup any common data once that will be 
			used for each pass.

			\param light Active shadow light.
            \param camera Active light.
            \param sceneAABB This is the bounds of the shadow scene.
            \return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS OnPassSetup(MashLight *light,
			const MashCamera *camera,
			const MashAABB &sceneAABB) = 0;

		//! Called at the end of scene rendering for each light.
		/*!
			Here you could perform mip map generation.
		*/
		virtual void OnPassEnd() = 0;
        
        //! Gets the light type to which this caster will be used for.
		virtual eLIGHTTYPE GetShadowType()const = 0;
        
        //! Called to initialise the caster.
        /*!
            Called from Initialise().
            Render surfaces or any other resource should be created here.
        */
        virtual eMASH_STATUS OnInitialise() = 0;

		//! Called when this caster is set as an active caster.
		/*!
            From here you can set up any shader code that is needed for shader generation
            for this caster type. Use MashMaterialBuilder::SetCustomRuntimeIncludes() to
            set includes.
         
			If this caster is set within MashGameLoop::Initialise() then calling 
            this function will be delayed until MashShadowCaster::Initialise() 
            is called.
		*/
		virtual void OnLoad() = 0;
        
        //! Called when the caster is removed from the scene manager.
		virtual void OnUnload() = 0;
        
        //! Called to initialise the caster.
        /*!
            This is called at the end of MashGameLoop::Initialise() so we don't build
            resources that aren't being used.
        */
		void Initialise()
        {
			if (m_isInitialised)
				return;

            m_isInitialised = true;
            
            if (OnInitialise() == aMASH_FAILED)
                m_isValid = false;
            else
                m_isValid = true;
        }
        
        //! Returns true if Initialise() has been called.
        bool IsInitialised()const{return m_isInitialised;}
        
        //! Returns true if Initialise() returns ok.
        bool IsValid()const{return m_isValid;}
	};
}

#endif