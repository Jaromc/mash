//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_POINT_PARTICLE_EMITTER_H_
#define _C_MASH_POINT_PARTICLE_EMITTER_H_

#include "MashParticleEmitter.h"
#include "MashVector3.h"
#include "MashTypes.h"

namespace mash
{
	class MashParticleSystem;

	class CMashPointParticleEmitter : public MashParticleEmitter
	{
	private:
		f64 m_currentTime;
		f32 m_timeRemaining;
		MashParticleSystem *m_owner;

		mash::MashVector3 m_emitterVelocity;
		mash::MashVector3 m_previousEmitterPosition;
		bool m_initialised;
	public:
		CMashPointParticleEmitter(MashParticleSystem *owner);
		~CMashPointParticleEmitter();

		void Update(f32 dt, const mash::MashVector3 &position);

		ePARTICLE_EMITTER_TYPES GetEmitterType()const;
	};

	inline ePARTICLE_EMITTER_TYPES CMashPointParticleEmitter::GetEmitterType()const
	{
		return aPARTICLE_EMITTER_POINT;
	}
}

#endif