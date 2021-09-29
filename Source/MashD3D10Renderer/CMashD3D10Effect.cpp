//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10Effect.h"
#include "CMashD3D10Texture.h"
#include "CMashD3D10TextureState.h"
#include "CMashD3D10CubeTexture.h"
#include "CMashD3D10Renderer.h"
#include "MashHelper.h"
#include "MashMaterialManager.h"

#include "MashMatrix4.h"
#include "MashVector2.h"
#include "MashVector3.h"
#include "MashVector4.h"
#include "MashTexture.h"
#include "MashFileManager.h"
#include "MashEffectProgram.h"

#include "MashLog.h"
namespace mash
{
	CMashD3D10Effect::CMashD3D10Effect(CMashD3D10Renderer *renderer):MashEffect(),
			m_renderer(renderer), m_bIsValid(false), m_bIsCompiled(false), m_isAPIEffect(false)
	{
		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
			m_programs[i] = 0;
	}

	CMashD3D10Effect::~CMashD3D10Effect()
	{
		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
		{
			if (m_programs[i])
			{
				m_programs[i]->Drop();
				m_programs[i] = 0;
			}
		}
	}

	MashEffect* CMashD3D10Effect::CreateIndependentCopy()
	{
		CMashD3D10Effect *newEffect = (CMashD3D10Effect*)m_renderer->GetMaterialManager()->CreateEffect();
		newEffect->m_isAPIEffect = m_isAPIEffect;

		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
		{
			if (m_programs[i])
			{
				/*
					If copying from a runtime effect then we dont want to pass in the compiled code, else
					it will be treated as an api effect, and wont recompile with new lighting changes
				*/
				MashEffectProgram *program = 0;
				if (m_isAPIEffect)
					program = newEffect->AddProgramCompiled(m_programs[i]->GetHighLevelSource().GetCString(), m_programs[i]->GetEntry().GetCString(), m_programs[i]->GetProfile(), m_programs[i]->GetFileName().GetCString());
				else
					program = newEffect->AddProgram(m_programs[i]->GetFileName().GetCString(), m_programs[i]->GetEntry().GetCString(), m_programs[i]->GetProfile());

				program->SetCompileArguments(m_programs[i]->GetCompileArguments());
			}
		}

		return newEffect;
	}

	eMASH_STATUS CMashD3D10Effect::_Compile(MashFileManager *pFileManager, 
			const MashMaterialManager *pSkinManager,
			const sEffectCompileArgs &compileArgs)
	{
		m_bIsValid = true;
		m_bIsCompiled = true;

		if (!m_isAPIEffect)
		{
			if (((MashMaterialManager*)pSkinManager)->BuildRunTimeEffect(this, compileArgs) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to compile effect into native format.", 
					"CMashD3D10Effect::_Compile");

				return aMASH_FAILED;
			}
		}

		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
		{
			if (m_programs[i])
			{
				bool isValid = false;
				if (m_programs[i]->_Compile(pFileManager, pSkinManager, compileArgs.macros, compileArgs.macroCount, isValid) == aMASH_FAILED)
				{
					m_bIsCompiled = false;
					m_bIsValid = false;
				}

				if (!isValid)
					m_bIsValid = false;
			}
		}

