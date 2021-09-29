//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashCPUParticleSystem.h"
#include "MashDevice.h"
#include "MashCamera.h"
#include "CMashPointParticleEmitter.h"
#include "MashSceneManager.h"
#include "MashGeometryHelper.h"
#include "MashVertexBuffer.h"
#include "MashVertex.h"
#include "MashVideo.h"
#include "MashLog.h"
#include "MashTexture.h"
#include "MashHelper.h"
namespace mash
{
	CMashCPUParticleSystem::CMashCPUParticleSystem(MashSceneNode *parent, 
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
			 m_deadParticleIndexList(0), m_activeParticleCount(0), m_positionElementLocation(mash::math::MaxUInt32()),
			m_positionElementSize(mash::math::MaxUInt32()), m_colourElementLocation(mash::math::MaxUInt32()), m_colourElementSize(mash::math::MaxUInt32()),
			m_texcoordElementLocation(mash::math::MaxUInt32()), m_texcoordElementSize(mash::math::MaxUInt32())
	{
		const MashVertex *currentVertex = material->GetVertexDeclaration();
		const mash::sMashVertexElement *vertexElements = currentVertex->GetVertexElements();
		for(uint32 i = 0; i < currentVertex->GetVertexElementCount(); ++i)
		{
			switch(vertexElements[i].usage)
			{
			case aDECLUSAGE_POSITION:
				{
					m_positionElementLocation = vertexElements[i].stride;
					m_positionElementSize = math::Min<uint32>(sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			case aDECLUSAGE_COLOUR:
				{
					m_colourElementLocation = vertexElements[i].stride;
					m_colourElementSize = math::Min<uint32>(sizeof(mash::sMashColour), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			case aDECLUSAGE_TEXCOORD:
				{
					m_texcoordElementLocation = vertexElements[i].stride;
					m_texcoordElementSize = math::Min<uint32>(sizeof(mash::MashVector2), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
					break;
				}
			}
		}

		SetParticleSettings(settings);
	}

	CMashCPUParticleSystem::~CMashCPUParticleSystem()
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

	MashSceneNode* CMashCPUParticleSystem::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		MashParticleSystem *newParticleSystem = (MashParticleSystem*)m_sceneManager->AddParticleSystem(parent, name, m_particleSettings, m_particleType, GetParticleLightingType());

		return newParticleSystem;
	}

	void CMashCPUParticleSystem::OnMaxParticleCountChange(uint32 oldCount, uint32 newCount)
	{
		if (newCount > oldCount)
		{
			const uint32 verticesPerParticle = 6;

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
				m_particles[i].timeCreated = 0.0f;
				m_particles[i].destroyTime = 0.0f;
			}
		}
	}

	MashParticleEmitter* CMashCPUParticleSystem::CreatePointEmitter()
	{
		MashParticleEmitter *emitter = MASH_NEW_COMMON CMashPointParticleEmitter(this);
		if (m_particleEmitter)
			m_particleEmitter->Drop();

		m_particleEmitter = emitter;

		return emitter;
	}

	uint32 CMashCPUParticleSystem::GetVertexCount()const
	{
		return m_activeParticleCount * 6;
	}

	uint32 CMashCPUParticleSystem::GetPrimitiveCount()const
	{
		return m_activeParticleCount * 2;
	}

	void CMashCPUParticleSystem::AddParticle(const mash::MashVector3 &position, const mash::MashVector3 &emitterVelocity)
	{
		if (m_activeParticleCount < m_particleSettings.maxParticleCount)
		{
			sParticle *activeParticle = &m_particles[m_deadParticleIndexList[m_nextAvaliableParticle++]];

			f32 randomValueA = math::RandomFloat(0.0f, 1.0f);
			f32 randomValueB = math::RandomFloat(0.0f, 1.0f);
			f32 randomValueC = math::RandomFloat(0.0f, 1.0f);

			activeParticle->position = position;

			activeParticle->velocity.x = math::Lerp(m_particleSettings.minVelocity.x, m_particleSettings.maxVelocity.x, randomValueA);
			activeParticle->velocity.y = math::Lerp(m_particleSettings.minVelocity.y, m_particleSettings.maxVelocity.y, randomValueB);
			activeParticle->velocity.z = math::Lerp(m_particleSettings.minVelocity.z, m_particleSettings.maxVelocity.z, randomValueC);

			//transform the velocity by the orientation of the node
			activeParticle->velocity = GetWorldTransformState().TransformRotation(activeParticle->velocity) + emitterVelocity;

			activeParticle->timeCreated = m_startTime;
			activeParticle->destroyTime = m_startTime + (math::Lerp(m_particleSettings.minDuration, m_particleSettings.maxDuration, randomValueA));
			activeParticle->rotation = math::Lerp(m_particleSettings.minRotateSpeed, m_particleSettings.maxRotateSpeed, randomValueB);
			activeParticle->startScale = math::Lerp(m_particleSettings.minStartSize, m_particleSettings.maxStartSize, randomValueA);
			activeParticle->endScale = math::Lerp(m_particleSettings.minEndSize, m_particleSettings.maxEndSize, randomValueB);

			activeParticle->startColour.r = math::Lerp(m_particleSettings.minStartColour.r, m_particleSettings.maxStartColour.r, randomValueA);
			activeParticle->startColour.g = math::Lerp(m_particleSettings.minStartColour.g, m_particleSettings.maxStartColour.g, randomValueB);
			activeParticle->startColour.b = math::Lerp(m_particleSettings.minStartColour.b, m_particleSettings.maxStartColour.b, randomValueC);
			activeParticle->startColour.a = math::Lerp(m_particleSettings.minStartColour.a, m_particleSettings.maxStartColour.a, randomValueA);

			activeParticle->endColour.r = math::Lerp(m_particleSettings.minEndColour.r, m_particleSettings.maxEndColour.r, randomValueA);
			activeParticle->endColour.g = math::Lerp(m_particleSettings.minEndColour.g, m_particleSettings.maxEndColour.g, randomValueB);
			activeParticle->endColour.b = math::Lerp(m_particleSettings.minEndColour.b, m_particleSettings.maxEndColour.b, randomValueC);
			activeParticle->endColour.a = math::Lerp(m_particleSettings.minEndColour.a, m_particleSettings.maxEndColour.a, randomValueA);

			++m_activeParticleCount;
		}
	}

	bool CMashCPUParticleSystem::AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr)
	{
		if (!functPtr(this))
        {
			m_sceneManager->AddRenderableToRenderQueue(this, aHLPASS_PARTICLES, stage);
            return true;
        }
        
        return false;
	}

	void CMashCPUParticleSystem::OnPassCullImpl(f32 interpolateTime)
	{
		m_currentInterpolatedTime = math::Lerp(m_startTime, m_destinationTime, interpolateTime);
	}

	void CMashCPUParticleSystem::AdvanceSystemByDelta(f32 dt)
	{
		m_startTime = m_destinationTime;
		m_destinationTime += dt;

		/*
			We do all this in the post update so that particles are added
			in the correct world space
		*/
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
					if (m_particles[i].destroyTime <= m_destinationTime)
						m_deadParticleIndexList[deadParticleCount++] = i;
					else
						++m_activeParticleCount;
				}
			}

            m_particleEmitter->Update(dt, GetUpdatedWorldTransformState().translation);
		}
	}

