//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashPointShadowCaster.h"
#include "MashVideo.h"
#include "MashLog.h"
#include "MashMaterialManager.h"
#include "MashMaterialBuilder.h"
#include "MashCamera.h"
#include "MashLight.h"
#include "MashAABB.h"
#include "MashTexture.h"
#include "MashEffect.h"
#include "MashXMLWriter.h"
#include "MashXMLReader.h"
#include "MashSceneManager.h"
#include "MashDevice.h"

namespace mash
{
	bool CMashPointShadowCaster::m_isCasterTypeInitialised = false;

	int8 *CMashPointShadowCaster::ParamLightViewProjection::m_autoName = "autoPointLightVP";
	int8 *CMashPointShadowCaster::ParamLightPosRange::m_autoName = "autoPointLightPositionRange";

	CMashPointShadowCaster::CMashPointShadowCaster(MashVideo *renderer, eCASTER_TYPE casterType):MashPointShadowCaster(),
		m_renderTarget(0), m_textureState(0), 
		m_bias(0.005f), m_emsDarkeningFactor(30),
		m_textureSize(512), m_textureFormat(aSHADOW_FORMAT_16), m_renderer(renderer),
		m_blendState(0), m_rasterizerState(-1), m_rebuidRenderTarget(false), m_casterType(casterType),
		m_useBackface(false)
	{
		UseBackfaceGeometry(m_useBackface);
	}

	CMashPointShadowCaster::~CMashPointShadowCaster()
	{
		if (m_renderTarget)
		{
			m_renderTarget->Drop();
			m_renderTarget = 0;
		}

		if (m_textureState)
		{
			m_textureState->Drop();
			m_textureState = 0;
		}
	}

	void CMashPointShadowCaster::Serialise(MashXMLWriter *writer)
	{
		writer->WriteAttributeDouble("bias", GetBias());
		writer->WriteAttributeInt("textureformat", GetTextureFormat());
		writer->WriteAttributeInt("texturesize", GetTextureSize());

		writer->WriteAttributeInt("usebackface", GetBackfaceRenderingEnabled());
		writer->WriteAttributeDouble("esmdarkening", GetESMDarkeningFactor());
	}

	void CMashPointShadowCaster::Deserialise(MashXMLReader *reader)
	{
		int intval;
		float floatval;
		bool boolval;
		if (reader->GetAttributeFloat("bias", floatval))
			SetBias(floatval);
		if (reader->GetAttributeInt("textureformat", intval))
			SetTextureFormat((eSHADOW_MAP_FORMAT)intval);
		if (reader->GetAttributeInt("texturesize", intval))
			SetTextureSize(intval);
		if (reader->GetAttributeInt("usebackface", intval))
			UseBackfaceGeometry((bool)intval);
		if (reader->GetAttributeFloat("esmdarkening", floatval))
			SetESMDarkeningFactor(floatval);
	}

	int32 CMashPointShadowCaster::GetShadowCasterType()const
	{
		if (m_casterType == aCASTER_TYPE_STANDARD)
			return aSHADOW_CASTER_POINT_STANDARD;
		else if (m_casterType == aCASTER_TYPE_STANDARD_FILTERED)
			return aSHADOW_CASTER_POINT_STANDARD_FILTERED;
		else if (m_casterType == aCASTER_TYPE_ESM)
			return aSHADOW_CASTER_POINT_ESM;
	}

	void CMashPointShadowCaster::SetBias(f32 bias)
	{
		if (m_bias != bias)
		{
			m_bias = bias;
			MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
		}
	}

	void CMashPointShadowCaster::SetTextureFormat(eSHADOW_MAP_FORMAT format)
	{
		if (m_textureFormat != format)
		{
			m_textureFormat = format;
			m_rebuidRenderTarget = true;
		}
	}

	void CMashPointShadowCaster::SetTextureSize(int32 shadowMapSize)
	{
		if (m_textureSize != shadowMapSize)
		{
			m_textureSize = shadowMapSize;
			m_rebuidRenderTarget = true;
		}
	}

	void CMashPointShadowCaster::SetESMDarkeningFactor(f32 darkening)
	{
		if (darkening != m_emsDarkeningFactor)
		{
			m_emsDarkeningFactor = darkening;

			switch(m_casterType)
			{
			case aCASTER_TYPE_STANDARD:
			case aCASTER_TYPE_STANDARD_FILTERED:
				break;//do nothing
			case aCASTER_TYPE_ESM:
				MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
				break;
			};
		}
	}

	MashTexture* CMashPointShadowCaster::GetShadowMap(MashTextureState const **pTextureStateOut, uint32 textureIndex)
	{
		*pTextureStateOut = m_textureState;
		return m_renderTarget->GetTexture(textureIndex);
	}

