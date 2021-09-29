//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MATERIAL_BUILDER_H_
#define _MASH_MATERIAL_BUILDER_H_

#include "MashArray.h"
#include "MashFunctor.h"

namespace mash
{
	class MashEffect;
	class MashMaterial;
    struct sEffectMacro;
    struct sEffectCompileArgs;

	typedef MashFunctor<MashArray<sEffectMacro> > MashEffectIncludeFunctor;
    /*!
        This class builds runtime and shadow effects.
        This rarely needs to be accessed directly. Normally these methods
        will be accessed via MashMaterialManager or MashSceneManager.
     
        This class can be accessed from MashMaterialManager::GetMaterialBuilder().
    */
	class MashMaterialBuilder : public MashReferenceCounter
	{
	public:
		MashMaterialBuilder():MashReferenceCounter(){}
		virtual ~MashMaterialBuilder(){}
        
        //! Sets custom runtime effect includes.
        /*!
            This allows you to write custom include files to replace the default ones.
            You would need to use this if your using custom shadow casters for new caster
            and receiver code.

            \param type Include file to replace.
            \param includeString Custom code in .eff format. 
			
        */
        virtual void SetCustomRuntimeIncludes(eSHADER_EFFECT_INCLUDES type, const int8 *includeString) = 0;

		//! This callback will be called each time this file is included.
		/*!
			The callback is passed an array for adding macros to compile the effect with.

			\param includeString The effect file name.
			\param includeFunctor Include functor.
		*/
		virtual void SetIncludeCallback(const int8 *includeString, MashEffectIncludeFunctor includeFunctor) = 0;

        //! Loads a material file.
        /*!
            This parses a material script file and returns a list of
            material objects loaded from it.
         
            The returned materials are not compiled.
         
            \param filePath Path to the material script file.
            \param compileArgs Compile args that will be used for all the effects.
            \param materialsOut Loaded list of returned materials.
            \param reloadMaterial Set to true if previously loaded material should be reloaded.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS LoadMaterialFile(const int8 *filePath, 
			const sEffectMacro *compileArgs = 0, 
			uint32 argCount = 0, 
			MashArray<mash::MashMaterial*> *materialsOut = 0,
            bool reloadMaterial = false) = 0;

        //! Builds an effect that contains effect files and links them with runtime data.
        /*!
            This is used internally only and is the workhorse of the material system.
         
            The effect passed in is assumed to contain shaders in the custom
            effect format only. The runtime compatible shaders will be linked
            with runtime data to create a final shader in natice API format.
         
            \param effect Effect to compile.
            \param compileArgs Compile args to use for compilation.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS _BuildRunTimeEffect(MashEffect *effect, 
			const sEffectCompileArgs &compileArgs) = 0;

        //! Can be used before calling _BuildRunTimeEffect if a number of materials are to be compiled.
        /*!
            Used internally and called from the scene manager. This function
            should be accessed from there.
         
            This can improve load times significantly when compiling a number of materials
            at the same time.
         
            _EndBatchMaterialCompile must be called when compiling is done.
        */
		virtual void _BeginBatchMaterialCompile() = 0;
        
        //! Used after calling _BeginBatchMaterialCompile.
        /*!
            Used internally and called from the scene manager.
         
            See _BeginBatchMaterialCompile for details.
        */
		virtual void _EndBatchMaterialCompile() = 0;
        
        //! Builds runtime data for deferred shadres.
        /*!
            This is used internally only.
         
            \return On on success, failed otherwise.
         */
		virtual eMASH_STATUS _RebuildDeferredLightingShaders() = 0;
        
        //! Builds runtime data for forward rendered runtime shaders.
        /*!
            This is used internally only.
         
            \return On on success, failed otherwise.
         */
		virtual eMASH_STATUS _RebuildCommonSceneShaders() = 0;
	};

	class MashDevice;
	_MASH_EXPORT MashMaterialBuilder* CreateMashMaterialBuilder(MashDevice *pDevice);
}

#endif