	void CMashCPUParticleSystem::Draw()
	{
		if (m_activeParticleCount > 0)
		{
			uint8 *charVertices = 0;
			if (m_meshBuffer->GetVertexBuffer()->Lock(mash::aLOCK_WRITE_DISCARD, (void**)(&charVertices)) == aMASH_FAILED)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to fill particle buffer.", 
                                 "CMashCPUParticleSystem::Draw");
                
				return;
			}

			mash::MashVector2 primtiveCorners[4] = {mash::MashVector2(-0.5f, 0.5f), 
				mash::MashVector2(0.5f, 0.5f),
				mash::MashVector2(0.5f, -0.5f),
				mash::MashVector2(-0.5f, -0.5f)};

			mash::MashVector2 particleTextureCoords[4] = {mash::MashVector2(0.0f, 0.0f), 
				mash::MashVector2(1.0f, 0.0f), 
				mash::MashVector2(1.0f, 1.0f), 
				mash::MashVector2(0.0f, 1.0f)};

			uint32 shaderCornerIndex[] = {0, 1, 3, 1, 2, 3};

			sMashColour vertexColour;
			mash::MashVector3 vertexWorldPosition;
			mash::MashVector3 tempPosition;
			mash::MashVector3 tempTexcoord;
			mash::MashMatrix4 viewMatrix = m_pRenderer->GetRenderInfo()->GetCamera()->GetView();
			viewMatrix.Invert();
			uint32 vertexSize = m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0);

			uint32 activeParticleVertices = 0;
			sParticle *cpuParticle = 0;
			for(uint32 i = 0; i <  m_particleSettings.maxParticleCount; ++i)
			{
				cpuParticle = &m_particles[i];
				if (cpuParticle->destroyTime > m_currentInterpolatedTime)
				{
					f32 timeElapseSinceCreation = m_currentInterpolatedTime - cpuParticle->timeCreated;
					f32 normalizedAge = timeElapseSinceCreation / (cpuParticle->destroyTime - cpuParticle->timeCreated);

					vertexColour = cpuParticle->startColour.Lerp(cpuParticle->endColour, normalizedAge).ToColour();

					f32 rotationAmount = cpuParticle->rotation * normalizedAge;
					f32 s = sin(rotationAmount);
					f32 c = cos(rotationAmount);

					f32 scale = math::Lerp(cpuParticle->startScale, cpuParticle->endScale, normalizedAge);

					vertexWorldPosition = cpuParticle->position + (cpuParticle->velocity * timeElapseSinceCreation);
					vertexWorldPosition += m_particleSettings.gravity * 0.5f * timeElapseSinceCreation * timeElapseSinceCreation;

					for(uint32 vert = 0; vert < 6; ++vert)
					{
						//scale
						tempPosition.x = primtiveCorners[shaderCornerIndex[vert]].x * scale;
						tempPosition.y = primtiveCorners[shaderCornerIndex[vert]].y * scale;
						tempPosition.z = 0.0f;
						/*
							//produces an old school twinkle effect
							tempPosition.x = c * tempPosition.x - s * tempPosition.y;
							tempPosition.y  = s * tempPosition.x + c * tempPosition.y;
						*/
						//rotate
						f32 rotX = c * tempPosition.x - s * tempPosition.y;
						f32 rotY  = s * tempPosition.x + c * tempPosition.y;
						tempPosition.x = rotX;
						tempPosition.y = rotY;
						//billboard
						tempPosition = viewMatrix.TransformRotation(tempPosition);
						//world position
						tempPosition.x += vertexWorldPosition.x;
						tempPosition.y += vertexWorldPosition.y;
						tempPosition.z += vertexWorldPosition.z;

						if (m_positionElementLocation != mash::math::MaxUInt32())
							memcpy(&charVertices[(activeParticleVertices * vertexSize) + m_positionElementLocation], tempPosition.v, m_positionElementSize);
						if (m_colourElementLocation != mash::math::MaxUInt32())
							memcpy(&charVertices[(activeParticleVertices * vertexSize) + m_colourElementLocation], &vertexColour.colour, m_colourElementSize);
						if (m_texcoordElementLocation != mash::math::MaxUInt32())
							memcpy(&charVertices[(activeParticleVertices * vertexSize) + m_texcoordElementLocation], particleTextureCoords[shaderCornerIndex[vert]].v, m_texcoordElementSize);
						
						++activeParticleVertices;
					}
				}
			}

			if (m_meshBuffer->GetVertexBuffer()->Unlock() == aMASH_FAILED)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to fill particle buffer.", 
                                 "CMashCPUParticleSystem::Draw");
                
				return;
			}

			if (!m_pMaterial)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "No particle material set.", 
                                 "CMashCPUParticleSystem::Draw");
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