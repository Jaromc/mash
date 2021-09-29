//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashMeshParticleSystem.h"
#include "MashDevice.h"
#include "CMashPointParticleEmitter.h"
#include "MashSceneManager.h"
#include "MashGeometryHelper.h"
#include "MashCamera.h"
#include "MashVideo.h"
#include "MashLog.h"
#include "MashVertexBuffer.h"
#include "MashMeshBuffer.h"
#include "MashMesh.h"
#include "MashMeshBuilder.h"
#include "MashTexture.h"
namespace mash
{
	CMashMeshParticleSystem::CMashMeshParticleSystem(MashSceneNode *parent, 
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			ePARTICLE_TYPE particleType,
			MashMaterial *material,
			const MashStringc &sName,
			bool isCustomParticleSystem,
			bool isMaterialInstanced,
			const sParticleSettings &settings):CMashParticleSystemIntermediate(parent, pSceneManager, sName, material, isCustomParticleSystem, isMaterialInstanced),m_pRenderer(pRenderer),
			m_pMaterial(material), m_destinationTime(0.0f), m_startTime(0.0f), m_currentInterpolatedTime(0.0f),
			m_instanceBuffer(0), m_nextAvaliableParticle(0),
			m_deadParticleIndexList(0), m_activeParticleCount(0),
			m_particleType(particleType), m_particleModel(0), m_modelMeshIndex(0), m_modelLodIndex(0)
	{
		SetParticleSettings(settings);
	}

	CMashMeshParticleSystem::~CMashMeshParticleSystem()
	{
		if (m_particleModel)
		{
			m_particleModel->Drop();
			m_particleModel = 0;
		}

		if (m_instanceBuffer)
		{
			MASH_FREE(m_instanceBuffer);
			m_instanceBuffer = 0;
		}

		if (m_deadParticleIndexList)
		{
			MASH_FREE(m_deadParticleIndexList);
			m_deadParticleIndexList = 0;
		}
	}

	MashSceneNode* CMashMeshParticleSystem::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		MashParticleSystem *newParticleSystem = (MashParticleSystem*)m_sceneManager->AddParticleSystem(parent, name, m_particleSettings, m_particleType, GetParticleLightingType(), true, m_particleModel);

