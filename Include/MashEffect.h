//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_EFFECT_H_
#define _MASH_EFFECT_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashTypes.h"
#include "MashString.h"

namespace mash
{
	class MashMaterialManager;
	class MashRenderInfo;
	class MashMatrix4;
	class MashVector2;
	class MashVector3;
	class MashVector4;
	class MashTexture;
	class MashTextureState;
	class MashFileManager;
	class MashEffectProgram;
	class MashEffectParamHandle;

    /*!
        An effect can be made up from different GPU programs that when compiled
        can utilise the GPU in different ways.
    */
	class MashEffect : public MashReferenceCounter
	{
	public:
		MashEffect():MashReferenceCounter(){}
		virtual ~MashEffect(){}

		//! Creates an independent copy of this effect.
		/*!
			\return Independent copy of this effect.
		*/
		virtual MashEffect* CreateIndependentCopy() = 0;

        //! Sets int32 data.
        /*!
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
        */
		virtual void SetInt(MashEffectParamHandle *handle, const int32 *data, uint32 count = 1) = 0;
        
        //! Sets bool data.
        /*!
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
        */
		virtual void SetBool(MashEffectParamHandle *handle, const bool *data, uint32 count = 1) = 0;
        
        //! Sets f32 data.
        /*!
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
        */
		virtual void SetFloat(MashEffectParamHandle *handle, const f32 *data, uint32 count = 1) = 0;
        
        //! Sets float2 data.
        /*!
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
        */
		virtual void SetVector2(MashEffectParamHandle *handle, const mash::MashVector2 *data, uint32 count = 1) = 0;
        
        //! Sets float3 data.
        /*!
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
        */
		virtual void SetVector3(MashEffectParamHandle *handle, const MashVector3 *data, uint32 count = 1) = 0;
		
        //! Sets float4 data.
        /*!
            This is one of the methods that provide the fastest way to set GPU data.
         
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
         */
		virtual void SetVector4(MashEffectParamHandle *handle, const MashVector4 *data, uint32 count = 1) = 0;
        
        //! Sets matrix data.
        /*!
            This is one of the methods that provide the fastest way to set GPU data.
         
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
        */
		virtual void SetMatrix(MashEffectParamHandle *handle, const MashMatrix4 *data, uint32 count = 1) = 0;

        //! Sets constant buffer data.
        /*!
            This is one of the methods that provide the fastest way to set GPU data.
         
            \param handle Parameter to set.
            \param data Data to set to the parameter.
            \param count Number of elements in data.
        */
		virtual void SetValue(MashEffectParamHandle *handle, const void *data, uint32 sizeInBytes) = 0;
        
        //! Sets texture data.
        /*!
            \param handle Parameter to set.
            \param texture Texture to set to the parameter.
            \param textureState Parameter texture state.
            \return Status of the function.
        */
        virtual eMASH_STATUS SetTexture(MashEffectParamHandle *handle, MashTexture *texture, const MashTextureState *textureState) = 0;

        //! Fetches a parameter within this effect.
		/*!
            The parameter returned can then have its data set using one of
            the built in Set() functions.
         
            \param name Name of the parameter to fetch.
            \return The named parameter or NULL if the named parameter does not exist within this effect.
        */
		virtual MashEffectParamHandle* GetParameterByName(const int8 *name, ePROGRAM_TYPE type) = 0;

        //! Returns true if this effect compiled ok.
        /*!
            \return True if this effect is valid. False otherise.
        */
		virtual bool IsValid()const = 0;
        
        //! Returns true if this effect has been compiled.
        /*!
            \return True if this effect has been compiled. False otherise.
        */
		virtual bool IsCompiled()const = 0;
        
        //! Adds a program to this effect.
        /*!
			 The filename extention is important if loading from a file as this will determine if
			 the file contains native code or if shader genertion is needed.
			 
			 \param fileName Name of the effect program file.
			 \param entry Entry point.
			 \param profile Profile this effect program should use.
			 \return The effect program.
         */
        virtual MashEffectProgram* AddProgram(const int8 *fileName, const int8 *entry, eSHADER_PROFILE profile) = 0;

		//! Adds a program in native API format to this effect.
		/*!
			No shader generation or file loading will occur.

			\param highLevelCode Native shader code.
			\param entry Entry point.
			\param profile Profile this effect program should use.
			\param fileName Used for debugging only.
			\return The effect program.
		*/
		virtual MashEffectProgram* AddProgramCompiled(const int8 *highLevelCode, const int8 *entry, eSHADER_PROFILE profile, const int8 *fileName = 0) = 0;
        
        //! Returns a program.
        /*!
			\return The program pointer or NULL if a program hasn't been added.
         */
		virtual MashEffectProgram* GetProgramByType(ePROGRAM_TYPE type)const = 0;
        
        //! Compiles this effect.
        /*!
            \param fileManager FileManager.
            \param skinManager SkinManager.
			\param compileArgs Additional arguments.
            \return Function status.
        */
		virtual eMASH_STATUS _Compile(MashFileManager *pFileManager, 
			const MashMaterialManager *pSkinManager,
			const sEffectCompileArgs &compileArgs) = 0;
        
        //! Called once when this effect is set after a different effect.
        /*!
            \param skinManager SkinManager.
            \param renderInfo Render information.
            \return Function status.
        */
		virtual void _OnLoad(MashMaterialManager *skinManager, const MashRenderInfo *renderInfo) = 0;
        
        //! Called each time this effect is set for rendering
        /*!
            Mainly used for updating auto parameters.
         
            \param skinManager SkinManager.
            \param renderInfo Render information.
            \return Function status.
        */
		virtual void _OnUpdate(MashMaterialManager *skinManager, const MashRenderInfo *renderInfo) = 0;

		//! Called when an effect swap occurs.
		/*!
			\param skinManager SkinManager.
            \param renderInfo Render information.
            \return Function status.
		*/
		virtual void _OnUnload(MashMaterialManager *skinManager, const MashRenderInfo *renderInfo) = 0;
	};
}

#endif