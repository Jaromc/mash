//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TECHNIQUE_H_
#define _MASH_TECHNIQUE_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashArray.h"
#include "MashString.h"
namespace mash
{
	class MashTechniqueInstance;
	class MashSceneManager;
	class MashEffect;
	class MashVertex;
    struct sEffectMacro;

    /*!
        Techniques store effects, shadow caster effects and render states for rendering.
        They call on various parts of the engine to set effects, render states,
        and auto parameters when needed.
     
        Techniques hold 4 effects, one normal effect for colour and the rest are shadow
        casters for each light type. An effect for each light type is needed to support
        different ways of generating shadows. Shadow caster effects are created automatically
        when using runtime effects and rarely need to be accessed or created by the user.
     
        Technique instances share technique pointers to keep render state changes
        down and reduce memory usages. Technique instances are mainly used for materials
        that use the same effects but just use different textures.
    */
	class MashTechnique : public MashReferenceCounter
	{
	public:            
        /*!
            Callbacks can be used to send custom data to the GPU 
            when effects are set.
        */
		class TechniqueCallback : public MashReferenceCounter
		{
		public:
			TechniqueCallback(){}
			virtual ~TechniqueCallback(){}

            //! Called for each rendered object.
			/*!
				Here you would update things like shader constants.
             
                \param techniqueInstance The instance that owns this.
			*/
			virtual void OnSet(MashTechniqueInstance *techniqueInstance){}

            //! Called on an effect change.
			/*!
				\param techniqueInstance The instance that owns this.
			*/
			virtual void OnUnload(MashTechniqueInstance *techniqueInstance){}
		};
	public:
		MashTechnique():MashReferenceCounter(){}
		virtual ~MashTechnique(){}

        //! Creates an independent copy of this technique.
        /*!
            Creates new effects and other internal components then returns them
            in a new pointer.
         
            This is rarely needed.
         
            \return New technique.
        */
		virtual MashTechnique* CreateIndependentCopy() = 0;
        
        //! Returns the effect used for shadow casters.
        /*!
            If no effect has been initialised then a new one will be created and returned.
         
            This is used internally for loading and shouldn't be accessed directly.
         
            \param lightType 
        */
		virtual MashEffect* InitialiseShadowEffect(eLIGHTTYPE lightType) = 0;
        
        //! Compiles this technique into an API format.
        /*!
            Called from its material owner and shouldn't be accesed directly.
         
            \param fileManager File manager.
            \param sceneManager Scene manager.
            \param compileFlags Bitwise flags of type eMATERIAL_COMPILER_FLAGS.
            \param args Compile arguments.
            \param argCount Argument count.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS CompileTechnique(MashFileManager *fileManager, MashSceneManager *sceneManager, uint32 compileFlags, const sEffectMacro *args, uint32 argCount) = 0;
		
        //! Adds support for a particular lod level.
        /*!
            These values would be 0,1,2,3.....
            The distances are set within the materials.
         
            When a scene object is rendered, it tells the material how far from the camera
            it is. The material the works out what lod level to use based on user
            settings then checks its techniques to find one that supports that lod.
         
            \param lodLevel Adds a lod that this technique will support.
        */
		virtual void AddLodLevelSupport(uint16 lodLevel) = 0;
        
        //! Returns true if this technique supports a particular lod level.
        /*!
            \param lodLevel Lod to check.
            \return True if lod level is supported by this technique. False otherwise.
        */
		virtual bool IsLodLevelSupported(uint16 lodLevel)const = 0;

        //! Gets the render pass this technique should use based on this techniques lighting settings.
        /*!
            \param sceneManager Scene manager.
            \return Render pass.
        */
		virtual eRENDER_PASS GetRenderPass(MashSceneManager *sceneManager) = 0;

        //! Gets the lighting type assigned to this technique.
        /*!
            \return Assigned lighting type.
        */
		virtual eLIGHTING_TYPE GetLightingType()const = 0;
        
