//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PARTICLE_EMITTER_H_
#define _MASH_PARTICLE_EMITTER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"

namespace mash
{
	class MashVector3;

    /*!
        Particle emitters control the position of particles when they are created.
    */
	class MashParticleEmitter : public MashReferenceCounter
	{
	public:
		MashParticleEmitter():MashReferenceCounter(){}
		virtual ~MashParticleEmitter(){}

        //! Implimented by derived classes to emit new particles.
        /*!
            This can call MashParticleSystem::_AddParticle() to create new particles
            when needed.
         
            \param dt Delta time since last update.
            \param position Position of the particle scene node.
        */
		virtual void Update(f32 dt, const MashVector3 &position) = 0;
        
        //! Gets the emitter tyepe.
		virtual ePARTICLE_EMITTER_TYPES GetEmitterType()const = 0;
	};
}

#endif