		if (!m_bIsCompiled)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to compile effect.", 
				"CMashD3D10Effect::_Compile");

			return aMASH_FAILED;
		}

		if (!m_bIsValid)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
				"Effect is not valid on the current system.", 
				"CMashD3D10Effect::_Compile");

			return aMASH_OK;
		}
		
		return aMASH_OK;
	}

	MashEffectParamHandle* CMashD3D10Effect::GetParameterByName(const int8 *sName, ePROGRAM_TYPE type)
	{
		if (m_programs[type])
			return m_programs[type]->GetParameterByName(sName);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
				"CMashD3D10Effect::GetParameterByName",
				"An effect program of type '%i' does not exist. Parameter being searched for was '%s'.", type, sName);

		return 0;
	}

	MashEffectProgram* CMashD3D10Effect::AddProgramCompiled(const int8 *highLevelCode, const int8 *entry, eSHADER_PROFILE profile, const int8 *fileName)
	{
		if (!highLevelCode)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid high level code pointer was given. The program was not added.", 
				"CMashD3D10Effect::AddProgramCompiled");

			return 0;
		}

		ePROGRAM_TYPE programType = helpers::GetEffectProgramTypeFromProfile(profile);

		if (programType == aPROGRAM_UNKNOWN)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Unkown profile type used. The program was not added.", 
				"CMashD3D10Effect::AddProgramCompiled");

			return 0;
		}

		m_isAPIEffect = true;

		CMashD3D10EffectProgram *program = MASH_NEW_COMMON CMashD3D10EffectProgram(m_renderer->GetD3D10Device(),
			programType,
			highLevelCode,
			fileName,
			entry,
			profile);

		if (m_programs[programType])
			MASH_DELETE m_programs[programType];

		m_programs[programType] = program;

		return program;

	}

	MashEffectProgram* CMashD3D10Effect::AddProgram(/*const int8 *highLevelCode, */const int8 *fileName, const int8 *entry, eSHADER_PROFILE profile)
	{
		if (!fileName/* && !highLevelCode*/)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid effect file name pointer was given. The program was not added.", 
				"CMashD3D10Effect::AddProgram");

			return 0;
		}

		ePROGRAM_TYPE programType = helpers::GetEffectProgramTypeFromProfile(profile);

		if (programType == aPROGRAM_UNKNOWN)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Unkown profile type used. The program was not added.", 
				"CMashD3D10Effect::AddProgram");

			return 0;
		}

		m_isAPIEffect = helpers::IsFileANativeEffectProgram(fileName);

		CMashD3D10EffectProgram *program = MASH_NEW_COMMON CMashD3D10EffectProgram(m_renderer->GetD3D10Device(),
			programType,
			0,
			fileName,
			entry,
			profile);

		if (m_programs[programType])
			MASH_DELETE m_programs[programType];

		m_programs[programType] = program;

		return program;
	}

	void CMashD3D10Effect::SetMatrix(MashEffectParamHandle *pHandle, const mash::MashMatrix4 *data, uint32 count)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;
		han->OnSet(data, sizeof(float) * count * 16);
	}

	void CMashD3D10Effect::SetInt(MashEffectParamHandle *pHandle, const int32 *data, uint32 count)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;

		han->OnSet(data, sizeof(int32) * count);
	}

	void CMashD3D10Effect::SetBool(MashEffectParamHandle *pHandle, const bool *data, uint32 count)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;

		/*
			bools in HLSL are 4 bytes in size
		*/
		//very temp fix
		int32 t = 1;
		if (!*data)
			t = 0;

		han->OnSet(&t, 4);
	}

	void CMashD3D10Effect::SetFloat(MashEffectParamHandle *pHandle, const float *data, uint32 count)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;

		han->OnSet(data, sizeof(float) * count);
	}

	void CMashD3D10Effect::SetVector2(MashEffectParamHandle *pHandle, const mash::MashVector2 *data, uint32 count)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;

		if (count == 1)
			han->OnSet(data, sizeof(float) * count * 2);
		else
		{
			han->OnSetScalarArray(data, sizeof(float) * 2, count);
		}
	}

	void CMashD3D10Effect::SetVector3(MashEffectParamHandle *pHandle, const mash::MashVector3 *data, uint32 count)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;

		if (count == 1)
			han->OnSet(data, sizeof(float) * count * 3);
		else
		{
			han->OnSetScalarArray(data, sizeof(float) * 3, count);
		}
	}

	void CMashD3D10Effect::SetVector4(MashEffectParamHandle *pHandle, const mash::MashVector4 *data, uint32 count)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;
		han->OnSet(data, sizeof(float) * count * 4);
	}

	void CMashD3D10Effect::SetValue(MashEffectParamHandle *pHandle, const void *data, uint32 iSizeInBytes)
	{
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;
		han->OnSet(data, iSizeInBytes);
	}

	void CMashD3D10Effect::_OnUnload(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo)
	{
		m_renderer->ResetUsedShaderResources();
	}

	void CMashD3D10Effect::_OnLoad(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo)
	{
		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
		{
			if (m_programs[i])
				m_programs[i]->_OnLoad(pSkinManager, pRenderInfo);
		}
	}

	void CMashD3D10Effect::_OnUpdate(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo)
	{
		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
		{
			if (m_programs[i])
				m_programs[i]->_OnUpdate(this, pSkinManager, pRenderInfo);
		}
	}

	eMASH_STATUS CMashD3D10Effect::SetTexture(MashEffectParamHandle *pHandle, MashTexture *pTexture, const MashTextureState *pTextureState)
	{
		if (!pTexture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING, 
				"No texture pointer given.", 
				"CMashD3D10Effect::SetTexture");

			return aMASH_OK;
		}

		ID3D10Device *d3D10Device = m_renderer->GetD3D10Device();
		CMashD3D10EffectParamHandle *han = (CMashD3D10EffectParamHandle*)pHandle;
		ID3D10ShaderResourceView *D3DResource = 0;
		const eRESOURCE_TYPE resType = pTexture->GetType();
		if (resType == eRESOURCE_TYPE::aRESOURCE_TEXTURE)
			D3DResource = ((CMashD3D10Texture*)pTexture)->GetD3D10ResourceView();
		else if (resType == eRESOURCE_TYPE::aRESOURCE_CUBE_TEXTURE)
			D3DResource = ((CMashD3D10CubeTexture*)pTexture)->GetD3D10ResourceView();
		else
		{
			//should never happen
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid texture resource.", 
				"CMashD3D10Effect::SetTexture");

			return aMASH_FAILED;
		}

		if (!D3DResource)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid D3D resource pointer for texture resource.", 
				"CMashD3D10Effect::SetTexture");

			return aMASH_FAILED;
		}

		if (han->GetOwnerType() == aPROGRAM_PIXEL)
		{
			 m_renderer->SetD3D10PixelShaderResourceView(han->m_startOffset, D3DResource);

			if (pTextureState)
			{
				m_renderer->SetD3D10PixelShaderSampler(han->m_samplerSlotIndex, pTextureState);
			}
		}
		else if (han->GetOwnerType() == aPROGRAM_VERTEX)
		{
			m_renderer->SetD3D10VertexShaderResourceView(han->m_startOffset, D3DResource);

			if (pTextureState)
			{
				m_renderer->SetD3D10VertexShaderSampler(han->m_samplerSlotIndex, pTextureState);
			}
		}

		return aMASH_OK;
	}
}