        //! Sets the lighting type this technique will use.
        /*!
            This is used during loading to determine what lighting runtime effects
            will use and what render pass this technique will be used in.
         
            This does not force a recompile. That needs to be done manually.
         
            \param type Light type.
        */
		virtual void SetLightingType(eLIGHTING_TYPE type) = 0;
        
        //! Returns true if this technique is transparent.
        /*!
            This value is determined from the blend state. If blending is
            enabled then this will be true.
         
            \return Transparent state.
        */
		virtual bool IsTransparent()const = 0;
        
        //! Returns true if this techniques shadow caster effects are valid.
        /*!
            \return True if this technique contains a valid shadow caster.
        */
		virtual bool ContainsValidShadowCaster()const = 0;
        
        //! Gets a runtime generated unique technique id.
		virtual uint32 GetTechniqueId()const = 0;

        //! Sets a technique callback.
        /*!
            This will be called on particluar events for this technique.
            This function will drop any callback previously set and grab
            a copy of the new callback.
         
            \param callback New callback.
        */
		virtual void SetTechniqueCallback(TechniqueCallback *callback) = 0;
        
        //! Gets the currently set callback.
		virtual TechniqueCallback* GetTechniqueCallback()const = 0;

        //! Gets the vertex declaration the effects for this technique use.
		virtual const MashVertex* GetVertexDeclaration()const = 0;
        
        //! Sets the blend state index.
        /*!
            Blend state can be created from MashVideo.
         
            \param index Blend state index.
        */
		virtual void SetBlendStateIndex(int32 index) = 0;
        
        //! Gets the blend state assigned to this technique.
		virtual int32 GetBlendStateIndex()const = 0;
		
        //! Sets the rasterizer state index.
        /*!
            Rasterizer state can be created from MashVideo.
         
            \param index Rasterizer state index.
         */
		virtual void SetRasteriserStateIndex(int32 index) = 0;
        
        //! Gets the rasterizer state assigned to this technique.
		virtual int32 GetRasterizerStateIndex()const = 0;

        //! Gets the shadow caster effect for a light type.
        /*!
            \return If no effect is set then NULL will be returned.
        */
		virtual MashEffect* GetShadowEffect(eLIGHTTYPE lightType)const = 0;

        //! Gets the active effect.
        /*!
            This maybe a shadow caster effect or the normal colour effect depending on
            the current render pass.
         
            \return Active effect.
        */
		virtual MashEffect* GetActiveEffect()const = 0;
        
        //! Gets the normal colour effect.
		virtual MashEffect* GetEffect()const = 0;
        
        //! Returns true if this technique has compiled without any errors.
        /*!
            \return False if this technique has not been compiled or there were errors during compiling.
        */
		virtual bool IsValid()const = 0;
        
        //! Returns true if compiling has been attempted, regardless of compiling errors.
		virtual bool IsCompiled()const = 0;

		//! Sets the path to an effect file that will be used for light shading.
		/*!
			This allows you to have different shading algorithms for different objects. For example,
			a wooden object can actually look like wood instead of looking like plastic.

			\param effectPath The path to the effect (.eff) file.
		*/
		virtual void SetCustomLightShadingEffect(const MashStringc &effectPath) = 0;

		//! Gets the custom light shading effect path.
		virtual const MashStringc& GetCustomLightShadingEffect()const = 0;

		//! Gets the supported lod list.
        /*!
            \return Supported lod list. This list must not be modified.
        */
		virtual const MashArray<uint16>& GetSupportedLodList()const = 0;
        
        //! Called by materials to set this technique for rendering.
        /*!
            Use MashMaterial::OnSet to call this.
			If this This function returns failed then it simply means this technique is not valid for the current
			pass. It does not mean that an error has occured.

            \param techniqueInstance The instance that called this.
        */
        virtual eMASH_STATUS _OnSet(MashTechniqueInstance *techniqueInstance) = 0;
        
        //! Sets the vertex declaration all effects in this technique will use.
        /*!
            Should not be set manually.
            
            /param vertexDeclaration Vertex declaration all effects in this technique will use.
        */
        virtual void _SetVertexDeclaration(MashVertex *vertexDeclaration) = 0;
	};
}

#endif