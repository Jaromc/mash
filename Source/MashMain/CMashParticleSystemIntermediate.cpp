//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashParticleSystemIntermediate.h"
#include "MashTechniqueInstance.h"
#include "MashTechnique.h"

namespace mash
{
	CMashParticleSystemIntermediate::CMashParticleSystemIntermediate(MashSceneNode *parent, 
			MashSceneManager *pSceneManager,
			const MashStringc &sName,
			MashMaterial *material,
			bool isCustomParticleSystem,
			bool isMaterialInstanced):MashParticleSystem(parent, pSceneManager, sName), m_pMaterial(material),
			m_particleEmitter(0), m_isPlaying(false), m_isCustomParticleSystem(isCustomParticleSystem),
			m_isMaterialInstanced(isCustomParticleSystem)
	{
		if (m_pMaterial)
			m_pMaterial->Grab();
        
        m_aabb.min = mash::MashVector3(mash::math::MinFloat(), mash::math::MinFloat(), mash::math::MinFloat());
		m_aabb.max = mash::MashVector3(mash::math::MaxFloat(), mash::math::MaxFloat(), mash::math::MaxFloat());
        
        ParticleSystemCallback *particleCallback = MASH_NEW_COMMON ParticleSystemCallback();
        AddCallback(particleCallback);
        particleCallback->Drop();
	}

	CMashParticleSystemIntermediate::~CMashParticleSystemIntermediate()
	{
		if (m_pMaterial)
		{
			m_pMaterial->Drop();
			m_pMaterial = 0;
		}

		if (m_particleEmitter)
		{
			m_particleEmitter->Drop();
			m_particleEmitter = 0;
		}
	}
    
    const mash::MashAABB& CMashParticleSystemIntermediate::GetTotalWorldBoundingBox()const
	{
		return MashSceneNode::GetTotalBoundingBox();
	}
    
	const mash::MashAABB& CMashParticleSystemIntermediate::GetWorldBoundingBox()const
	{
		return MashSceneNode::GetWorldBoundingBox();
	}

	void CMashParticleSystemIntermediate::PlayEmitter()
	{
		if (m_isPlaying != true)
		{
			m_isPlaying = true;

			f32 dt = 1.0 / 60.0f;
			for(f32 i = 0.0f; i < m_particleSettings.startTime; i+=dt)
				AdvanceSystemByDelta(dt);
		}
	}

	void CMashParticleSystemIntermediate::SetGravity(const mash::MashVector3 &gravity)
	{
		m_particleSettings.gravity = gravity;
	}

	void CMashParticleSystemIntermediate::SetMinStartSize(f32 size)
	{
		m_particleSettings.minStartSize = size;
	}

	void CMashParticleSystemIntermediate::SetMaxStartSize(f32 size)
	{
		m_particleSettings.maxStartSize = size;
	}

	void CMashParticleSystemIntermediate::SetMinEndSize(f32 size)
	{
		m_particleSettings.minEndSize = size;
	}

	void CMashParticleSystemIntermediate::SetMaxEndSize(f32 size)
	{
		m_particleSettings.maxEndSize = size;
	}

	void CMashParticleSystemIntermediate::SetMinStartVelocity(const mash::MashVector3 &velocity)
	{
		m_particleSettings.minVelocity = velocity;
	}

	void CMashParticleSystemIntermediate::SetMaxStartVelocity(const mash::MashVector3 &velocity)
	{
		m_particleSettings.maxVelocity = velocity;
	}

	void CMashParticleSystemIntermediate::SetMaxParticleCount(uint32 count)
	{
		if (m_particleSettings.maxParticleCount != count)
		{
			uint32 oldCount = m_particleSettings.maxParticleCount;
			m_particleSettings.maxParticleCount = count;
			OnMaxParticleCountChange(oldCount, m_particleSettings.maxParticleCount);
		}
	}

	void CMashParticleSystemIntermediate::SetParticleSettings(const sParticleSettings &settings)
	{
		SetMaxParticleCount(settings.maxParticleCount);
		SetParticlesPerSecond(settings.particlesPerSecond);
		SetMinStartColour(settings.minStartColour);
		SetMaxStartColour(settings.maxStartColour);
		SetMinEndColour(settings.minEndColour);
		SetMaxEndColour(settings.maxEndColour);
		SetMinStartSize(settings.minStartSize);
		SetMaxStartSize(settings.maxStartSize);
		SetMinEndSize(settings.minEndSize);
		SetMaxEndSize(settings.maxEndSize);
		SetMinRotateSpeed(settings.minRotateSpeed);
		SetMaxRotateSpeed(settings.maxRotateSpeed);
		SetMinStartVelocity(settings.minVelocity);
		SetMaxStartVelocity(settings.maxVelocity);
		SetGravity(settings.gravity);
		SetMinDuration(settings.minDuration);
		SetMaxDuration(settings.maxDuration);
		SetSoftParticleScale(settings.softParticleScale);
		SetStartTime(settings.startTime);
		SetEmitterVelocityWeight(settings.emitterVelocityWeight);
	}

	const sTexture* CMashParticleSystemIntermediate::GetDiffuseTexture()const
	{
		if (m_pMaterial)
			return m_pMaterial->GetActiveTechnique()->GetTexture(0);

		return 0;
	}

	eMASH_STATUS CMashParticleSystemIntermediate::SetDiffuseTexture(mash::MashTexture *texture, MashTextureState *state)
	{
		if (!m_pMaterial)
			return aMASH_FAILED;

		m_pMaterial->SetTexture(0, texture);
		m_pMaterial->SetTextureState(0, state);

		return aMASH_OK;
	}

	eLIGHTING_TYPE CMashParticleSystemIntermediate::GetParticleLightingType()
	{
		return m_pMaterial->GetActiveTechnique()->GetTechnique()->GetLightingType();
	}
}


