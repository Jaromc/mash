//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_EFFECT_H_
#define _C_MASH_D3D10_EFFECT_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashEffect.h"
#include "CMashD3D10EffectProgram.h"
#include <d3d10_1.h>
namespace mash
{
	class CMashD3D10Renderer;
	class CMashD3D10Effect : public MashEffect
	{
	private:
		CMashD3D10Renderer *m_renderer;
		CMashD3D10EffectProgram *m_programs[aPROGRAM_UNKNOWN];
		bool m_bIsValid;
		bool m_bIsCompiled;
		bool m_isAPIEffect;
	public:
		CMashD3D10Effect(CMashD3D10Renderer *renderer);

		~CMashD3D10Effect();

		MashEffect* CreateIndependentCopy();

		void SetMatrix(MashEffectParamHandle *pHandle, const mash::MashMatrix4 *data, uint32 count = 1);
		void SetInt(MashEffectParamHandle *pHandle, const int32 *data, uint32 count = 1);
		void SetBool(MashEffectParamHandle *pHandle, const bool *data, uint32 count = 1);
		void SetFloat(MashEffectParamHandle *pHandle, const float *data, uint32 count = 1);
		void SetVector2(MashEffectParamHandle *pHandle, const mash::MashVector2 *data, uint32 count = 1);
		void SetVector3(MashEffectParamHandle *pHandle, const mash::MashVector3 *data, uint32 count = 1);

		//provides the fastest way to set shader variables
		void SetVector4(MashEffectParamHandle *pHandle, const mash::MashVector4 *data, uint32 count = 1);
		void SetValue(MashEffectParamHandle *pHandle, const void *pData, uint32 iSizeInBytes);

		MashEffectParamHandle* GetParameterByName(const int8 *sName, ePROGRAM_TYPE type);

		bool IsValid()const;
		bool IsCompiled()const;
		eMASH_STATUS _Compile(MashFileManager *pFileManager, 
			const MashMaterialManager *pSkinManager,
			const sEffectCompileArgs &compileArgs);

		void _OnUnload(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo);
		void _OnLoad(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo);
		void _OnUpdate(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo);

		MashEffectProgram* AddProgramCompiled(const int8 *highLevelCode, const int8 *entry, eSHADER_PROFILE profile, const int8 *fileName = 0);
		MashEffectProgram* AddProgram(/*const int8 *highLevelCode, */const int8 *sFileName, const int8 *entry, eSHADER_PROFILE profile);
		MashEffectProgram* GetProgramByType(ePROGRAM_TYPE type)const;

		eMASH_STATUS SetTexture(MashEffectParamHandle *pHandle, MashTexture *pTexture, const MashTextureState *pTextureState);
	};

	inline MashEffectProgram* CMashD3D10Effect::GetProgramByType(ePROGRAM_TYPE type)const
	{
		return m_programs[type];
	}

	inline bool CMashD3D10Effect::IsValid()const
	{
		return m_bIsValid;
	}

	inline bool CMashD3D10Effect::IsCompiled()const
	{
		return m_bIsCompiled;
	}
}

#endif

#endif