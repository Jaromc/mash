//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashSpotShadowCaster.h"
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
	bool CMashSpotShadowCaster::m_isCasterTypeInitialised = false;

	int8 *CMashSpotShadowCaster::ParamCamViewToLightView::m_autoName = "autoSpotCamViewToLightView";
	int8 *CMashSpotShadowCaster::ParamSpotProjection::m_autoName = "autoSpotLightProjection";
	int8 *CMashSpotShadowCaster::ParamSpotLightRange::m_autoName = "autoSpotLightInvRange";

	CMashSpotShadowCaster::CMashSpotShadowCaster(MashVideo *renderer, eCASTER_TYPE casterType):MashSpotShadowCaster(),
		m_samples(eSHADOW_SAMPLES_15), m_sampleSize(0.0009f), m_renderTarget(0), m_textureState(0), 
		m_bias(0.005f),
		m_textureSize(512), m_textureFormat(aSHADOW_FORMAT_16), m_renderer(renderer), m_activePass(0),
		m_blendState(0), m_rasterizerState(-1), m_rebuidRenderTarget(false), m_casterType(casterType),
		m_emsDarkeningFactor(30), m_currentLightInvRange(),
		m_useBackface(false)
	{
		UseBackfaceGeometry(m_useBackface);
	}

	CMashSpotShadowCaster::~CMashSpotShadowCaster()
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

	void CMashSpotShadowCaster::Serialise(MashXMLWriter *writer)
	{
		writer->WriteAttributeDouble("bias", GetBias());
		writer->WriteAttributeInt("textureformat", GetTextureFormat());
		writer->WriteAttributeInt("texturesize", GetTextureSize());

		writer->WriteAttributeInt("samples", GetSampleCount());
		writer->WriteAttributeDouble("samplesize", GetSampleSize());
		writer->WriteAttributeInt("usebackface", GetBackfaceRenderingEnabled());
		writer->WriteAttributeDouble("esmdarkening", GetESMDarkeningFactor());
	}

	void CMashSpotShadowCaster::Deserialise(MashXMLReader *reader)
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
		if (reader->GetAttributeInt("samples", intval))
			SetSamples((eSHADOW_SAMPLES)intval);
		if (reader->GetAttributeFloat("samplesize", floatval))
			SetSampleSize(floatval);
		if (reader->GetAttributeInt("usebackface", intval))
			UseBackfaceGeometry((bool)intval);
		if (reader->GetAttributeFloat("esmdarkening", floatval))
			SetESMDarkeningFactor(floatval);
	}

	int32 CMashSpotShadowCaster::GetShadowCasterType()const
	{
		if (m_casterType == aCASTER_TYPE_STANDARD)
			return aSHADOW_CASTER_SPOT_STANDARD;
		else if (m_casterType == aCASTER_TYPE_ESM)
			return aSHADOW_CASTER_SPOT_ESM;
	}

	void CMashSpotShadowCaster::SetBias(f32 bias)
	{
		if (m_bias != bias)
		{
			m_bias = bias;
			MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
		}
	}

	void CMashSpotShadowCaster::SetTextureFormat(eSHADOW_MAP_FORMAT format)
	{
		if (m_textureFormat != format)
		{
			m_textureFormat = format;
			m_rebuidRenderTarget = true;
		}
	}

	void CMashSpotShadowCaster::SetTextureSize(int32 shadowMapSize)
	{
		if (m_textureSize != shadowMapSize)
		{
			m_textureSize = shadowMapSize;
			m_rebuidRenderTarget = true;
		}
	}

	void CMashSpotShadowCaster::SetSamples(eSHADOW_SAMPLES samples)
	{
		if (m_samples != samples)
		{
			m_samples = samples;
			
			switch(m_casterType)
			{
			case aCASTER_TYPE_STANDARD:
				MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
				break;
			case aCASTER_TYPE_ESM:
				MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
				break;
			};
		}
	}

	void CMashSpotShadowCaster::SetSampleSize(f32 size)
	{
		if (m_sampleSize != size)
		{
			m_sampleSize = size;

			switch(m_casterType)
			{
			case aCASTER_TYPE_STANDARD:
				MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
				break;
			case aCASTER_TYPE_ESM:
				MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
				break;
			};
		}
	}

	MashTexture* CMashSpotShadowCaster::GetShadowMap(MashTextureState const **pTextureStateOut, uint32 textureIndex)
	{
		*pTextureStateOut = m_textureState;

		return m_renderTarget->GetTexture(0);
	}

	void CMashSpotShadowCaster::SetESMDarkeningFactor(f32 darkening)
	{
		if (darkening != m_emsDarkeningFactor)
		{
			m_emsDarkeningFactor = darkening;

			switch(m_casterType)
			{
			case aCASTER_TYPE_STANDARD:
				break;//do nothing
			case aCASTER_TYPE_ESM:
				MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
				break;
			};
		}
	}

	eMASH_STATUS CMashSpotShadowCaster::RebuildRenderTarget()
	{
		if (m_renderTarget)
		{
			m_renderTarget->Drop();
			m_renderTarget = 0;
		}

		eFORMAT shadowFormat = aFORMAT_R32_FLOAT;
		if (m_textureFormat == aSHADOW_FORMAT_16)
			shadowFormat = aFORMAT_R16_FLOAT;

		int32 textureX = m_textureSize;
		int32 textureY = m_textureSize;

		m_renderTarget = m_renderer->CreateRenderSurface(textureX, textureY, &shadowFormat, 1, false, aDEPTH_OPTION_OWN_DEPTH, aFORMAT_DEPTH32_FLOAT);

		if (!m_renderTarget)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to create render target",
						"CMashSpotShadowCaster::OnRebuildRenderTarget");

			return aMASH_FAILED;
		}

		m_rebuidRenderTarget = false;

		return aMASH_OK;
	}

	void CMashSpotShadowCaster::OnPixelReceiverBuild(MashArray<sEffectMacro> &macros)
	{
        macros.PushBack(sEffectMacro("MASH_SAMPLES", MashStringc::CreateFrom(m_samples)));

		switch(m_casterType)
		{
		case aCASTER_TYPE_STANDARD:
			{
                macros.PushBack(sEffectMacro("MASH_SHADOW_BIAS", MashStringc::CreateFrom(m_bias)));
                macros.PushBack(sEffectMacro("MASH_FILTER_SIZE", MashStringc::CreateFrom(m_sampleSize)));
				break;
			}
		case aCASTER_TYPE_ESM:
			{
                macros.PushBack(sEffectMacro("MASH_USE_ESM", MashStringc::CreateFrom(1)));
                macros.PushBack(sEffectMacro("MASH_SHADOW_BIAS", MashStringc::CreateFrom(m_bias)));
                macros.PushBack(sEffectMacro("MASH_DARKENING", MashStringc::CreateFrom(m_emsDarkeningFactor)));
                macros.PushBack(sEffectMacro("MASH_SHADOW_BIAS", MashStringc::CreateFrom(m_bias)));
                macros.PushBack(sEffectMacro("MASH_FILTER_SIZE", MashStringc::CreateFrom(m_sampleSize)));
				break;
			}
		}
	}

	void CMashSpotShadowCaster::OnLoad()
	{
		const int8 *casterVertexEffectPath = "MashSpotShadowCasterVertex.eff";
		const int8 *casterPixelEffectPath = "MashSpotShadowCasterPixel.eff";
		const int8 *receiverPixelEffectPath = "MashSpotShadowReceiver.eff";

		MashMaterialBuilder *materialBuilder = m_renderer->GetMaterialManager()->GetMaterialBuilder();
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_SPOT_SHADOW_CASTER_VERTEX, casterVertexEffectPath);
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_SPOT_SHADOW_CASTER_PIXEL, casterPixelEffectPath);
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_SPOT_SHADOW_RECEIVER, receiverPixelEffectPath);

		materialBuilder->SetIncludeCallback(receiverPixelEffectPath, MashEffectIncludeFunctor(&CMashSpotShadowCaster::OnPixelReceiverBuild, this));
	}

	void CMashSpotShadowCaster::UseBackfaceGeometry(bool enable)
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

	eMASH_STATUS CMashSpotShadowCaster::OnInitialise()
	{
		//only done once for all objects
		if (!m_isCasterTypeInitialised)
		{
			MashAutoEffectParameter *param = MASH_NEW_COMMON ParamCamViewToLightView();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			param = MASH_NEW_COMMON ParamSpotProjection();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			param = MASH_NEW_COMMON ParamSpotLightRange();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			m_isCasterTypeInitialised = true;
		}

		if (RebuildRenderTarget() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to create render target",
						"CMashSpotShadowCaster::OnInitialise");

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

	eMASH_STATUS CMashSpotShadowCaster::OnPassSetup(mash::MashLight *light,
			const mash::MashCamera *camera,
			const mash::MashAABB &sceneAABB)
	{
		if (m_rebuidRenderTarget)
		{
			RebuildRenderTarget();
			m_rebuidRenderTarget = false;
		}

		const sMashLight *pLightData = light->GetLightData();

		if (!pLightData)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"No light data set",
					"CMashSpotShadowCaster::OnCasterPass");

			return aMASH_FAILED;
		}

		//get inverse of cam view
		mash::MashMatrix4 inverseCameraView = camera->GetView();
		inverseCameraView.Invert();

		MashMatrix4 lightView;
		lightView.CreateCameraLookAt(pLightData->position, 
			pLightData->position + pLightData->direction, 
			MashVector3(0.0f, 1.0f, 0.0f));

		m_camViewToLightView = inverseCameraView * lightView;

		m_lightProjection.CreatePerspectiveFOV(acos(pLightData->outerCone) * 2.0f, 
			1.0f, camera->GetNear(), pLightData->range );

		m_currentLightInvRange.x = 1.0f / pLightData->range;

		m_worldPosition = pLightData->position;

		m_renderer->SetBlendState(m_blendState);
		m_renderer->SetRasteriserState(m_rasterizerState);

		if (m_renderer->SetRenderTarget(m_renderTarget) == aMASH_FAILED)
		 {
			  MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to set render target",
						"CMashSpotShadowCaster::OnCasterPass");

			 return aMASH_FAILED;
		 }

		m_renderer->ClearTarget(mash::aCLEAR_TARGET | mash::aCLEAR_DEPTH, mash::sMashColour4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSpotShadowCaster::OnPass(uint32 iPass, 
			mash::MashLight *pLight,
			const mash::MashCamera *pCamera,
			const mash::MashAABB &sceneAABB)
	{
		return aMASH_OK;
	}
}
