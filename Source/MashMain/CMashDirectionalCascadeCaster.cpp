//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashDirectionalCascadeCaster.h"
#include "MashMaterialManager.h"
#include "MashVideo.h"
#include "MashLog.h"
#include "MashAABB.h"
#include "MashTexture.h"
#include "MashEffect.h"
#include "MashMaterialBuilder.h"
#include "MashSceneManager.h"
#include "MashXMLWriter.h"
#include "MashXMLReader.h"
#include "MashDevice.h"

namespace mash
{
	bool CMashDirectionalCascadeCaster::m_isCasterTypeInitialised = false;

	int8 *CMashDirectionalCascadeCaster::MashParamCamViewToLightVPArray::m_autoName = "autoCascadeCamViewToLightVPArray";
	int8 *CMashDirectionalCascadeCaster::MashParamCamViewToLightVP::m_autoName = "autoCascadeCamViewToLightVP";
	int8 *CMashDirectionalCascadeCaster::MashParamCascadeClipPlanes::m_autoName = "autoCascadeClipPlanes";
	int8 *CMashDirectionalCascadeCaster::MashParamCamViewToLightView::m_autoName = "autoCascadeCamViewToLightView";
	int8 *CMashDirectionalCascadeCaster::MashParamCurrentCascade::m_autoName = "autoCurrentCascade";

	CMashDirectionalCascadeCaster::CMashDirectionalCascadeCaster(MashVideo *renderer, eCASTER_TYPE casterType):MashDirectionalShadowCascadeCaster(),
		m_samples(eSHADOW_SAMPLES_15), m_sampleSize(0.0009f), m_renderTarget(0), m_textureState(0),
		m_cascadeCount(3), m_edgeBlendDistance(0.1f), m_cascadeDivider(0.5f), m_bias(0.005f),
		m_textureSize(512), m_textureFormat(aSHADOW_FORMAT_16), m_renderer(renderer), m_activePass(0),
		m_blendState(0), m_rasterizerState(-1), m_rebuidRenderTarget(false),
		m_camViewToLightVP(0), m_cascadeClipPlanes(0), m_casterType(casterType), m_emsDarkeningFactor(30),
		m_fixedShadowDistanceEnabled(false), m_fixedShadowDistance(1000.0f), m_useBackface(false), m_cascadePartitions(0)
	{
		//TODO : Move this into init
		m_camViewToLightVP = MASH_ALLOC_T_COMMON(MashMatrix4, m_cascadeCount);
		m_cascadeClipPlanes = MASH_ALLOC_T_COMMON(MashVector2, m_cascadeCount);
		m_cascadePartitions = MASH_ALLOC_T_COMMON(f32, m_cascadeCount);
		UpdateCascadePartitions();

		//create default rasterizer state
		UseBackfaceGeometry(m_useBackface);
	}

	CMashDirectionalCascadeCaster::~CMashDirectionalCascadeCaster()
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

		if (m_camViewToLightVP)
			MASH_FREE(m_camViewToLightVP);

		if (m_cascadeClipPlanes)
			MASH_FREE(m_cascadeClipPlanes);

