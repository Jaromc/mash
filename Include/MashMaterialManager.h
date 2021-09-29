//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MATERIAL_MANAGER_H_
#define _MASH_MATERIAL_MANAGER_H_

#include "MashReferenceCounter.h"
#include "MashCreationParameters.h"
#include "MashTypes.h"
#include "MashEnum.h"
#include "MashList.h"

namespace mash
{
	class MashSceneManager;
    class MashMaterialBuilder;
	class MashMaterial;
	class MashTechnique;
	class MashTechniqueInstance;
	class MashEffectParamHandle;
	class MashEffect;
	class MashAutoEffectParameter;

    /*!
        Handles material and GPU data. This can be accessed from MashVideo::GetMaterialManager().
     
        Runtime effects are built when needed through this class.
     
        Materials are used to describe how objects look when rendered and
        send data from the CPU to GPU when needed. 
    */
	class MashMaterialManager : public MashReferenceCounter
	{
	public:
        //! Built in materials.
		enum eSTANDARD_MATERIAL
		{
			aSTANDARD_MATERIAL_DEFAULT_MESH,
			aSTANDARD_MATERIAL_DECAL_STANDARD,
			aSTANDARD_MATERIAL_DECAL_SKINNED,
			aSTANDARD_MATERIAL_PARTICLE_CPU,
			aSTANDARD_MATERIAL_PARTICLE_GPU,
			aSTANDARD_MATERIAL_PARTICLE_CPU_SOFT,
			aSTANDARD_MATERIAL_PARTICLE_GPU_SOFT,
			aSTANDARD_MATERIAL_PARTICLE_MESH,
			aSTANDARD_MATERIAL_DRAW_TEXTURE,
            aSTANDARD_MATERIAL_DRAW_TRANS_TEXTURE,
			aSTANDARD_MATERIAL_PRIMITIVE,
			aSTANDARD_MATERIAL_GUI_LINE,
			aSTANDARD_MATERIAL_GUI_SPRITE,
			aSTANDARD_MATERIAL_GUI_FONT,
			aSTANDARD_MATERIAL_GBUFFER_DIR_LIGHT,
			aSTANDARD_MATERIAL_GBUFFER_SPOT_LIGHT,
			aSTANDARD_MATERIAL_GBUFFER_POINT_LIGHT,
			aSTANDARD_MATERIAL_GBUFFER_COMBINE,
			aSTANDARD_MATERIAL_GBUFFER_CLEAR,
			aSTANDARD_MATERIAL_COUNT
		};
	public:
		//! Constructor.
		MashMaterialManager():MashReferenceCounter(){}

		//! Destructor.
		virtual ~MashMaterialManager(){}

        //! Registers an auto parameter handler
        /*!
            You would use this to handle your own GPU uniforms that will be updated via this.
         
            \param autoParamHandler Custom parameter handler to add. This will grab a copy of the handler.
            \param overWrite Set to true if you want this handler to replace any handlers found with the same name.
        */
		virtual void RegisterAutoParameterHandler(MashAutoEffectParameter *autoParamHandler, bool overWrite = false) = 0;
        
        //! Returns true if an auto paramter with a given name is currently handled.
        /*!
            \param name Auto paramter name to check.
            \param autoParamHandlerIndex Returns the handle to this param within the manager.
            \return True if the auto paramter exists, false otherwise.
        */
		virtual bool IsAutoParameter(const int8 *name, uint32 &autoParamHandlerIndex)const = 0;
        
        //! Returns an auto parameter.
        /*!
            \param name Auto paramters name.
            \return Auto paramter if found. NULL otherwise.
        */
		virtual MashAutoEffectParameter* GetAutoParameterByName(const int8 *name)const = 0;

