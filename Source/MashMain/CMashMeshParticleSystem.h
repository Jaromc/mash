//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_MESH_PARTICLE_SYSTEM_H_
#define _C_MASH_MESH_PARTICLE_SYSTEM_H_

#include "CMashParticleSystemIntermediate.h"
#include "MashTypes.h"
#include "MashGeometryBatch.h"
#include "MashModel.h"
namespace mash
{
	class MashMeshBuffer;

	class CMashMeshParticleSystem : public CMashParticleSystemIntermediate
	{
	private:
		struct sInstanceStream
		{
			mash::MashVector3 position;
			mash::MashVector3 velocity;
			mash::MashVector3 rotation;
			MashVector3 emitterVelocity;
			sMashColour4 startColour;
			sMashColour4 endColour;
			f32 timeCreated;
			f32 destroyTime;
			f32 scale;
		};

		struct sInstanceStreamVertex
		{
			mash::MashMatrix4 world;
			sMashColour colour;
		};
	private:
		mash::MashVideo *m_pRenderer;
		f32 m_destinationTime;
		f32 m_startTime;
		f32 m_currentInterpolatedTime;

		MashMaterial *m_pMaterial;

		mash::MashModel *m_particleModel;
		uint32 m_modelMeshIndex;
		uint32 m_modelLodIndex;

		uint32 m_nextAvaliableParticle;
		uint32 m_activeParticleCount;
		int32 *m_deadParticleIndexList;
		sInstanceStream *m_instanceBuffer;

		ePARTICLE_TYPE m_particleType;

		eMASH_STATUS ResizeMeshInstanceBuffer();
		void OnPassCullImpl(f32 interpolateAmount);

		void OnMaxParticleCountChange(uint32 oldCount, uint32 newCount);
		void AdvanceSystemByDelta(f32 dt);
	public:
		CMashMeshParticleSystem(MashSceneNode *parent, 
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			ePARTICLE_TYPE particleType,
			MashMaterial *material,
			const MashStringc &sName,
			bool isCustomParticleSystem,
			bool isMaterialInstanced,
			const sParticleSettings &settings);

		~CMashMeshParticleSystem();

		ePARTICLE_TYPE GetParticleType()const;
		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		bool AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr);

		bool IsShadowCaster();
		bool ContainsRenderables()const;

		void AddParticle(const mash::MashVector3 &emitterPosition, const mash::MashVector3 &emitterVelocity);
		f64 GetParticleSystemTime()const;

		MashParticleEmitter* CreatePointEmitter();

		eMASH_STATUS SetModel(mash::MashModel *model, uint32 mesh = 0, uint32 lod = 0);
		mash::MashModel* GetModel()const;

		uint32 GetNodeType()const;
		MashMeshBuffer* GetMeshBuffer()const;

		MashMaterial* GetMaterial()const;

		void Draw();
	};

	inline ePARTICLE_TYPE CMashMeshParticleSystem::GetParticleType()const
	{
		return m_particleType;
	}

	inline mash::MashModel* CMashMeshParticleSystem::GetModel()const
	{
		return m_particleModel;
	}

	inline bool CMashMeshParticleSystem::ContainsRenderables()const
	{
		return true;
	}

	inline f64 CMashMeshParticleSystem::GetParticleSystemTime()const
	{
		return m_currentInterpolatedTime;
	}

	inline uint32 CMashMeshParticleSystem::GetNodeType()const
	{
		return aNODETYPE_PARTICLE_EMITTER;
	}

	inline MashMaterial* CMashMeshParticleSystem::GetMaterial()const
	{
		return m_pMaterial;
	}
}

#endif