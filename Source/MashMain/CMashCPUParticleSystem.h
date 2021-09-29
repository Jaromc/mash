//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_LOW_DETAIL_PARTICLE_SYSTEM_H_
#define _C_MASH_LOW_DETAIL_PARTICLE_SYSTEM_H_

#include "CMashParticleSystemIntermediate.h"
#include "MashTypes.h"
#include "MashGeometryBatch.h"
#include "MashMeshBuffer.h"

namespace mash
{
	class CMashCPUParticleSystem : public CMashParticleSystemIntermediate
	{
		struct sParticle
		{
			mash::MashVector3 position;
			mash::MashVector3 velocity;
			f32 startScale;
			f32 endScale;
			f32 rotation;
			f32 timeCreated;
			f32 destroyTime;
			sMashColour4 startColour;
			sMashColour4 endColour;
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

		uint32 m_positionElementLocation;
		uint32 m_positionElementSize;
		uint32 m_colourElementLocation;
		uint32 m_colourElementSize;
		uint32 m_texcoordElementLocation;
		uint32 m_texcoordElementSize;

		void ResizeParticleArray();
		void OnPassCullImpl(f32 interpolateAmount);

		void OnMaxParticleCountChange(uint32 oldCount, uint32 newCount);
		void AdvanceSystemByDelta(f32 dt);
	public:
		CMashCPUParticleSystem(MashSceneNode *parent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			ePARTICLE_TYPE particleType,
			MashMaterial *material,
			const MashStringc &sName,
			bool isCustomParticleSystem,
			bool isMaterialInstanced,
			const sParticleSettings &settings);
		~CMashCPUParticleSystem();

		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		ePARTICLE_TYPE GetParticleType()const;

		eMASH_STATUS SetModel(mash::MashModel *model, uint32 mesh = 0, uint32 lod = 0){return aMASH_OK;}
		mash::MashModel* GetModel()const{return 0;}

		bool AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr);

		bool IsShadowCaster();
		bool ContainsRenderables()const;

		void AddParticle(const mash::MashVector3 &emitterPosition, const mash::MashVector3 &emitterVelocity);
		f64 GetParticleSystemTime()const;

		MashParticleEmitter* CreatePointEmitter();

		uint32 GetNodeType()const;

		///////////renderable stuff//////////////////
		MashMeshBuffer* GetMeshBuffer()const{return m_meshBuffer;}
		int32 GetPrimitiveType()const;
		uint32 GetPrimitiveCount()const;
		uint32 GetVertexCount()const;

		MashMaterial* GetMaterial()const;
		void Draw();
	};

	inline ePARTICLE_TYPE CMashCPUParticleSystem::GetParticleType()const
	{
		return m_particleType;
	}

	inline bool CMashCPUParticleSystem::ContainsRenderables()const
	{
		return true;
	}

	inline f64 CMashCPUParticleSystem::GetParticleSystemTime()const
	{
		return m_currentInterpolatedTime;
	}

	inline uint32 CMashCPUParticleSystem::GetNodeType()const
	{
		return aNODETYPE_PARTICLE_EMITTER;
	}

	inline MashMaterial* CMashCPUParticleSystem::GetMaterial()const
	{
		return m_pMaterial;
	}

	inline int32 CMashCPUParticleSystem::GetPrimitiveType()const
	{
		return aPRIMITIVE_POINT_LIST;
	}
}

#endif