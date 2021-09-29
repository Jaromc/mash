//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PARTICLE_SYSTEM_H_
#define _MASH_PARTICLE_SYSTEM_H_

#include "MashRenderable.h"
#include "MashSceneNode.h"

namespace mash
{
	class MashModel;
	class MashTexture;
	class MashTextureState;
	class MashParticleEmitter;

    /*!
        Particle systems call on their emitters to create particles in the current scene.
     
        Emitters for a particle system can be created using MashParticleSystem::Createxx.
     
        Use PlayEmitter() and StopEmitter() to start and stop the particle flow.
    */
	class MashParticleSystem : public MashSceneNode, public MashRenderable
	{
	public:
		MashParticleSystem(MashSceneNode *parent, 
			MashSceneManager *pSceneManager,
			const MashStringc &sName):MashSceneNode(parent, pSceneManager, sName){}

		virtual ~MashParticleSystem(){}

        //! Creates and sets a point emitter to this system.
        /*!
            Drops any emitter previously set. The new emitter is called
            upon automatically by the system.
         
            The returned emitter should not be dropped.
         
            \return New point emitter.
        */
		virtual MashParticleEmitter* CreatePointEmitter() = 0;
        
        //! Gets the current emitter.
        /*!
            \return Current emitter.
        */
		virtual MashParticleEmitter* GetCurrentEmitter()const = 0;

        //! Starts the emitter spawning particles.
		virtual void PlayEmitter() = 0;
        
        //! Stops the emitter spawning particles.
		virtual void StopEmitter() = 0;
        
        //! Returns true if the emitter is currently spawing particles.
		virtual bool IsPlaying()const = 0;

        //! Was the system originally set up as a custom system.
        /*!
            Mainly used for saving and loading.
         
            \return True if this is a custom system. False otherwise.
        */
		virtual bool IsCustomParticleSystem()const = 0;
        
        //! Was the original material instanced from a built in material.
        /*!
            Mainly used for saving and loading.
         
            \return True if this system is using an instance of a built in material. False otherwise.
        */
		virtual bool IsParticleMaterialInstanced()const = 0;

		//! Sets the model that will be used for particles.
        /*!
            Only used for mesh particle systems.
         
            \param model Model particle.
            \param mesh Mesh index to use.
            \param lod Lod index to use.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SetModel(MashModel *model, uint32 mesh = 0, uint32 lod = 0) = 0;
        
        //! Gets the model used for particles.
        /*!
            Only used for mesh particle systems.
         
            \return Model particle.
        */
		virtual MashModel* GetModel()const = 0;
        
        //! Gets the up time of the particle system.
        /*!
            This time is only interpolated forward if the node passes culling.
            When it hasn't passed culling an internal time still advances forward,
            but the time returned is an interpolated time that is only used for rendering.
         */ 
		virtual f64 GetParticleSystemTime()const = 0;
        
        //! Sets the diffuse particle texture.
        /*!
            Your copy of the texture and state can be dropped after calling this.
         
            \param texture Particle diffuse texture.
            \param state Texture state.
        */
		virtual eMASH_STATUS SetDiffuseTexture(MashTexture *texture, MashTextureState *state = 0) = 0;
        
        //! Gets the diffuse texture information.
        /*!
            \return A structure that contains the diffuse texture and its state.
        */
		virtual const sTexture* GetDiffuseTexture()const = 0; 

        //! Sets the max particle count.
        /*!
            \param count Max particle count.
        */
		virtual void SetMaxParticleCount(uint32 count) = 0;
        
        //! Sets the number of particles emitted per second.
        /*!
            \param count Particles to emitt per second.
        */
		virtual void SetParticlesPerSecond(uint32 count) = 0;
        
        //! Sets the min start colour.
        /*!
            This is the colour particles will appear at the start of its life.
            Start colour is randomly interpolated from min to max.
         
            \param colour Min start colour.
        */
		virtual void SetMinStartColour(const sMashColour4 &colour) = 0;
        
