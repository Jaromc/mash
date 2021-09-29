//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGPUParticleSystem.h"
#include "MashDevice.h"
#include "CMashPointParticleEmitter.h"
#include "MashSceneManager.h"
#include "MashGeometryHelper.h"
#include "MashCamera.h"
#include "MashVertexBuffer.h"
#include "MashVertex.h"
#include "MashVideo.h"
#include "MashLog.h"
#include "MashTexture.h"
namespace mash
{
	CMashGPUParticleSystem::CMashGPUParticleSystem(MashSceneNode *parent, 
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			ePARTICLE_TYPE particleType,
			MashMaterial *material,
			const MashStringc &sName,
			bool isCustomParticleSystem,
			bool isMaterialInstanced,
			const sParticleSettings &settings):CMashParticleSystemIntermediate(parent, pSceneManager, sName, material, isCustomParticleSystem, isMaterialInstanced),m_pRenderer(pRenderer),
			m_pMaterial(material), m_destinationTime(0.0f), m_startTime(0.0f), m_currentInterpolatedTime(0.0f),
			m_particles(0), m_nextAvaliableParticle(0),
			m_particleType(particleType), m_meshBuffer(0),
			 m_deadParticleIndexList(0), m_activeParticleCount(0)
	{
		SetParticleSettings(settings);
	}

	CMashGPUParticleSystem::~CMashGPUParticleSystem()
	{
		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}

		if (m_particles)
		{
			MASH_FREE(m_particles);
			m_particles = 0;
		}