        //! Gets a built in material.
        /*!
            This loads the material if it has not already been loaded. The returned pointer must
            not be dropped.
         
            \param materialType Material to return.
            \param wasLoaded Returns true if this material was just loaded. False if it had been previously loaded.
            \return Built in material.
        */
		virtual MashMaterial* GetStandardMaterial(eSTANDARD_MATERIAL materialType, bool *wasLoaded = 0) = 0;

        //! Creates a new empty effect.
        /*!
            The returned effect must be dropped when you are done with it.
         
            \return New effect.
        */
		virtual MashEffect* CreateEffect() = 0;

        //! Gets all the materials currently loaded. This list should not be modified.
		virtual const MashList<MashMaterial*>& GetMaterialList()const = 0;

        //! Adds a new material to the manager.
        /*!
            The returned pointer must not be dropped.
            
            \param name Material name. This must be unqiue. Use NULL to generate a unique name.
            \param vertexElements Vertex elements that will create the vertex declaration for this material.
            \param elementCount Number of elements in the vertex array.
            \return New material.
        */
		virtual MashMaterial* AddMaterial(const int8 *name, const sMashVertexElement *vertexElements, uint32 elementCount) = 0;
		
        //! Finds a material in the manager.
        /*!
            \param name Material to search for.
            \return NULL if a material with the given name was not found.
        */
        virtual MashMaterial* FindMaterial(const int8 *name)const = 0;
        
        //! Gets a material from the manager.
        /*!
            Helper function. Calls FindMaterial() then LoadMaterialFile() if the material was not found.
            
            \param materialName Material name to search for.
            \param filePath If the material was not found then this file will be loaded.
            \param compileArgs Compile arguments.
            \param argCount Number of arguments in the argument list.
            \param wasLoaded Returns true if the material was just loaded. False if it was already loaded.
			\param reload If the material has already been loaded, setting this to true will reload the material from file reflecting any changes made.
            \return Loaded material. NULL if there were any errors.
        */
		virtual MashMaterial* GetMaterial(const int8 *materialName, const int8 *filePath, const sEffectMacro *compileArgs, uint32 argCount, bool *wasLoaded = 0, bool reload = false) = 0;
        
        //! Loads a material file.
        /*!
            A material file may contain many materials. They can be accessed from FindMaterial().
         
            After calling this and reloadMaterial is true, materials currently loaded with the
            same name as those found in the material file will be reloaded with new data.
            Reloaded material pointers will remain valid but internal techniques will now
            be invalid.
            Any new materials within the material file will be loaded as usual.
         
            \param filePath Path of the material script file.
            \param compileArgs Compile arguments.
            \param argCount Number of arguments in the argument list.
            \param materialsOut Optional parameter to return loaded materials.
            \param reloadMaterial Set to true to reload any previous materials loaded.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS LoadMaterialFile(const int8 *filePath, 
                                             const sEffectMacro *compileArgs = 0, 
                                             uint32 argCount = 0, 
                                             MashArray<MashMaterial*> *materialsOut = 0,
                                             bool reloadMaterial = false) = 0;
        
        //! Removes a material from the manager.
        /*!
            Removes and drops the managers copy. If the material has been grabbed elsewhere then
            the material may still be alive after calling this.
         
            \param material Material to remove from the manager.
        */
		virtual void RemoveMaterial(MashMaterial *material) = 0;
        
        //! Removes all materials from the manager.
        /*!
            Removes and drops the managers copy. If the materials have been grabbed elsewhere then
            the materials may still be alive after calling this.
        */
		virtual void RemoveAllMaterials() = 0;

        //! Returns true if the shader profile is valid on the current system and renderer.
        /*!
            \param profile Profile to test.
            \return True if it is valid. False otherwise.
        */
		virtual bool IsProfileSupported(eSHADER_PROFILE profile)const = 0;
        
        //! Gets the best vertex profile that is supported on the current system and renderer.
		virtual eSHADER_PROFILE GetLatestVertexProfile()const = 0;
        
        //! Gets the best fragment profile that is supported on the current system and renderer.
		virtual eSHADER_PROFILE GetLatestFragmentProfile()const = 0;
        