        //! Sets the max start colour.
        /*!
            This is the colour particles will appear at the start of its life.
            Start colour is randomly interpolated from min to max.
         
            /param colour Max start colour.
         */
		virtual void SetMaxStartColour(const sMashColour4 &colour) = 0;
        
        //! Sets the min end colour.
        /*!
            This is the colour particles will appear at the end of its life.
            End colour is randomly interpolated from min to max.
         
            \param colour Min end colour.
        */
		virtual void SetMinEndColour(const sMashColour4 &colour) = 0;
        
        //! Sets the max end colour.
        /*!
            This is the colour particles will appear at the end of its life.
            End colour is randomly interpolated from min to max.
         
            \param colour Max end colour.
         */
		virtual void SetMaxEndColour(const sMashColour4 &colour) = 0;
        
        //! Sets the min start size.
        /*!
            Size the particle will be at the start of its life.
            Start size is randomly interpolated from min to max.
         
            \param size Particle start size.
        */
		virtual void SetMinStartSize(f32 size) = 0;
        
        //! Sets the max start size.
        /*!
            Size the particle will be at the start of its life.
            Start size is randomly interpolated from min to max.
         
            \param size Particle start size.
         */
		virtual void SetMaxStartSize(f32 size) = 0;
        
        //! Sets the min end size.
        /*!
            Size the particle will be at the end of its life.
            End size is randomly interpolated from min to max.
         
            \param size Particle end size.
         */
		virtual void SetMinEndSize(f32 size) = 0;
        
        //! Sets the max end size.
        /*!
            Size the particle will be at the end of its life.
            End size is randomly interpolated from min to max.
         
            \param size Particle end size.
         */
		virtual void SetMaxEndSize(f32 size) = 0;
        
        //! Sets the min rotate speed.
        /*!
            Speed is randomly interpolated from min to max.
         
            \param speed Min rotate speed.
        */
		virtual void SetMinRotateSpeed(f32 speed) = 0;
        
        //! Sets the max rotate speed.
        /*!
            Speed is randomly interpolated from min to max.
            
            \param speed Max rotate speed.
         */
		virtual void SetMaxRotateSpeed(f32 speed) = 0;
        
        //! Sets the min start velocity of particles.
        /*!
            Particle velocity is randomly interpolated from start to end.
         
            \param velocity Min start velocity.
        */
		virtual void SetMinStartVelocity(const MashVector3 &velocity) = 0;
        
        //! Sets the max start velocity of particles.
        /*!
            Particle velocity is randomly interpolated from start to end.
         
            \param velocity Max start velocity.
         */
		virtual void SetMaxStartVelocity(const MashVector3 &velocity) = 0;
        
        //! Sets the gravity that particles will be affected by.
        /*!
            \param gravity Particle gravity. 
        */
		virtual void SetGravity(const MashVector3 &gravity) = 0;
        
        //! Sets the min life of a particle.
        /*!
            Particle life is randomly interpolated from min to max.
         
            \param duration Min particle duration.
        */
		virtual void SetMinDuration(f32 duration) = 0;
        
        //! Sets the max life of a particle.
        /*!
            Particle life is randomly interpolated from min to max.
         
            \param duration Max particle duration.
         */
		virtual void SetMaxDuration(f32 duration) = 0;
        
        //! Sets the soft particle scale.
        /*!
            This is only valid with soft particle systems.
            Soft particles fade particle edges when they cross scene geometry. This reduces or
            removes noticable transitions from particle to scene.
            This value is dependent of your camera clipping distance and may need to be tweeked.
            Try values from 10 - 100
         
            \param scale Soft particle scale.
        */
		virtual void SetSoftParticleScale(f32 scale) = 0;
        
        //! Sets the start time of the particle system when play is first called.
        /*!
            Normally particle systems need to 'warm up' before they ar producing
            their max amount of particles. This setting makes the particle system
            look like it's been running for a given period of time.
         
            \param time Warm up time.
        */
		virtual void SetStartTime(uint32 time) = 0;

