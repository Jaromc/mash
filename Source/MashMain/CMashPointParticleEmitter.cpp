//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashPointParticleEmitter.h"
#include "MashParticleSystem.h"

namespace mash
{
	CMashPointParticleEmitter::CMashPointParticleEmitter(MashParticleSystem *owner):MashParticleEmitter(), m_currentTime(0.0), m_timeRemaining(0.0f),
		m_owner(owner), m_initialised(false), m_emitterVelocity(0.0f, 0.0f, 0.0f), 
		m_previousEmitterPosition(0.0f, 0.0f, 0.0f)
	{
	}

	CMashPointParticleEmitter::~CMashPointParticleEmitter()
	{

	}

	void CMashPointParticleEmitter::Update(f32 dt, const mash::MashVector3 &position)
	{
		m_currentTime += dt;
		if (m_owner->GetParticlesPerSecond() > 0 && m_owner->IsPlaying())
		{
			const f32 timeElapsePerParticle = 1.0f / m_owner->GetParticlesPerSecond();

			if (!m_initialised)
			{
				m_previousEmitterPosition = position;
				m_emitterVelocity = mash::MashVector3(0.0f, 0.0f, 0.0f);
				m_initialised = true;

				//do this so that particles emit right from the start, rather than wait for some time to elapse.
				m_timeRemaining += timeElapsePerParticle;
			}
			else
			{
				if (dt == 0.0f)
					dt = 0.0001f;

				m_emitterVelocity = (position - m_previousEmitterPosition) / dt;
				m_emitterVelocity *= m_owner->GetEmitterVelocityWeight();
			}

			m_timeRemaining += dt;

			while (m_timeRemaining >= timeElapsePerParticle)
			{
				m_timeRemaining -= timeElapsePerParticle;
				f32 lerp = timeElapsePerParticle / m_timeRemaining;

				mash::MashVector3 particlePos = m_previousEmitterPosition.Lerp(position, lerp);
				m_owner->AddParticle(particlePos, m_emitterVelocity);
			}

			m_previousEmitterPosition = position;
		}
	}
}