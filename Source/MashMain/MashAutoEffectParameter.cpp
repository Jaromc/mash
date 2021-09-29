//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashAutoEffectParameter.h"
#include "MashCamera.h"
#include "MashLight.h"
#include "MashShadowCaster.h"
#include "MashVideo.h"
#include "MashParticleSystem.h"
#include "MashPlane.h"
#include "MashTexture.h"
#include "MashTechniqueInstance.h"
#include "MashEffect.h"

namespace mash
{
	void MashParameterWVP::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		mash::MashMatrix4 mWVP = pRenderInfo->GetWorldTransform();
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
			mWVP = mWVP * pCamera->GetViewProjection();

		pEffect->SetMatrix(pParameter, &mWVP);
	}

	void MashParameterViewProj::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
		{
			pEffect->SetMatrix(pParameter, &pCamera->GetViewProjection());
		}
		else
        {
            mash::MashMatrix4 ident;
			pEffect->SetMatrix(pParameter, &ident);
        }
	}

	void MashParameterWorld::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		pEffect->SetMatrix(pParameter, &pRenderInfo->GetWorldTransform());
	}

	void MashParameterView::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
			pEffect->SetMatrix(pParameter, &pCamera->GetView());
		else
        {
            mash::MashMatrix4 ident;
			pEffect->SetMatrix(pParameter, &ident);
        }
	}

	void MashParameterProj::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
			pEffect->SetMatrix(pParameter, &pCamera->GetProjection());
		else
        {
            mash::MashMatrix4 ident;
			pEffect->SetMatrix(pParameter, &ident);
        }
	}

	void MashParameterWorldInvTrans::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		mash::MashMatrix4 mWorld = pRenderInfo->GetWorldTransform();
		mWorld.Invert();
		mWorld.Transpose();
		pEffect->SetMatrix(pParameter, &mWorld);
	}

	void MashParameterViewInvTrans::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
		{
			mash::MashMatrix4 mView = pCamera->GetView();
			mView.Invert();
			mView.Transpose();
			pEffect->SetMatrix(pParameter, &mView);
		}
		else
        {
            mash::MashMatrix4 ident;
			pEffect->SetMatrix(pParameter, &ident);
        }
	}

	void MashParameterWorldViewInvTrans::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		mash::MashMatrix4 mWorldView = pRenderInfo->GetWorldTransform();

		if (pCamera)
		{
			mWorldView = mWorldView * pCamera->GetView();
			mWorldView.Invert();
			mWorldView.Transpose();
			pEffect->SetMatrix(pParameter, &mWorldView);
		}
		else
		{
			mWorldView.Invert();
			mWorldView.Transpose();
			pEffect->SetMatrix(pParameter, &mWorldView);
		}
	}

	void MashParameterInvViewProj::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
		{
			if (!m_lastData.FastEquals(pCamera->GetViewProjection()))
			{
				m_lastData = pCamera->GetViewProjection();
				m_lastData.Invert();
			}

			pEffect->SetMatrix(pParameter, &m_lastData);
		}
		else
        {
            mash::MashMatrix4 ident;
			pEffect->SetMatrix(pParameter, &ident);
        }
	}

	void MashParameterInvView::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
		{
			if (!m_lastData.FastEquals(pCamera->GetView()))
			{
				m_lastData = pCamera->GetView();
				m_lastData.Invert();
			}

			pEffect->SetMatrix(pParameter, &m_lastData);
		}
		else
        {
            mash::MashMatrix4 ident;
			pEffect->SetMatrix(pParameter, &ident);
        }
	}

	void MashParameterInvProj::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
		{
			if (!m_lastData.FastEquals(pCamera->GetProjection()))
			{
				m_lastData = pCamera->GetProjection();
				m_lastData.Invert();
			}

			pEffect->SetMatrix(pParameter, &m_lastData);
		}
		else
        {
            mash::MashMatrix4 ident;
			pEffect->SetMatrix(pParameter, &ident);
        }
	}

	MashParameterTexture::MashParameterTexture(MashVideo *pRenderer):MashAutoEffectParameter(),m_pDefaultState(0)
	{
		sSamplerState state;
		state.type = aSAMPLER2D;
		state.filter = aFILTER_MIN_MAG_MIP_LINEAR;
		state.uMode = aTEXTURE_ADDRESS_CLAMP;
		state.vMode = aTEXTURE_ADDRESS_CLAMP;
		m_pDefaultState = pRenderer->AddSamplerState(state);
		if (m_pDefaultState)
			m_pDefaultState->Grab();
	}

	MashParameterTexture::~MashParameterTexture()
	{
		if (m_pDefaultState)
			m_pDefaultState->Drop();
	}

	void MashParameterTexture::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashTechniqueInstance *pTechnique = pRenderInfo->GetTechnique();

		const mash::sTexture *pTexture = pTechnique->GetTexture(iIndex);
		if (pTexture)
		{
			if (pTexture->state)
				pEffect->SetTexture(pParameter, pTexture->texture, pTexture->state);
			else
				pEffect->SetTexture(pParameter, pTexture->texture, m_pDefaultState);
		}
	}

	void MashParameterLight::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
        uint32 iSizeInBytes = 0;
		const void *pBuffer = pRenderInfo->GetLightBuffer(&iSizeInBytes);
		pEffect->SetValue(pParameter, pBuffer, iSizeInBytes);
	}

	void MashParameterCameraNearFar::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
		{
			mash::MashVector2 vNearFar(pCamera->GetNear(), pCamera->GetFar());
			pEffect->SetVector2(pParameter, &vNearFar);
		}
		else
        {
            mash::MashVector2 t(0.0f, 1.0f);
			pEffect->SetVector2(pParameter, &t);
        }
	}

	MashParameterShadowMap::MashParameterShadowMap(MashVideo *pRenderer):MashAutoEffectParameter()/*,m_pDefaultState(0)*/
	{
	}

	void MashParameterShadowMap::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashShadowCaster *pCaster = pRenderInfo->GetShadowCaster();
		if (!pCaster)
			return;

		const MashTextureState *textureState = 0;
		MashTexture *pTexture = pCaster->GetShadowMap(&textureState);
		if (!pTexture)
			return;

		pEffect->SetTexture(pParameter, pTexture, textureState);
	}

	MashParameterGBufferDiffuseSampler::MashParameterGBufferDiffuseSampler(MashVideo *pRenderer):MashAutoEffectParameter()
	{
	}

	void MashParameterGBufferDiffuseSampler::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashTexture *pTexture = pRenderInfo->GetSceneDiffuseMap();
		if (!pTexture)
			return;

		pEffect->SetTexture(pParameter, pTexture, pRenderInfo->GetTechnique()->GetTexture(iIndex)->state);
	}

	MashParameterGBufferLightSampler::MashParameterGBufferLightSampler(MashVideo *pRenderer):MashAutoEffectParameter()
	{
	}

	void MashParameterGBufferLightSampler::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashTexture *pTexture = pRenderInfo->GetSceneLightMap();
		if (!pTexture)
			return;

		pEffect->SetTexture(pParameter, pTexture, /*m_pDefaultState*/pRenderInfo->GetTechnique()->GetTexture(iIndex)->state);
	}

	MashParameterGBufferLightSpecualrSampler::MashParameterGBufferLightSpecualrSampler(MashVideo *pRenderer):MashAutoEffectParameter()
	{
	}

	void MashParameterGBufferLightSpecualrSampler::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashTexture *pTexture = pRenderInfo->GetSceneLightSpecMap();
		if (!pTexture)
			return;

		pEffect->SetTexture(pParameter, pTexture, pRenderInfo->GetTechnique()->GetTexture(iIndex)->state);
	}

	MashParameterGBufferSpecualrSampler::MashParameterGBufferSpecualrSampler(MashVideo *pRenderer):MashAutoEffectParameter()
	{
	}

	void MashParameterGBufferSpecualrSampler::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashTexture *pTexture = pRenderInfo->GetSceneSpecularMap();
		if (!pTexture)
			return;

		pEffect->SetTexture(pParameter, pTexture, pRenderInfo->GetTechnique()->GetTexture(iIndex)->state);
	}

	MashParameterGBufferNormalSampler::MashParameterGBufferNormalSampler(MashVideo *pRenderer):MashAutoEffectParameter()
	{
	}

	void MashParameterGBufferNormalSampler::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashTexture *pTexture = pRenderInfo->GetSceneNormalMap();
		if (!pTexture)
			return;

		pEffect->SetTexture(pParameter, pTexture, pRenderInfo->GetTechnique()->GetTexture(iIndex)->state);
	}

	MashParameterGBufferDepthSampler::MashParameterGBufferDepthSampler(MashVideo *pRenderer):MashAutoEffectParameter()
	{
	}

	void MashParameterGBufferDepthSampler::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashTexture *pTexture = pRenderInfo->GetSceneDepthMap();
		if (!pTexture)
			return;

		pEffect->SetTexture(pParameter, pTexture, pRenderInfo->GetTechnique()->GetTexture(iIndex)->state);
	}

	void MashParamBonePaletteArray::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		if (pRenderInfo->GetBonePalette())
			pEffect->SetMatrix(pParameter, pRenderInfo->GetBonePalette(), pRenderInfo->GetCurrentBoneCount());
	}

	void MashParamLightWorldPosition::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		const MashLight *light = pRenderInfo->GetLight();
		if (light)
		{
			mash::MashVector3 lightPos;
			if (light->GetLightType() == aLIGHT_DIRECTIONAL)
			{
				lightPos = light->GetLightData()->direction * -1000000;
			}
			else
			{
				lightPos = light->GetLightData()->position;
			}

			pEffect->SetVector3(pParameter, &lightPos);
		}
	}

	void MashParamWorldView::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		MashCamera *pCamera = pRenderInfo->GetCamera();

		if (pCamera)
		{
			mash::MashMatrix4 worldView = pRenderInfo->GetWorldTransform() * pCamera->GetView();

			if (!(worldView.Equals(m_lastMatrix)))
				pEffect->SetMatrix(pParameter, &worldView);
		}
		else
		{
			pEffect->SetMatrix(pParameter, &pRenderInfo->GetWorldTransform());
		}
	}

	void MashParamWorldPosition::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
        MashVector3 t = pRenderInfo->GetWorldTransform().GetTranslation();
		pEffect->SetVector3(pParameter, &t);
	}

	void MashParamShadowsEnabled::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		bool b = pRenderInfo->GetLight()->IsShadowsEnabled();
		pEffect->SetBool(pParameter, &b);
	}

	void MashParamSoftParticleScale::OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index)
	{
		const MashParticleSystem *particleSystem = renderInfo->GetParticleSystem();
		if (particleSystem)
		{
			const sParticleSettings *settings = particleSystem->GetParticleSettings();
			effect->SetFloat(parameter, &settings->softParticleScale);
		}
	}

	void MashParameterParticleBuffer::OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index)
	{
		const mash::MashParticleSystem *particleSystem = renderInfo->GetParticleSystem();
		if (!particleSystem)
			return;

		const MashVector3 &gravity = particleSystem->GetGravity();
        
		MashVector4 particleBuffer;
		particleBuffer.v[0] = particleSystem->GetParticleSystemTime();
		particleBuffer.v[1] = gravity.x;
		particleBuffer.v[2] = gravity.y;
		particleBuffer.v[3] = gravity.z;

		effect->SetVector4(parameter, &particleBuffer);
	}

	void MashParamGUIAlphaMaskThreshold::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
		pEffect->SetFloat(pParameter, &m_threshold, 1);
	}

	void MashParamGUIBaseColour::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
        mash::MashVector4 temp(m_colour.r, m_colour.g, m_colour.b, m_colour.a);
		pEffect->SetVector4(pParameter, &temp, 1);
	}

	void MashParamGUIFontColour::OnSet(const MashRenderInfo *pRenderInfo, 
			MashEffect *pEffect, 
			MashEffectParamHandle *pParameter,
			uint32 iIndex)
	{
        mash::MashVector4 temp(m_colour.r, m_colour.g, m_colour.b, m_colour.a);
		pEffect->SetVector4(pParameter, &temp, 1);
	}
}