		//! Modifies how much of the emitters velocity is added to each particles velocity.
		/*!
			\param weight A value between 0.0 and 1.0.
		*/
		virtual void SetEmitterVelocityWeight(f32 weight) = 0;

        //! Gets the lighting type this system was created with.
        /*!
            \return Particle lighting type.
        */
		virtual eLIGHTING_TYPE GetParticleLightingType() = 0;
        
        //! Gets the particle system type.
        /*!
            \return Particle system type.
        */
		virtual ePARTICLE_TYPE GetParticleType()const = 0;

        //! Adds a new particle to the system.
        /*!
            A new particle will be created if there are still particles avaliable
            to be created.
         
            This is called automatically by the emitter so users wouldn't normally 
            need to call this.
         
            \param emitterPosition Position the new particle should be created from.
            \param emitterVelocity This velocity will be added to the new particles velocity. This is handy for moving emitters.
        */
		virtual void AddParticle(const MashVector3 &emitterPosition, const MashVector3 &emitterVelocity) = 0;

        //! Sets all particle settings from a struct.
        virtual void SetParticleSettings(const sParticleSettings &settings) = 0;
        
        //! Gets particle settings.
        /*!
            \return Particle settings. This struct must not be altered.
        */
		virtual const sParticleSettings* GetParticleSettings()const = 0;

		/*!
			Used for culling. The user will need to tweak this for best performance.
			This is not calculated automatically cause it would need to be updated
			many times as particles are created.

			By default the bounds is very large and therefore never culled.
		*/
		virtual void SetLocalBoundingBox(const MashAABB &bounds) = 0;

        //! Gets the maximum particle count.
		virtual uint32 GetMaxParticleCount()const = 0;
        
        //! Gets the number of particles emitted per seconds.
		virtual uint32 GetParticlesPerSecond()const = 0;
        
        //! Gets colour particles will begin their life at.
		virtual const sMashColour4& GetMinStartColour()const = 0;
        
        //! Gets colour particles will begin their life at.
		virtual const sMashColour4& GetMaxStartColour()const = 0;
        
        //! Gets colour particles will end their life at.
		virtual const sMashColour4& GetMinEndColour()const = 0;
        
        //! Gets colour particles will end their life at.
		virtual const sMashColour4& GetMaxEndColour()const = 0;
        
        //! Gets size particles will start at.
		virtual f32 GetMinStartSize()const = 0;
        
        //! Gets size particles will start at.
		virtual f32 GetMaxStartSize()const = 0;
        
        //! Gets size particles will end at.
		virtual f32 GetMinEndSize()const = 0;
        
        //! Gets size particles will end at.
		virtual f32 GetMaxEndSize()const = 0;
        
        //! Gets the min rotation speed. 
		virtual f32 GetMinRotateSpeed()const = 0;
        
        //! Gets the max rotation speed. 
		virtual f32 GetMaxRotateSpeed()const = 0;
        
        //! Gets the minimum start velocity.
		virtual const mash::MashVector3& GetMinStartVelocity()const = 0;
        
        //! Gets the maximum start velocity.
		virtual const mash::MashVector3& GetMaxStartVelocity()const = 0;
        
        //! Gets the gravity that will affect particles.
		virtual const mash::MashVector3& GetGravity()const = 0;
        
        //! Gets the minimum particle life.
		virtual f32 GetMinDuration()const = 0;
        
        //! Gets the maximum particle life.
		virtual f32 GetMaxDuration()const = 0;
        
        //! Gets the soft particle blend amount. (For soft particles only).
		virtual f32 GetSoftParticleScale()const = 0;
        
        //! Gets the particle advance time. This advances the system forward on start.
		virtual uint32 GetStartTime()const = 0;
        
        //! Gets the emmiter velocity weight.
		virtual f32 GetEmitterVelocityWeight()const = 0;
	};
}

#endif