		return newParticleSystem;
	}

	MashMeshBuffer* CMashMeshParticleSystem::GetMeshBuffer()const
	{
		if (!m_particleModel)
			return 0;

		return m_particleModel->GetMesh(m_modelMeshIndex, m_modelLodIndex)->GetMeshBuffer();
	}

	eMASH_STATUS CMashMeshParticleSystem::ResizeMeshInstanceBuffer()
	{
		if (m_particleSettings.maxParticleCount == 0 || !m_particleModel)
			return aMASH_OK;

		mash::MashMesh *mesh = m_particleModel->GetMesh(m_modelMeshIndex, m_modelLodIndex);

		uint32 newInstanceBufferSize = sizeof(sInstanceStreamVertex) *  m_particleSettings.maxParticleCount;
		uint32 vertexBufferCount = mesh->GetMeshBuffer()->GetVertexBufferCount();
		MashVertexBuffer *instanceBuffer = mesh->GetMeshBuffer()->GetVertexBuffer(1);
		if ((vertexBufferCount == 1) || 
			!instanceBuffer ||
			(instanceBuffer->GetBufferSize() < newInstanceBufferSize))
		{
			if (mesh->GetMeshBuffer()->ResizeVertexBuffers(1, newInstanceBufferSize, aUSAGE_DYNAMIC) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							 "Failed to resize particle vertex buffer.", 
							 "CMashMeshParticleSystem::ResizeMeshInstanceBuffer");

				return aMASH_FAILED;
			}
		}

		return aMASH_OK;
	}

	void CMashMeshParticleSystem::OnMaxParticleCountChange(uint32 oldCount, uint32 newCount)
	{
		if (newCount > oldCount)
		{
			if (m_instanceBuffer)
			{
				MASH_FREE(m_instanceBuffer);
				m_instanceBuffer = 0;
			}

			if (m_deadParticleIndexList)
			{
				MASH_FREE(m_deadParticleIndexList);
				m_deadParticleIndexList = 0;
			}

			//resize particle array
			m_instanceBuffer = MASH_ALLOC_T_COMMON(sInstanceStream, m_particleSettings.maxParticleCount);
			if (!m_instanceBuffer)
			{
				//cant allocate memory
				m_particleSettings.maxParticleCount = 0;
			}

			ResizeMeshInstanceBuffer();

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
				m_instanceBuffer[i].timeCreated = 0.0f;
				m_instanceBuffer[i].destroyTime = 0.0f;
			}
		}
	}

	MashParticleEmitter* CMashMeshParticleSystem::CreatePointEmitter()
	{
		MashParticleEmitter *emitter = MASH_NEW_COMMON CMashPointParticleEmitter(this);
		if (m_particleEmitter)
			m_particleEmitter->Drop();

		m_particleEmitter = emitter;

		return emitter;
	}

	eMASH_STATUS CMashMeshParticleSystem::SetModel(mash::MashModel *model, uint32 mesh, uint32 lod)
	{
		if (!model)
			return aMASH_OK;

		mash::MashMesh *meshPtr = model->GetMesh(mesh, lod);
		if (!meshPtr)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                     "Failed to get mesh from particle model. The mesh or lod index was invalid.", 
                     "CMashMeshParticleSystem::SetModel");

			return aMASH_FAILED;
		}

		if (m_particleModel)
			m_particleModel->Drop();

		m_particleModel = model;
		m_particleModel->Grab();

		m_modelMeshIndex = mesh;
		m_modelLodIndex = lod;

		//this should always be true, sanity check.
		if (m_pMaterial)
		{
			uint32 flags = MashMeshBuilder::aMESH_UPDATE_CHANGE_VERTEX_FORMAT;
			for(uint32 l = 0; l < model->GetLodCount(); ++l)
			{
				for(uint32 m = 0; m < model->GetMeshCount(l); ++m)
				{
					if (m_sceneManager->GetMeshBuilder()->UpdateMesh(model->GetMesh(m, l), flags, m_pMaterial->GetVertexDeclaration()) == aMASH_FAILED)
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							 "Failed to update meshes vertex declaration to match material.", 
							 "CMashMeshParticleSystem::SetModel");

						return aMASH_FAILED;
					}
				}
			}
			
			if (ResizeMeshInstanceBuffer() == aMASH_FAILED)
				return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	void CMashMeshParticleSystem::AddParticle(const mash::MashVector3 &position, const mash::MashVector3 &emitterVelocity)
	{
		if (m_activeParticleCount < m_particleSettings.maxParticleCount)
		{
			sInstanceStream *activeParticle = &m_instanceBuffer[m_deadParticleIndexList[m_nextAvaliableParticle++]];

			f32 randomValueA = math::RandomFloat(0.0f, 1.0f);
			f32 randomValueB = math::RandomFloat(0.0f, 1.0f);
			f32 randomValueC = math::RandomFloat(0.0f, 1.0f);

			activeParticle->position = position;
			activeParticle->timeCreated = m_startTime;

			activeParticle->velocity.x = math::Lerp(m_particleSettings.minVelocity.x, m_particleSettings.maxVelocity.x, randomValueA);
			activeParticle->velocity.y = math::Lerp(m_particleSettings.minVelocity.y, m_particleSettings.maxVelocity.y, randomValueB);
			activeParticle->velocity.z = math::Lerp(m_particleSettings.minVelocity.z, m_particleSettings.maxVelocity.z, randomValueC);

			//transform the velocity by the orientation of the node
			activeParticle->velocity = GetWorldTransformState().TransformRotation(activeParticle->velocity) + emitterVelocity;

			activeParticle->destroyTime = m_startTime + (math::Lerp(m_particleSettings.minDuration, m_particleSettings.maxDuration, randomValueA));

			activeParticle->rotation.x = math::Lerp(m_particleSettings.minRotateSpeed, m_particleSettings.maxRotateSpeed, randomValueA);
			activeParticle->rotation.y = math::Lerp(m_particleSettings.minRotateSpeed, m_particleSettings.maxRotateSpeed, randomValueB);
			activeParticle->rotation.z = math::Lerp(m_particleSettings.minRotateSpeed, m_particleSettings.maxRotateSpeed, randomValueC);
			activeParticle->scale = math::Lerp(m_particleSettings.minStartSize, m_particleSettings.maxStartSize, randomValueA);

			sMashColour4 startColour, endColour;
			startColour.r = math::Lerp(m_particleSettings.minStartColour.r, m_particleSettings.maxStartColour.r, randomValueA);
			startColour.g = math::Lerp(m_particleSettings.minStartColour.g, m_particleSettings.maxStartColour.g, randomValueB);
			startColour.b = math::Lerp(m_particleSettings.minStartColour.b, m_particleSettings.maxStartColour.b, randomValueC);
			startColour.a = math::Lerp(m_particleSettings.minStartColour.a, m_particleSettings.maxStartColour.a, randomValueA);

			endColour.r = math::Lerp(m_particleSettings.minEndColour.r, m_particleSettings.maxEndColour.r, randomValueA);
			endColour.g = math::Lerp(m_particleSettings.minEndColour.g, m_particleSettings.maxEndColour.g, randomValueB);
			endColour.b = math::Lerp(m_particleSettings.minEndColour.b, m_particleSettings.maxEndColour.b, randomValueC);
			endColour.a = math::Lerp(m_particleSettings.minEndColour.a, m_particleSettings.maxEndColour.a, randomValueA);

			activeParticle->startColour = startColour.ToColour();
			activeParticle->endColour = endColour.ToColour();

			++m_activeParticleCount;
		}
	}

	bool CMashMeshParticleSystem::AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr)
	{
		if (!functPtr(this))
        {
			m_sceneManager->AddRenderableToRenderQueue(this, aHLPASS_PARTICLES, stage);
            return true;
        }
        
        return false;
	}

	void CMashMeshParticleSystem::AdvanceSystemByDelta(f32 dt)
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
					if (m_instanceBuffer[i].destroyTime <= m_destinationTime)
						m_deadParticleIndexList[deadParticleCount++] = i;
					else
						++m_activeParticleCount;
				}
			}

            m_particleEmitter->Update(dt, GetUpdatedWorldTransformState().translation);
		}
	}

	void CMashMeshParticleSystem::OnPassCullImpl(f32 interpolateTime)
	{
		m_currentInterpolatedTime = math::Lerp(m_startTime, m_destinationTime, interpolateTime);
	}

	void CMashMeshParticleSystem::Draw()
	{
		if (m_particleModel && (m_activeParticleCount > 0))
		{
			mash::MashMesh *particleMesh = m_particleModel->GetMesh(m_modelMeshIndex, m_modelLodIndex);
			sInstanceStreamVertex *vertexPtr = 0;
			if (particleMesh->GetMeshBuffer()->GetVertexBuffer(1)->Lock(mash::aLOCK_WRITE_DISCARD, (void**)(&vertexPtr)) == aMASH_FAILED)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to fill particle buffer.", 
                                 "CMashMeshParticleSystem::Draw");
                
				return;
			}

			uint32 activeInstances = 0;
			sInstanceStreamVertex *currentInstanceVertexData = 0;
			sInstanceStream *currentInstanceCPUData = 0;
			mash::MashMatrix4/*tmat,*/ rmat, smat;
			mash::MashQuaternion qrot;
			mash::MashVector3 translation;
			for(uint32 i = 0; i <  m_particleSettings.maxParticleCount; ++i)
			{
				currentInstanceCPUData = &m_instanceBuffer[i];
				if (currentInstanceCPUData->destroyTime > m_currentInterpolatedTime)
				{
					f32 timeElapseSinceCreation = m_currentInterpolatedTime - currentInstanceCPUData->timeCreated;
					f32 normalizedAge = timeElapseSinceCreation / (currentInstanceCPUData->destroyTime - currentInstanceCPUData->timeCreated);

					currentInstanceVertexData = &vertexPtr[activeInstances++];
					currentInstanceVertexData->colour = currentInstanceCPUData->startColour.Lerp(currentInstanceCPUData->endColour, normalizedAge).ToColour();

					qrot.SetEuler(currentInstanceCPUData->rotation * normalizedAge);
					rmat.SetRotation(qrot);

					smat.SetScale(mash::MashVector3(currentInstanceCPUData->scale, currentInstanceCPUData->scale, currentInstanceCPUData->scale));
					translation = currentInstanceCPUData->position + (currentInstanceCPUData->velocity * timeElapseSinceCreation);
					translation += m_particleSettings.gravity * 0.5f * timeElapseSinceCreation * timeElapseSinceCreation;

					currentInstanceVertexData->world = rmat * smat;
					currentInstanceVertexData->world.SetTranslation(translation);
				}
			}

			if (particleMesh->GetMeshBuffer()->GetVertexBuffer(1)->Unlock() == aMASH_FAILED)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to fill particle buffer.", 
                                 "CMashMeshParticleSystem::Draw");
				return ;
			}

			if (!m_pMaterial)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "No particle material set.", 
                                 "CMashMeshParticleSystem::Draw");
				return ;
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
					m_pRenderer->DrawMesh(particleMesh, activeInstances);
				}
			}
		}

		return;
	}
}