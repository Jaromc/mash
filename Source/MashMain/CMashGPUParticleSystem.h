//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PARTICLE_SYSTEM_H_
#define _C_MASH_PARTICLE_SYSTEM_H_

#include "CMashParticleSystemIntermediate.h"
#include "MashTypes.h"
#include "MashGeometryBatch.h"
#include "MashMeshBuffer.h"
namespace mash
{
	class CMashGPUParticleSystem : public CMashParticleSystemIntermediate
	{
		struct sMashVertexParticle
		{
			sMashVertexParticle():position(), velocity(), spin(0.0f), scale(),
				startColour(), endColour(), time(){}

			mash::MashVector4 position;
			mash::MashVector3 velocity;
			mash::MashVector2 scale;
			mash::MashVector2 time;
			f32 spin;
			sMashColour startColour;
			sMashColour endColour;
		};

		struct sParticle
		{
            sMashVertexParticle vertices[6];
		};
	private:
		mash::MashVideo *m_pRenderer;
		f32 m_destinationTime;
		f32 m_startTime;
		f32 m_currentInterpolatedTime;
		MashMaterial *m_pMaterial;
		MashMeshBuffer *m_meshBuffer;

		uint32 m_nextAvaliableParticle;
		uint32 m_activeParticleCount;
		int32 *m_deadParticleIndexList;
		sParticle *m_particles;

		ePARTICLE_TYPE m_particleType;

		void ResizeParticleArray();
		void OnPassCullImpl(f32 interpolateAmount);

		void OnMaxParticleCountChange(uint32 oldCount, uint32 newCount);
		void AdvanceSystemByDelta(f32 dt);
	public:
		CMashGPUParticleSystem(MashSceneNode *parent, 
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			ePARTICLE_TYPE particleType,
			MashMaterial *material,
			const MashStringc &sName,
			bool isCustomParticleSystem,
			bool isMaterialInstanced,
			const sParticleSettings &settings);

		~CMashGPUParticleSystem();

		ePARTICLE_TYPE GetParticleType()const;
		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		eMASH_STATUS SetModel(mash::MashModel *model, uint32 mesh = 0, uint32 lod = 0){return aMASH_OK;}
		mash::MashModel* GetModel()const{return 0;}

		bool AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr);

		bool IsShadowCaster();
		bool ContainsRenderables()const;

		void AddParticle(const mash::MashVector3 &emitterPosition, const mash::MashVector3 &emitterVelocity);
		f64 GetParticleSystemTime()const;

		MashParticleEmitter* CreatePointEmitter();

		uint32 GetNodeType()const;

		MashMeshBuffer* GetMeshBuffer()const{return m_meshBuffer;}
		int32 GetPrimitiveType()const;
		uint32 GetPrimitiveCount()const;
		uint32 GetVertexCount()const;

		MashMaterial* GetMaterial()const;
		void Draw();
	};

	inline ePARTICLE_TYPE CMashGPUParticleSystem::GetParticleType()const
	{
		return m_particleType;
	}

	inline bool CMashGPUParticleSystem::ContainsRenderables()const
	{
		return true;
	}

	inline f64 CMashGPUParticleSystem::GetParticleSystemTime()const
	{
		return m_currentInterpolatedTime;
	}

	inline uint32 CMashGPUParticleSystem::GetNodeType()const
	{
		return aNODETYPE_PARTICLE_EMITTER;
	}

	inline MashMaterial* CMashGPUParticleSystem::GetMaterial()const
	{
		return m_pMaterial;
	}

	inline int32 CMashGPUParticleSystem::GetPrimitiveType()const
	{
		return aPRIMITIVE_POINT_LIST;
	}
}

#endif