        //! Gets the best geometry profile that is supported on the current system and renderer.
		virtual eSHADER_PROFILE GetLatestGeometryProfile()const = 0;
        
        virtual MashMaterialBuilder* GetMaterialBuilder()const = 0;

        //! Builds an effect that contains runtime data.
        /*!
            This effect will be built based on current scene settings such as lighting.
            The returned effect is not compiled.
         
            This would rarely need to be accessed by the user.
         
            \param effect Effect to build.
            \param compileArgs Compile arguments.
        */
		virtual eMASH_STATUS BuildRunTimeEffect(MashEffect *effect, 
			const sEffectCompileArgs &compileArgs) = 0;
        
        //! Gets the debug compiled effect directory.
		virtual const MashStringc& GetCompiledEffectOutputDirectory()const = 0;
        
        //! Gets the debug intermediate effect directory.
		virtual const MashStringc& GetIntermediateEffectOutputDirectory()const = 0;

		 //! Sets the debug compiled effect directory.
		virtual void SetCompiledEffectOutputDirectory(const MashStringc &dir) = 0;
        
        //! Sets the debug intermediate effect directory.
		virtual void SetIntermediateEffectOutputDirectory(const MashStringc &dir) = 0;

        //! Rebuilds the deferred lighting shaders. This is called on lighting changed.
		virtual eMASH_STATUS _RebuildDeferredLightingShaders() = 0;
        
        //! Rebuilds common runtime shaders. This is called on lighting changes.
		virtual eMASH_STATUS _RebuildCommonSceneShaders() = 0;
        
        //! Compiles all materials based on flags.
        /*!
            \param sceneManager Scene manager.
            \param compileFlags Bitwise eMATERIAL_COMPILER_FLAGS.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS _CompileAllMaterials(MashSceneManager *sceneManager, uint32 compileFlags) = 0;

        //! Called before compiling many materials. This speeds material compiling.
        /*!
			Do not call this directly, instad use MashSceneManager::CompileAllMaterials().
			Internal methods must call _EndBatchMaterialCompile() after calling this.
        */
		virtual void _BeginBatchMaterialCompile() = 0;
        
        //! Called after _BeginBatchMaterialCompile().
		virtual void _EndBatchMaterialCompile() = 0;

        //! Creates a new empty technique.
		virtual MashTechnique* _CreateTechnique() = 0;
        
        //! Creates a new technique instance.
        /*!
            \param refTechnique Technique to create an instance from. If this is NULL then
                a new MashTechnique is created an set to the instance.
            \return New technique instance.
        */
		virtual MashTechniqueInstance* _CreateTechniqueInstance(MashTechnique *refTechnique) = 0;
        
        //! Called by effects to set their parameters.
        /*!
            This will call on the auto parameters to send data to the GPU.
         
            \param effect Calling effect.
            \param parameter Calling paramter whose data to send.
            \param parameterHandler This handler is found using IsAutoParameter().
            \param index If this paramter is a texture than this is the texture index used in MashTechniqeInstance.
        */
		virtual void _SetProgramAutoParameter(MashEffect *effect, MashEffectParamHandle *parameter, uint32 parameterHandler, uint32 index = 0) = 0;

        //! Creates a new material.
        /*!
            \param name New material name. Set to NULL to generate a unique name.
            \param reference Reference material to instance from.
            \return New material.
        */
		virtual MashMaterial* _CreateMaterial(const int8 *name, MashMaterial *reference) = 0;

        //! Gets the current API shader header.
		/*!
			Used during effect creation. GLSL shaders need to have a
			"#version xxx" at the top of the shader.
		*/
		virtual const int8* _GetAPIShaderHeader(){return 0;}
        
        //! Called internally to initialise this manager.
        /*!
            \param creationParameters Creation parameters.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS _Initialise(const sMashDeviceSettings &creationParameters) = 0;
	};
}

#endif