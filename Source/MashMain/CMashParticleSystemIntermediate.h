//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PARTICLE_SYSTEM_INTERMEDIATE_H_
#define _C_MASH_PARTICLE_SYSTEM_INTERMEDIATE_H_

#include "MashParticleSystem.h"
#include "MashParticleEmitter.h"
#include "MashMaterial.h"
#include "MashSceneNodeCallback.h"

namespace mash
{
	class CMashParticleSystemIntermediate : public MashParticleSystem
	{
    private:
        class ParticleSystemCallback : public MashSceneNodeCallback
        {
        public:
            ParticleSystemCallback(){}
            ~ParticleSystemCallback(){}
            
            void OnNodeUpdate(MashSceneNode *sceneNode, f32 dt)
            {
                ((CMashParticleSystemIntermediate*)sceneNode)->AdvanceSystemByDelta(dt);
            }
        };
	protected:
		CMashParticleSystemIntermediate(MashSceneNode *parent, 
			MashSceneManager *pSceneManager,
			const MashStringc &sName,
			MashMaterial *material,
			bool isCustomParticleSystem,
			bool isMaterialInstanced);

		virtual ~CMashParticleSystemIntermediate();

		sParticleSettings m_particleSettings;
		MashMaterial *m_pMaterial;
		MashParticleEmitter *m_particleEmitter;
		bool m_isPlaying;
		bool m_isCustomParticleSystem;
		bool m_isMaterialInstanced;
        MashAABB m_aabb;

		virtual void OnMaxParticleCountChange(uint32 oldCount, uint32 newCount) = 0;
		virtual void AdvanceSystemByDelta(f32 dt) = 0;
	public:

		MashParticleEmitter* GetCurrentEmitter()const;

		void PlayEmitter();
		void StopEmitter();
		bool IsPlaying()const;

		bool IsCustomParticleSystem()const;
		bool IsParticleMaterialInstanced()const;

		void SetParticleSettings(const sParticleSettings &settings);
		void SetMaxParticleCount(uint32 count);
		void SetParticlesPerSecond(uint32 count);
		void SetMinStartColour(const sMashColour4 &colour);
		void SetMaxStartColour(const sMashColour4 &colour);
		void SetMinEndColour(const sMashColour4 &colour);
		void SetMaxEndColour(const sMashColour4 &colour);
		void SetMinStartSize(f32 size);
		void SetMaxStartSize(f32 size);
		void SetMinEndSize(f32 size);
		void SetMaxEndSize(f32 size);
		void SetMinRotateSpeed(f32 speed);
		void SetMaxRotateSpeed(f32 speed);
		void SetMinStartVelocity(const mash::MashVector3 &velocity);
		void SetMaxStartVelocity(const mash::MashVector3 &velocity);
		void SetGravity(const mash::MashVector3 &gravity);
		void SetMinDuration(f32 duration);
		void SetMaxDuration(f32 duration);
		void SetSoftParticleScale(f32 scale);
		void SetStartTime(uint32 time);
		void SetEmitterVelocityWeight(f32 weight);

		uint32 GetMaxParticleCount()const;
		uint32 GetParticlesPerSecond()const;
		const sMashColour4& GetMinStartColour()const;
		const sMashColour4& GetMaxStartColour()const;
		const sMashColour4& GetMinEndColour()const;
		const sMashColour4& GetMaxEndColour()const;
		f32 GetMinStartSize()const;
		f32 GetMaxStartSize()const;
		f32 GetMinEndSize()const;
		f32 GetMaxEndSize()const;
		f32 GetMinRotateSpeed()const;
		f32 GetMaxRotateSpeed()const;
		const mash::MashVector3& GetMinStartVelocity()const;
		const mash::MashVector3& GetMaxStartVelocity()const;
		const mash::MashVector3& GetGravity()const;
		f32 GetMinDuration()const;
		f32 GetMaxDuration()const;
		f32 GetSoftParticleScale()const;
		uint32 GetStartTime()const;
		f32 GetEmitterVelocityWeight()const;

		eLIGHTING_TYPE GetParticleLightingType();
		const sParticleSettings* GetParticleSettings()const;

		const sTexture* GetDiffuseTexture()const;
		eMASH_STATUS SetDiffuseTexture(mash::MashTexture *texture, MashTextureState *state = 0);
        
        const mash::MashAABB& GetTotalWorldBoundingBox()const;
		const mash::MashAABB& GetWorldBoundingBox()const;
		const mash::MashAABB& GetLocalBoundingBox()const;

		void SetLocalBoundingBox(const MashAABB &bounds);
	};

	inline void CMashParticleSystemIntermediate::SetLocalBoundingBox(const MashAABB &bounds)
	{
		m_aabb = bounds;
	}
    
    inline const mash::MashAABB& CMashParticleSystemIntermediate::GetLocalBoundingBox()const
	{
		return m_aabb;
	}

	inline bool CMashParticleSystemIntermediate::IsCustomParticleSystem()const
	{
		return m_isCustomParticleSystem;
	}

	inline bool CMashParticleSystemIntermediate::IsParticleMaterialInstanced()const
	{
		return m_isMaterialInstanced;
	}

	inline void CMashParticleSystemIntermediate::SetStartTime(uint32 time)
	{
		m_particleSettings.startTime = time;
	}