	eMASH_STATUS CMashPointShadowCaster::OnRebuildRenderTarget()
	{
		if (m_renderTarget)
		{
			m_renderTarget->Drop();
			m_renderTarget = 0;
		}

		eFORMAT shadowFormat = aFORMAT_R32_FLOAT;
		if (m_textureFormat == aSHADOW_FORMAT_16)
			shadowFormat = aFORMAT_R16_FLOAT;

		m_renderTarget = m_renderer->CreateCubicRenderSurface(m_textureSize, false, shadowFormat, true, aFORMAT_DEPTH32_FLOAT);

		if (!m_renderTarget)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to create render target",
						"CMashPointShadowCaster::OnRebuildRenderTarget");

			return aMASH_FAILED;
		}

		m_rebuidRenderTarget = false;

		return aMASH_OK;
	}

	void CMashPointShadowCaster::OnPixelReceiverBuild(MashArray<sEffectMacro> &macros)
	{
		if (m_casterType == aCASTER_TYPE_ESM)
		{
            macros.PushBack(sEffectMacro("MASH_USE_ESM", MashStringc::CreateFrom(1)));
            macros.PushBack(sEffectMacro("MASH_DARKENING", MashStringc::CreateFrom(m_emsDarkeningFactor)));
            macros.PushBack(sEffectMacro("MASH_TEX_SIZE", MashStringc::CreateFrom((float)m_textureSize)));
            macros.PushBack(sEffectMacro("MASH_INV_T_SIZE", MashStringc::CreateFrom(1.0f / (float)m_textureSize)));
		}
		else if (m_casterType == aCASTER_TYPE_STANDARD_FILTERED)
		{
            macros.PushBack(sEffectMacro("MASH_USE_FILTER", MashStringc::CreateFrom(1)));
            macros.PushBack(sEffectMacro("MASH_TEX_SIZE", MashStringc::CreateFrom((float)m_textureSize)));
            macros.PushBack(sEffectMacro("MASH_INV_T_SIZE", MashStringc::CreateFrom(1.0f / (float)m_textureSize)));
		}

        macros.PushBack(sEffectMacro("MASH_SHADOW_BIAS", MashStringc::CreateFrom(m_bias)));
	}

	void CMashPointShadowCaster::OnLoad()
	{
		const int8 *casterVertexEffectPath = "MashPointShadowCasterVertex.eff";
		const int8 *casterPixelEffectPath = "MashPointShadowCasterPixel.eff";
		const int8 *receiverPixelEffectPath = "MashPointShadowReceiver.eff";

		MashMaterialBuilder *materialBuilder = m_renderer->GetMaterialManager()->GetMaterialBuilder();
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_POINT_SHADOW_CASTER_VERTEX, casterVertexEffectPath);
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_POINT_SHADOW_CASTER_PIXEL, casterPixelEffectPath);
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_POINT_SHADOW_RECEIVER, receiverPixelEffectPath);

		materialBuilder->SetIncludeCallback(receiverPixelEffectPath, MashEffectIncludeFunctor(&CMashPointShadowCaster::OnPixelReceiverBuild, this));
	}

	void CMashPointShadowCaster::UseBackfaceGeometry(bool enable)
	{
		if ((m_rasterizerState == -1) || (enable != m_useBackface))
		{
			//create default rasterizer state
			sRasteriserStates rasterizerState;
			
			rasterizerState.fillMode = aFILL_SOLID;
			rasterizerState.depthBias = 0.0f;
			rasterizerState.depthBiasClamp = 0.0f;
			rasterizerState.slopeScaledDepthBias = 0.0f;
			//rasterizerState.multisampleEnable = true;
			rasterizerState.depthTestingEnable = true;
			rasterizerState.depthWritingEnabled = true;
			rasterizerState.depthComparison = aZCMP_LESS_EQUAL;

			if (enable)
				rasterizerState.cullMode = aCULL_CW;
			else
				rasterizerState.cullMode = aCULL_CCW;
			
			m_rasterizerState = m_renderer->AddRasteriserState(rasterizerState);

			m_useBackface = enable;
		}
	}

	eMASH_STATUS CMashPointShadowCaster::OnInitialise()
	{
		if (!m_isCasterTypeInitialised)
		{
			MashAutoEffectParameter *param = MASH_NEW_COMMON ParamLightViewProjection();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			param = MASH_NEW_COMMON ParamLightPosRange();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			m_isCasterTypeInitialised = true;
		}

		if (OnRebuildRenderTarget() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to create render target",
						"CMashPointShadowCaster::OnInitialise");

			return aMASH_FAILED;
		}

		sSamplerState state;
		state.type = aSAMPLER2D;
		//must use point filtering in ogl. dx Seems to handle linear fine.
		state.filter = aFILTER_MIN_MAG_LINEAR;
		state.uMode = aTEXTURE_ADDRESS_CLAMP;
		state.vMode = aTEXTURE_ADDRESS_CLAMP;
		m_textureState = m_renderer->AddSamplerState(state);
		m_textureState->Grab();

		sBlendStates blendState;
		blendState.blendingEnabled = false;
		blendState.srcBlend = aBLEND_ONE;
		blendState.destBlend = aBLEND_ZERO;
		blendState.blendOp = aBLENDOP_ADD;
		blendState.srcBlendAlpha = aBLEND_ONE;
		blendState.destBlendAlpha = aBLEND_ZERO;
		blendState.blendOpAlpha = aBLENDOP_ADD;
		blendState.colourWriteMask = aCOLOUR_WRITE_RED;//only one channel needed
		m_blendState = m_renderer->AddBlendState(blendState);

		return aMASH_OK;
	}

	eMASH_STATUS CMashPointShadowCaster::OnPassSetup(mash::MashLight *light,
			const mash::MashCamera *camera,
			const mash::MashAABB &sceneAABB)
	{
		if (m_rebuidRenderTarget)
		{
			OnRebuildRenderTarget();
			m_rebuidRenderTarget = false;
		}

		const sMashLight *pLightData = light->GetLightData();

		if (!pLightData)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"No light data set",
					"CMashPointShadowCaster::OnCasterPass");

			return aMASH_FAILED;
		}

		m_worldPosAndRange = MashVector4(pLightData->position.x, pLightData->position.y, pLightData->position.z, 1.0f / pLightData->range);

		m_lightProjection.CreatePerspectiveFOV(mash::math::Pi() / 2.0f, 
			1.0f, camera->GetNear(), pLightData->range );

		m_renderer->SetBlendState(m_blendState);
		m_renderer->SetRasteriserState(m_rasterizerState);

		return aMASH_OK;
	}

	eMASH_STATUS CMashPointShadowCaster::OnPass(uint32 iPass, 
			mash::MashLight *light,
			const mash::MashCamera *pCamera,
			const mash::MashAABB &sceneAABB)
	{
		MashMatrix4 lightView;
		const sMashLight *pLightData = light->GetLightData();

		switch ((eCUBEMAP_FACE)iPass)
		{
		case aCUBEMAP_FACE_NEG_X:
			{
				mash::MashVector3 vLook(-1.0f, 0.0f, 0.0f);
				mash::MashVector3 vUp(0.0f, 1.0f, 0.0f);
				lightView.CreateCameraLookAt(pLightData->position,
					pLightData->position + vLook,
					vUp);

				break;
			}
		case aCUBEMAP_FACE_NEG_Y:
			{
				mash::MashVector3 vLook(0.0f, -1.0f, 0.0f);
				mash::MashVector3 vUp(0.0f, 0.0f, 1.0f);
				lightView.CreateCameraLookAt(pLightData->position,
					pLightData->position + vLook,
					vUp);

				break;
			}
		case aCUBEMAP_FACE_NEG_Z:
			{
				mash::MashVector3 vLook(0.0f, 0.0f, -1.0f);
				mash::MashVector3 vUp(0.0f, 1.0f, 0.0f);
				lightView.CreateCameraLookAt(pLightData->position,
                                             pLightData->position + vLook,
                                             vUp);

				break;
			}
		case aCUBEMAP_FACE_POS_X:
			{
				mash::MashVector3 vLook(1.0f, 0.0f, 0.0f);
				mash::MashVector3 vUp(0.0f, 1.0f, 0.0f);
				lightView.CreateCameraLookAt(pLightData->position,
                                             pLightData->position + vLook,
                                             vUp);
				break;
			}
		case aCUBEMAP_FACE_POS_Y:
			{
				mash::MashVector3 vLook(0.0f, 1.0f, 0.0f);
				mash::MashVector3 vUp(0.0f, 0.0f, -1.0f);
				lightView.CreateCameraLookAt(pLightData->position,
                                             pLightData->position + vLook,
                                             vUp);
				break;
			}
		case aCUBEMAP_FACE_POS_Z:
			{
				mash::MashVector3 vLook(0.0f, 0.0f, 1.0f);
				mash::MashVector3 vUp(0.0f, 1.0f, 0.0f);
				lightView.CreateCameraLookAt(pLightData->position,
                                             pLightData->position + vLook,
                                             vUp);
				break;
			}
		};

		m_lightViewProjection = lightView * m_lightProjection;

		if (m_renderer->SetRenderTarget(m_renderTarget, iPass) == aMASH_FAILED)
		 {
			  MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to set render target",
						"CMashPointShadowCaster::OnCasterPass");

			 return aMASH_FAILED;
		 }

		m_renderer->ClearTarget(mash::aCLEAR_TARGET | mash::aCLEAR_DEPTH, mash::sMashColour4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

		return aMASH_OK;
	}
}