		if (m_cascadePartitions)
			MASH_FREE(m_cascadePartitions);
	}

	void CMashDirectionalCascadeCaster::Serialise(MashXMLWriter *writer)
	{
		writer->WriteAttributeInt("cascadecount", GetCascadeCount());
		writer->WriteAttributeDouble("cascadeblenddistance", GetCascadeEdgeBlendDistance());
		writer->WriteAttributeDouble("cascadedivider", GetCascadeDivider());
		writer->WriteAttributeDouble("bias", GetBias());
		writer->WriteAttributeInt("textureformat", GetTextureFormat());
		writer->WriteAttributeInt("texturesize", GetTextureSize());

		writer->WriteAttributeInt("samples", GetSampleCount());
		writer->WriteAttributeDouble("samplesize", GetSampleSize());
		writer->WriteAttributeInt("usebackface", GetBackfaceRenderingEnabled());
		writer->WriteAttributeInt("fixedshadowdistanceenabled", IsFixedShadowDistanceEnabled());
		writer->WriteAttributeDouble("fixedshadowdistance", GetFixedShadowDistance());
		writer->WriteAttributeDouble("esmdarkening", GetESMDarkeningFactor());
	}

	void CMashDirectionalCascadeCaster::Deserialise(MashXMLReader *reader)
	{
		int intval;
		float floatval;
		bool boolval;
		if (reader->GetAttributeInt("cascadecount", intval))
			SetCascadeCount(intval);
		if (reader->GetAttributeFloat("cascadeblenddistance", floatval))
			SetCascadeEdgeBlendDistance(floatval);
		if (reader->GetAttributeFloat("cascadedivider", floatval))
			SetCascadeDivider(floatval);
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

		reader->GetAttributeFloat("fixedshadowdistance", floatval);
		if (reader->GetAttributeInt("fixedshadowdistanceenabled", intval))
			SetFixedShadowDistance((bool)intval, floatval);
	}

	int32 CMashDirectionalCascadeCaster::GetShadowCasterType()const
	{
		if (m_casterType == aCASTER_TYPE_STANDARD)
			return aSHADOW_CASTER_DIRECTIONAL_CASCADE_STANDARD;
		else if (m_casterType == aCASTER_TYPE_ESM)
			return aSHADOW_CASTER_DIRECTIONAL_CASCADE_ESM;
	}

	void CMashDirectionalCascadeCaster::SetESMDarkeningFactor(f32 darkening)
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

	void CMashDirectionalCascadeCaster::SetCascadeCount(uint32 count)
	{
		if (m_cascadeCount != count)
		{
			m_cascadeCount = count;

			if (m_camViewToLightVP)
				MASH_FREE(m_camViewToLightVP);

			if (m_cascadeClipPlanes)
				MASH_FREE(m_cascadeClipPlanes);

			if (m_cascadePartitions)
				MASH_FREE(m_cascadePartitions);

			m_camViewToLightVP = MASH_ALLOC_T_COMMON(MashMatrix4, m_cascadeCount);
			m_cascadeClipPlanes = MASH_ALLOC_T_COMMON(MashVector2, m_cascadeCount);
			m_cascadePartitions = MASH_ALLOC_T_COMMON(f32, m_cascadeCount);
			UpdateCascadePartitions();

			m_rebuidRenderTarget = true;

			MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
		}
	}

	void CMashDirectionalCascadeCaster::SetCascadeEdgeBlendDistance(f32 dist)
	{
		if (m_edgeBlendDistance != dist)
		{
			m_edgeBlendDistance = dist;
			MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
		}
	}

	void CMashDirectionalCascadeCaster::SetFixedShadowDistance(bool enable, f32 distance)
	{
		m_fixedShadowDistanceEnabled = enable;
		m_fixedShadowDistance = math::Max<float>(1.0, fabs(distance));
	}

	void CMashDirectionalCascadeCaster::SetCascadeDivider(f32 div)
	{
		if (m_cascadeDivider != div)
		{
			m_cascadeDivider = div;
            UpdateCascadePartitions();
		}
	}

	void CMashDirectionalCascadeCaster::SetBias(f32 bias)
	{
		if (m_bias != bias)
		{
			m_bias = bias;
			MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
		}
	}

	void CMashDirectionalCascadeCaster::SetTextureFormat(eSHADOW_MAP_FORMAT format)
	{
		if (m_textureFormat != format)
		{
			m_textureFormat = format;
			m_rebuidRenderTarget = true;
		}
	}

	void CMashDirectionalCascadeCaster::SetTextureSize(int32 shadowMapSize)
	{
		if (m_textureSize != shadowMapSize)
		{
			m_textureSize = shadowMapSize;
			m_rebuidRenderTarget = true;
		}
	}

	void CMashDirectionalCascadeCaster::SetSamples(eSHADOW_SAMPLES samples)
	{
		if (m_samples != samples)
		{
			m_samples = samples;
			MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
		}
	}

	void CMashDirectionalCascadeCaster::SetSampleSize(f32 size)
	{
		if (m_sampleSize != size)
		{
			m_sampleSize = size;
			MashDevice::StaticDevice->GetSceneManager()->OnShadowReceiverMaterialRebuildNeeded(this);
		}
	}

	/*
		From http://www.flipcode.com/archives/Frustum_Culling.shtml
	*/
	void CMashDirectionalCascadeCaster::GetFrustumPoints(const MashCamera *camera,
		f32 near, f32 far, MashVector3 *pointsOut)
	{
		f32 fov = camera->GetFOV();
		f32 aspect = camera->GetAspect();
		MashVector3 wpos = camera->GetRenderTransformState().translation;
		MashVector3 direction = camera->GetTarget() - wpos;
		direction.Normalize();

		// calculate the radius of the frustum sphere
		f32 fViewLen = far - near;
		// use some trig to find the height of the frustum at the far plane
		//f32 fHeight = fViewLen * tan(fov * 0.5f);
		f32 fHalfHeight = far * tan(fov * 0.5f);
		f32 fHalfWidth = fHalfHeight * aspect;
		// halfway point between near/far planes starting at the origin and extending along the z axis
		mash::MashVector3 P(0.0f, 0.0f, near + (fViewLen * 0.5f));
		// the calculate far corner of the frustum
		mash::MashVector3 Q(fHalfWidth, fHalfHeight, fViewLen);
		// the vector between P and Q
		mash::MashVector3 vDiff(P - Q);
		// the radius becomes the length of this vector
		f32 sphereRadius = vDiff.Length();
		// calculate the center of the sphere
		MashVector3 sphereCenter = wpos + (direction * ((fViewLen * 0.5f) + near));

		MashAABB aabb;
		aabb.min = sphereCenter - MashVector3(sphereRadius, sphereRadius, sphereRadius);
		aabb.max = sphereCenter + MashVector3(sphereRadius, sphereRadius, sphereRadius);

		aabb.GetVerticies(pointsOut);
	}

	void CMashDirectionalCascadeCaster::UpdateCascadePartitions()
	{
		float lambda = m_cascadeDivider;
		float nd = 0.0f;
		float fd = 1.0f;
		float ratio = 1.0f;
		m_cascadePartitions[0] = 0.0f;
		for(int32 i = 1; i < m_cascadeCount; ++i)
		{
			float si = i / (float)m_cascadeCount;
			m_cascadePartitions[i] = lambda*(nd*powf(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
		}
	}

	eMASH_STATUS CMashDirectionalCascadeCaster::OnPassSetup(mash::MashLight *light,
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
						"CMashDirectionalCascadeCaster::OnCasterPass");

			 return aMASH_FAILED;
		 }
		
		 mash::MashVector3 worldLightPos = camera->GetRenderTransformState().translation;

		 MashMatrix4 lightView;

		 //create light view matrix
		lightView.CreateCameraLookAt(worldLightPos, 
		worldLightPos + pLightData->direction, 
		mash::MashVector3(0.0f, 1.0f, 0.0f));

		//get inverse of cam view
		mash::MashMatrix4 inverseCameraView = camera->GetView();
		inverseCameraView.Invert();

		//create cam view to light view matrix
		m_camViewToLightView = inverseCameraView * lightView;
		
		mash::MashVector3 sceneAABBLightSpace[8];
		if (m_fixedShadowDistanceEnabled)
		{
			MashAABB fixedAABB(MashVector3(-m_fixedShadowDistance, -m_fixedShadowDistance, -m_fixedShadowDistance) + worldLightPos,
				MashVector3(m_fixedShadowDistance, m_fixedShadowDistance, m_fixedShadowDistance) + worldLightPos);

			fixedAABB.GetVerticies(sceneAABBLightSpace);
		}
		else
		{
			/*
				we need to add a bit extra to the bounds to elminate z fighting for
				meshes such as planes that may lie right on the edge
			*/
			MashAABB newAABB = sceneAABB;
			newAABB.min += MashVector3(-1.0f, -1.0f, -1.0f);
			newAABB.max += MashVector3(1.0f, 1.0f, 1.0f);

			newAABB.GetVerticies(sceneAABBLightSpace);
		}


		//transform world space AABB into light space
		for(uint32 i = 0; i < 8; ++i)
		{
			sceneAABBLightSpace[i] = lightView.TransformVector(sceneAABBLightSpace[i]);
		}

		//compute cascade partitions...TODO : This should be cached
		//create evenly spaced cascades
		const f32 cameraFar = (m_fixedShadowDistanceEnabled)?m_fixedShadowDistance:camera->GetFar();
		const f32 cameraNear = camera->GetNear();
		const f32 cameraNearFar = cameraFar - cameraNear;

		MashMatrix4 cascadeProjection;
		f32 cascadeIntervalBegin, cascadeIntervalEnd;
		MashVector3 frustumPoints[8];
		for(uint32 cascade = 0; cascade < m_cascadeCount; ++cascade)
		{
			cascadeIntervalBegin = m_cascadePartitions[cascade] * cameraNearFar;

			if (cascade == (m_cascadeCount-1))
				cascadeIntervalEnd = 1.0f * cameraNearFar;
			else
				cascadeIntervalEnd = m_cascadePartitions[cascade+1] * cameraNearFar;

			GetFrustumPoints(camera, cascadeIntervalBegin, cascadeIntervalEnd, frustumPoints);

			//calculate ortho min max
			MashVector3 transformedFrustumPoint;
			mash::MashVector3 lightSpaceOrthoMin(mash::math::MaxFloat(), mash::math::MaxFloat(), mash::math::MaxFloat());
			mash::MashVector3 lightSpaceOrthoMax(mash::math::MinFloat(), mash::math::MinFloat(), mash::math::MinFloat());
			for(int32 i = 0; i < 8; ++i)
			{
				//transformedFrustumPoint = inverseCameraView.TransformVector(frustumPoints[i]);
				transformedFrustumPoint = lightView.TransformVector(frustumPoints[i]);
				math::MinVec3(transformedFrustumPoint.v, lightSpaceOrthoMin.v, lightSpaceOrthoMin.v);
				math::MaxVec3(transformedFrustumPoint.v, lightSpaceOrthoMax.v, lightSpaceOrthoMax.v);
			}

			//calculate light space scene near far
			mash::MashVector3 lightSpaceMin = /*lightSpaceOrthoMin;*/mash::MashVector3(mash::math::MaxFloat(), mash::math::MaxFloat(), mash::math::MaxFloat());
			mash::MashVector3 lightSpaceMax =/* lightSpaceOrthoMax;*/mash::MashVector3(mash::math::MinFloat(), mash::math::MinFloat(), mash::math::MinFloat());
			for(int32 i = 0; i < 8; ++i)
			{
				math::MinVec3(sceneAABBLightSpace[i].v, lightSpaceMin.v, lightSpaceMin.v);
				math::MaxVec3(sceneAABBLightSpace[i].v, lightSpaceMax.v, lightSpaceMax.v);
			}

			float orthoNear = lightSpaceMin.z;
			float orthoFar = lightSpaceMax.z;

			cascadeProjection.CreateOrthographicOffCenter(lightSpaceOrthoMin.x,
                                                      lightSpaceOrthoMin.y,
                                                      lightSpaceOrthoMax.x,
                                                      lightSpaceOrthoMax.y,
                                                      orthoNear, orthoFar);

			//from shaderx6 "Stable Rendering of Cascaded Shadow Maps"
			MashVector3 shadowOrigin(0.0f, 0.0f, 0.0f);
			MashMatrix4 shadowMat = lightView * cascadeProjection;
			shadowOrigin = shadowMat.TransformVector(shadowOrigin);

			float texCoordX = shadowOrigin.x * (float)m_textureSize * 0.5f;
			float texCoordY = shadowOrigin.y * (float)m_textureSize * 0.5f;

			float texCoordRoundX = floorf(texCoordX);
			float texCoordRoundY = floorf(texCoordY);

			float dx = texCoordRoundX - texCoordX;
			float dy = texCoordRoundY - texCoordY;

			dx /= (float)m_textureSize * 0.5f;
			dy /= (float)m_textureSize * 0.5f;

			MashMatrix4 roundingMat;
			roundingMat.SetTranslation(MashVector3(dx, dy, 0.0f));

			//TODO : Matrix multiply not needed. Just add transform
			shadowMat *= roundingMat;

			//TODO : After testing use m_camViewToLightView * cascadeProjection;
			m_camViewToLightVP[cascade] = inverseCameraView * shadowMat;//*lightView * cascadeProjection;

			m_cascadeClipPlanes[cascade].x = cascadeIntervalBegin;
			m_cascadeClipPlanes[cascade].y = cascadeIntervalEnd;
		}

		m_renderer->SetBlendState(m_blendState);
		m_renderer->SetRasteriserState(m_rasterizerState);

		if (m_renderer->SetRenderTarget(GetSceneRenderTarget()) == aMASH_FAILED)
		 {
			  MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to set render target",
						"CMashDirectionalCascadeCaster::OnCasterPass");

			 return aMASH_FAILED;
		 }

		m_renderer->ClearTarget(mash::aCLEAR_TARGET | mash::aCLEAR_DEPTH, mash::sMashColour4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

		return aMASH_OK;
	}

	eMASH_STATUS CMashDirectionalCascadeCaster::OnPass(uint32 iPass, 
			mash::MashLight *pLight,
			//const mash::MashVector3 &vCameraPosition,
			const mash::MashCamera *pCamera,
			const mash::MashAABB &sceneAABB)
	{
		m_activePass = iPass;
		/*
			Cascaded shadow maps are laid out in a texture atlis. The dimentions are
				x = (shadowMapSize * numCascades) y = (shadowMapSize)
			Therefore we use the y value here because it holds the original size
			before being multiplied by the cascade count.
		*/
		sMashViewPort splitViewport;
		splitViewport.width = m_textureSize;
		splitViewport.height = m_textureSize;
		splitViewport.x = iPass * m_textureSize;
		splitViewport.y = 0;
		splitViewport.minZ = 0.0f;
		splitViewport.maxZ = 1.0f;

		m_renderer->SetViewport(splitViewport);

		return aMASH_OK;
	}

	MashTexture* CMashDirectionalCascadeCaster::GetShadowMap(MashTextureState const **pTextureStateOut, uint32 textureIndex)
	{
		if (!m_renderTarget)
			return 0;

		*pTextureStateOut = m_textureState;
		return m_renderTarget->GetTexture(0);
	}

	eMASH_STATUS CMashDirectionalCascadeCaster::OnRebuildRenderTarget()
	{
		if (m_renderTarget)
		{
			m_renderTarget->Drop();
			m_renderTarget = 0;
		}

		eFORMAT shadowFormat = aFORMAT_R32_FLOAT;
		if (m_textureFormat == aSHADOW_FORMAT_16)
			shadowFormat = aFORMAT_R16_FLOAT;

		int32 textureX = m_textureSize * m_cascadeCount;
		int32 textureY = m_textureSize;

		m_renderTarget = m_renderer->CreateRenderSurface(textureX, textureY, &shadowFormat, 1, false, aDEPTH_OPTION_OWN_DEPTH, aFORMAT_DEPTH32_FLOAT);

		if (!m_renderTarget)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to create render target",
						"CMashDirectionalCascadeCaster::OnRebuildRenderTarget");

			return aMASH_FAILED;
		}

		m_rebuidRenderTarget = false;

		return aMASH_OK;
	}

	void CMashDirectionalCascadeCaster::OnPixelReceiverBuild(MashArray<sEffectMacro> &macros)
	{
		if (m_casterType == aCASTER_TYPE_ESM)
		{
            macros.PushBack(sEffectMacro("MASH_USE_ESM", MashStringc::CreateFrom(1)));
            macros.PushBack(sEffectMacro("MASH_DARKENING", MashStringc::CreateFrom(m_emsDarkeningFactor)));
		}

        macros.PushBack(sEffectMacro("MASH_SPLIT_COUNT", MashStringc::CreateFrom(m_cascadeCount)));
        macros.PushBack(sEffectMacro("MASH_INV_SPLIT", MashStringc::CreateFrom(1.0f / (float)m_cascadeCount)));
        macros.PushBack(sEffectMacro("MASH_BLEND_DIST", MashStringc::CreateFrom(m_edgeBlendDistance)));
        macros.PushBack(sEffectMacro("MASH_SHADOW_BIAS", MashStringc::CreateFrom(m_bias)));
        macros.PushBack(sEffectMacro("MASH_FILTER_SIZE", MashStringc::CreateFrom(m_sampleSize)));
		
        macros.PushBack(sEffectMacro("MASH_SAMPLES", MashStringc::CreateFrom(m_samples)));
	}

	void CMashDirectionalCascadeCaster::OnLoad()
	{
		const int8 *casterVertexEffectPath = "MashCasterShadowCasterVertex.eff";
		const int8 *casterPixelEffectPath = "MashCascadeShadowCasterPixel.eff";
		const int8 *receiverPixelEffectPath = "MashCascadeShadowReceiver.eff";

		MashMaterialBuilder *materialBuilder = m_renderer->GetMaterialManager()->GetMaterialBuilder();
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_DIRECTIONAL_SHADOW_CASTER_VERTEX, casterVertexEffectPath);
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_DIRECTIONAL_SHADOW_CASTER_PIXEL, casterPixelEffectPath);
		materialBuilder->SetCustomRuntimeIncludes(aEFF_INC_DIRECTIONAL_SHADOW_RECEIVER, receiverPixelEffectPath);

		materialBuilder->SetIncludeCallback(receiverPixelEffectPath, MashEffectIncludeFunctor(&CMashDirectionalCascadeCaster::OnPixelReceiverBuild, this));
	}

	void CMashDirectionalCascadeCaster::UseBackfaceGeometry(bool enable)
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

	eMASH_STATUS CMashDirectionalCascadeCaster::OnInitialise()
	{
		//only done once for all objects
		if (!m_isCasterTypeInitialised)
		{
			MashAutoEffectParameter *param = MASH_NEW_COMMON MashParamCamViewToLightVPArray();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			param = MASH_NEW_COMMON MashParamCamViewToLightVP();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			param = MASH_NEW_COMMON MashParamCascadeClipPlanes();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			param = MASH_NEW_COMMON MashParamCamViewToLightView();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			param = MASH_NEW_COMMON MashParamCurrentCascade();
			m_renderer->GetMaterialManager()->RegisterAutoParameterHandler(param);
			param->Drop();

			m_isCasterTypeInitialised = true;
		}

		if (OnRebuildRenderTarget() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
						"Failed to create render target",
						"CMashDirectionalCascadeCaster::OnInitialise");

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
}