	inline void CMashParticleSystemIntermediate::StopEmitter()
	{
		m_isPlaying = false;
	}

	inline bool CMashParticleSystemIntermediate::IsPlaying()const
	{
		return m_isPlaying;
	}

	inline MashParticleEmitter* CMashParticleSystemIntermediate::GetCurrentEmitter()const
	{
		return m_particleEmitter;
	}

	inline const sParticleSettings* CMashParticleSystemIntermediate::GetParticleSettings()const
	{
		return &m_particleSettings;
	}

	inline void CMashParticleSystemIntermediate::SetEmitterVelocityWeight(f32 weight)
	{
		m_particleSettings.emitterVelocityWeight = weight;
	}

	inline void CMashParticleSystemIntermediate::SetParticlesPerSecond(uint32 count)
	{
		m_particleSettings.particlesPerSecond = count;
	}

	inline void CMashParticleSystemIntermediate::SetMinStartColour(const sMashColour4 &colour)
	{
		m_particleSettings.minStartColour = colour;
	}

	inline void CMashParticleSystemIntermediate::SetMaxStartColour(const sMashColour4 &colour)
	{
		m_particleSettings.maxStartColour = colour;
	}

	inline void CMashParticleSystemIntermediate::SetMinEndColour(const sMashColour4 &colour)
	{
		m_particleSettings.minEndColour = colour;
	}

	inline void CMashParticleSystemIntermediate::SetMaxEndColour(const sMashColour4 &colour)
	{
		m_particleSettings.maxEndColour = colour;
	}

	inline void CMashParticleSystemIntermediate::SetMinRotateSpeed(f32 speed)
	{
		m_particleSettings.minRotateSpeed = speed;
	}

	inline void CMashParticleSystemIntermediate::SetMaxRotateSpeed(f32 speed)
	{
		m_particleSettings.maxRotateSpeed = speed;
	}

	inline void CMashParticleSystemIntermediate::SetMinDuration(f32 duration)
	{
		m_particleSettings.minDuration = duration;
	}

	inline void CMashParticleSystemIntermediate::SetMaxDuration(f32 duration)
	{
		m_particleSettings.maxDuration = duration;
	}

	inline void CMashParticleSystemIntermediate::SetSoftParticleScale(f32 scale)
	{
		m_particleSettings.softParticleScale = scale;
	}

	inline uint32 CMashParticleSystemIntermediate::GetMaxParticleCount()const
	{
		return m_particleSettings.maxParticleCount;
	}

	inline uint32 CMashParticleSystemIntermediate::GetParticlesPerSecond()const
	{
		return m_particleSettings.particlesPerSecond;
	}

	inline const sMashColour4& CMashParticleSystemIntermediate::GetMinStartColour()const
	{
		return m_particleSettings.minStartColour;
	}

	inline const sMashColour4& CMashParticleSystemIntermediate::GetMaxStartColour()const
	{
		return m_particleSettings.maxStartColour;
	}

	inline const sMashColour4& CMashParticleSystemIntermediate::GetMinEndColour()const
	{
		return m_particleSettings.minEndColour;
	}

	inline const sMashColour4& CMashParticleSystemIntermediate::GetMaxEndColour()const
	{
		return m_particleSettings.maxEndColour;
	}

	inline f32 CMashParticleSystemIntermediate::GetMinStartSize()const
	{
		return m_particleSettings.minStartSize;
	}

	inline f32 CMashParticleSystemIntermediate::GetMaxStartSize()const
	{
		return m_particleSettings.maxStartSize;
	}

	inline f32 CMashParticleSystemIntermediate::GetMinEndSize()const
	{
		return m_particleSettings.minEndSize;
	}

	inline f32 CMashParticleSystemIntermediate::GetMaxEndSize()const
	{
		return m_particleSettings.maxEndSize;
	}

	inline f32 CMashParticleSystemIntermediate::GetMinRotateSpeed()const
	{
		return m_particleSettings.minRotateSpeed;
	}

	inline f32 CMashParticleSystemIntermediate::GetMaxRotateSpeed()const
	{
		return m_particleSettings.maxRotateSpeed;
	}

	inline const mash::MashVector3& CMashParticleSystemIntermediate::GetMinStartVelocity()const
	{
		return m_particleSettings.minVelocity;
	}

	inline const mash::MashVector3& CMashParticleSystemIntermediate::GetMaxStartVelocity()const
	{
		return m_particleSettings.maxVelocity;
	}

	inline const mash::MashVector3& CMashParticleSystemIntermediate::GetGravity()const
	{
		return m_particleSettings.gravity;
	}

	inline f32 CMashParticleSystemIntermediate::GetMinDuration()const
	{
		return m_particleSettings.minDuration;
	}

	inline f32 CMashParticleSystemIntermediate::GetMaxDuration()const
	{
		return m_particleSettings.maxDuration;
	}

	inline f32 CMashParticleSystemIntermediate::GetSoftParticleScale()const
	{
		return m_particleSettings.softParticleScale;
	}

	inline uint32 CMashParticleSystemIntermediate::GetStartTime()const
	{
		return m_particleSettings.startTime;
	}

	inline f32 CMashParticleSystemIntermediate::GetEmitterVelocityWeight()const
	{
		return m_particleSettings.emitterVelocityWeight;
	}
}

#endif