		if (m_deadParticleIndexList)
		{
			MASH_FREE(m_deadParticleIndexList);
			m_deadParticleIndexList = 0;
		}
	}

	MashSceneNode* CMashGPUParticleSystem::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		MashParticleSystem *newParticleSystem = (MashParticleSystem*)m_sceneManager->AddParticleSystem(parent, name, m_particleSettings, m_particleType, GetParticleLightingType());

		return newParticleSystem;
	}

	void CMashGPUParticleSystem::OnMaxParticleCountChange(uint32 oldCount, uint32 newCount)
	{
		const uint32 verticesPerParticle = 6;

		if (newCount > oldCount)
		{
			if (m_particles)
			{
				MASH_FREE(m_particles);
				m_particles = 0;
			}

			if (m_deadParticleIndexList)
			{
				MASH_FREE(m_deadParticleIndexList);
				m_deadParticleIndexList = 0;
			}

			uint32 totalVertexCount = m_particleSettings.maxParticleCount * verticesPerParticle;

			//resize particle array
			m_particles = MASH_ALLOC_T_COMMON(sParticle, m_particleSettings.maxParticleCount);
			if (!m_particles)
			{
				//cant allocate memory
				m_particleSettings.maxParticleCount = 0;
			}

			if (!m_meshBuffer)
			{
				sVertexStreamInit streamData;
				streamData.data = 0;
				streamData.dataSizeInBytes = totalVertexCount * m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);
				streamData.usage = aUSAGE_DYNAMIC;

				m_meshBuffer = m_pRenderer->CreateMeshBuffer(&streamData, 1, m_pMaterial->GetVertexDeclaration());
				if (!m_meshBuffer)
				{
					//error!
					m_particleSettings.maxParticleCount = 0;
				}
			}
			else
			{
				m_meshBuffer->ResizeVertexBuffers(0, totalVertexCount * m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0));
			}

			m_deadParticleIndexList = MASH_ALLOC_T_COMMON(int32, m_particleSettings.maxParticleCount);
			if (!m_deadParticleIndexList)
			{
				//cant allocate memory
				m_particleSettings.maxParticleCount = 0;
			}

			m_activeParticleCount = 0;

			for(int32 i = 0; i < m_particleSettings.maxParticleCount; ++i)
				m_deadParticleIndexList[i] = i;

			for(uint32 i = 0; i < m_particleSettings.maxParticleCount; ++i)
			{
				m_particles[i].vertices[0].time.x = 0.0f;
				m_particles[i].vertices[0].time.y = 0.0f;
			}
		}
	}

	MashParticleEmitter* CMashGPUParticleSystem::CreatePointEmitter()
	{
		MashParticleEmitter *emitter = MASH_NEW_COMMON CMashPointParticleEmitter(this);
		if (m_particleEmitter)
			m_particleEmitter->Drop();

		m_particleEmitter = emitter;

		return emitter;
	}

	uint32 CMashGPUParticleSystem::GetVertexCount()const
	{
		return m_activeParticleCount * 6;
	}

	uint32 CMashGPUParticleSystem::GetPrimitiveCount()const
	{
		return m_activeParticleCount * 2;
	}

	void CMashGPUParticleSystem::AddParticle(const mash::MashVector3 &position, const mash::MashVector3 &emitterVelocity)
	{
		if (m_activeParticleCount < m_particleSettings.maxParticleCount)
		{
			sParticle *activeParticle = &m_particles[m_deadParticleIndexList[m_nextAvaliableParticle++]];

			f32 shaderCornerIndex[] = {0, 1, 3, 1, 2, 3};

			sMashVertexParticle *activeVertex = 0;
			sMashColour4 startColour, endColour;
			sMashColour convertedStartColour, convertedEndColour;
			mash::MashVector3 velocity;
			mash::MashVector2 scale;
			f32 time, torque;

			f32 randomValueA = math::RandomFloat(0.0f, 1.0f);
			f32 randomValueB = math::RandomFloat(0.0f, 1.0f);
			f32 randomValueC = math::RandomFloat(0.0f, 1.0f);
			velocity.x = math::Lerp(m_particleSettings.minVelocity.x, m_particleSettings.maxVelocity.x, randomValueA);
			velocity.y = math::Lerp(m_particleSettings.minVelocity.y, m_particleSettings.maxVelocity.y, randomValueB);
			velocity.z = math::Lerp(m_particleSettings.minVelocity.z, m_particleSettings.maxVelocity.z, randomValueC);

			//transform the velocity by the orientation of the node
			velocity = GetWorldTransformState().TransformRotation(velocity) + emitterVelocity;
			//activeParticle->emitterVelocity = emitterVelocity;

			time = m_startTime + (math::Lerp(m_particleSettings.minDuration, m_particleSettings.maxDuration, randomValueA));
			torque = math::Lerp(m_particleSettings.minRotateSpeed, m_particleSettings.maxRotateSpeed, randomValueB);
			scale.x = math::Lerp(m_particleSettings.minStartSize, m_particleSettings.maxStartSize, randomValueA);
			scale.y = math::Lerp(m_particleSettings.minEndSize, m_particleSettings.maxEndSize, randomValueB);

			startColour.r = math::Lerp(m_particleSettings.minStartColour.r, m_particleSettings.maxStartColour.r, randomValueA);
			startColour.g = math::Lerp(m_particleSettings.minStartColour.g, m_particleSettings.maxStartColour.g, randomValueB);
			startColour.b = math::Lerp(m_particleSettings.minStartColour.b, m_particleSettings.maxStartColour.b, randomValueC);
			startColour.a = math::Lerp(m_particleSettings.minStartColour.a, m_particleSettings.maxStartColour.a, randomValueA);

			endColour.r = math::Lerp(m_particleSettings.minEndColour.r, m_particleSettings.maxEndColour.r, randomValueA);
			endColour.g = math::Lerp(m_particleSettings.minEndColour.g, m_particleSettings.maxEndColour.g, randomValueB);
			endColour.b = math::Lerp(m_particleSettings.minEndColour.b, m_particleSettings.maxEndColour.b, randomValueC);
			endColour.a = math::Lerp(m_particleSettings.minEndColour.a, m_particleSettings.maxEndColour.a, randomValueA);

			convertedStartColour = startColour.ToColour();
			convertedEndColour = endColour.ToColour();

			for(uint32 i = 0; i < 6; ++i)
			{
				activeVertex = &activeParticle->vertices[i];

				activeVertex->position.x = position.x;
				activeVertex->position.y = position.y;
				activeVertex->position.z = position.z;
				activeVertex->position.w = shaderCornerIndex[i];
				activeVertex->velocity = velocity;
				activeVertex->time.x = m_startTime;
				activeVertex->time.y = time;
				activeVertex->scale = scale;
				activeVertex->spin = torque;
				activeVertex->startColour = convertedStartColour;
				activeVertex->endColour = convertedEndColour;
			}

			++m_activeParticleCount;
		}
	}

	bool CMashGPUParticleSystem::AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr)
	{
		if (!functPtr(this))
        {
			m_sceneManager->AddRenderableToRenderQueue(this, aHLPASS_PARTICLES, stage);
            return true;
        }
        
        return false;
	}

	void CMashGPUParticleSystem::AdvanceSystemByDelta(f32 dt)
	{
		m_startTime = m_destinationTime;
		m_destinationTime += dt;

		if (m_particleEmitter && (m_particleSettings.maxParticleCount > 0))
		{
			if (m_activeParticleCount > 0)
			{
				memset(m_deadParticleIndexList, -1, sizeof(int32) * m_particleSettings.maxParticleCount);
				m_nextAvaliableParticle = 0;
				m_activeParticleCount = 0;
				uint32 deadParticleCount = 0;
				for(uint32 i = 0; i <  m_particleSettings.maxParticleCount; ++i)
				{
					if (m_particles[i].vertices[0].time.y <= m_destinationTime)
						m_deadParticleIndexList[deadParticleCount++] = i;
					else
						++m_activeParticleCount;
				}
			}

            m_particleEmitter->Update(dt, GetUpdatedWorldTransformState().translation);
		}
	}
    
	void CMashGPUParticleSystem::OnPassCullImpl(f32 interpolateTime)
	{
		m_currentInterpolatedTime = math::Lerp(m_startTime, m_destinationTime, interpolateTime);
	}

	void CMashGPUParticleSystem::Draw()
	{
		if (m_activeParticleCount > 0)
		{
			sMashVertexParticle *vertexPtr = 0;
			if (m_meshBuffer->GetVertexBuffer()->Lock(mash::aLOCK_WRITE_DISCARD, (void**)(&vertexPtr)) == aMASH_FAILED)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to lock particle buffer for writing.", 
                                 "CMashGPUParticleSystem::Draw");
                
				return;
			}

			uint32 activeParticleVertices = 0;
			for(uint32 i = 0; i <  m_particleSettings.maxParticleCount; ++i)
			{
				if (m_particles[i].vertices[0].time.y > m_currentInterpolatedTime)
				{
					memcpy(&vertexPtr[activeParticleVertices], m_particles[i].vertices, sizeof(sMashVertexParticle) * 6);
					activeParticleVertices += 6;
				}
			}

			if (m_meshBuffer->GetVertexBuffer()->Unlock() == aMASH_FAILED)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to unlock particle buffer for writing.", 
                                 "CMashGPUParticleSystem::Draw");
                
				return;;
			}

			if (!m_pMaterial)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "No particle material set.", 
                                 "CMashGPUParticleSystem::Draw");
                
				return;
            }

			if (m_pMaterial->GetHasMultipleLodLevels())
			{
				const MashCamera *pActiveCamera = m_pRenderer->GetRenderInfo()->GetCamera();
				int32 iDistanceFromCamera = (GetRenderTransformState().translation - pActiveCamera->GetWorldTransformState().translation).Length();
				m_pMaterial->UpdateActiveTechnique(iDistanceFromCamera);
			}

			//set data for both the normal and custom renderer
			m_pRenderer->GetRenderInfo()->SetWorldTransform(GetRenderTransformation());
			m_pRenderer->GetRenderInfo()->SetParticleSystem(this);

			MashCustomRenderPath *pCustomRenderer = m_pMaterial->GetCustomRenderPath();
			if (pCustomRenderer)
			{
				m_sceneManager->_AddCustomRenderPathToFlushList(pCustomRenderer);
				pCustomRenderer->AddObject(this);
			}
			else
			{
				if (m_pMaterial->OnSet())
				{
					m_pRenderer->DrawVertexList(m_meshBuffer, 
						GetVertexCount(),
						GetPrimitiveCount(),
						aPRIMITIVE_TRIANGLE_LIST);
				}
			}
		}

		return;
	}
}