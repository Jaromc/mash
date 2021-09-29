//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashSceneLoader.h"
#include "MashSceneManager.h"
#include "MashVideo.h"
#include "MashFileStream.h"
#include "MashFileManager.h"
#include "MashDevice.h"
#include "MashVertex.h"
#include "MashDummy.h"
#include "MashModel.h"
#include "MashSubEntity.h"
#include "MashEntity.h"
#include "MashLight.h"
#include "MashCamera.h"
#include "MashMaterial.h"
#include "MashControllerManager.h"
#include "MashBone.h"
#include "MashParticleSystem.h"
#include "MashParticleEmitter.h"
#include "MashSkin.h"
#include "MashModel.h"
#include "MashStaticMesh.h"
#include "MashMaterialManager.h"
#include "MashMeshBuilder.h"
#include "MashHelper.h"
#include "MashLog.h"
#include <set>
#include "MashString.h"
#include "MashStringHelper.h"
#include "MashKeySet.h"
namespace mash
{
	const int8 ROOT_NODE_NAME[] = "Scene Root";
	static const uint32 g_MemPoolTypeSize = 10000;

	CMashSceneLoader::CMashSceneLoader():m_memoryPool(g_MemPoolTypeSize)
	{
	}

	CMashSceneLoader::~CMashSceneLoader()
	{
		
	}

	MashBone* CMashSceneLoader::FindBone(MashArray<MashBone*> &bones, const int8 *sName)const
	{
		const int32 iSize = bones.Size();
		for(int32 i = 0; i < iSize; ++i)
		{
			if (strcmp(bones[i]->GetNodeName().GetCString(), sName) == 0)
				return bones[i];
		}

		return 0;
	}

	eMASH_STATUS CMashSceneLoader::ReadNodeData(sLoadedData &loadedData,
		const uint8 *data, 
		uint32 &currentLocation,
		sNode &node)
	{
		memcpy(&node.staticData, &data[currentLocation], sizeof(sNodeStatic));
		currentLocation += sizeof(sNodeStatic);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::ReadModelData(MashDevice *pDevice, 
		sLoadedData &loadData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		//allocate memory for model
		sModel *model = (sModel*)m_memoryPool.GetMemory(sizeof(sModel));

		memcpy(&model->staticData, &data[currentLocation], sizeof(sModelStatic));
		currentLocation += sizeof(sModelStatic);

		//allocate space for model meshes
		model->meshLodList = (sMeshContainer*)m_memoryPool.GetMemory(sizeof(sMeshContainer) * model->staticData.lodCount);
		memset(model->meshLodList, 0, sizeof(sMeshContainer) * model->staticData.lodCount);

		for(int32 lod = 0; lod < model->staticData.lodCount; ++lod)
		{
			memcpy(&model->meshLodList[lod].subMeshCount, &data[currentLocation], sizeof(uint32));
			currentLocation += sizeof(uint32);

			//allocate space for lod meshes
			model->meshLodList[lod].meshArray = (sMesh*)m_memoryPool.GetMemory(sizeof(sMesh) * model->staticData.lodCount);
			memset(model->meshLodList[lod].meshArray, 0, sizeof(sMesh) * model->staticData.lodCount);

			for(int32 j = 0; j < model->meshLodList[lod].subMeshCount; ++j)
			{
				sMesh *currentMesh = &model->meshLodList[lod].meshArray[j];

				//grab jth static mesh data
				memcpy(&currentMesh->staticData, &data[currentLocation], sizeof(sMeshStatic));
				currentLocation += sizeof(sMeshStatic);
                
				//allocate memory for jth mesh geometry 
				uint32 totalVertexBufferSize = currentMesh->staticData.vertexStride * currentMesh->staticData.vertexCount;
				currentMesh->vertices = (int8*)m_memoryPool.GetMemory(totalVertexBufferSize);
				//grab jth meshes geometry data
				memcpy(currentMesh->vertices, &data[currentLocation], totalVertexBufferSize);
				currentLocation += totalVertexBufferSize;
				
				//grab index information
				currentMesh->indices = 0;
				if (currentMesh->staticData.indexFormat == aFORMAT_R16_UINT)
				{
					currentMesh->indices = (int8*)m_memoryPool.GetMemory(sizeof(uint16) * currentMesh->staticData.indexCount);
					memcpy(currentMesh->indices, &data[currentLocation], sizeof(uint16) * currentMesh->staticData.indexCount);
					currentLocation += sizeof(uint16) * currentMesh->staticData.indexCount;
				}
				else
				{
					currentMesh->indices = (int8*)m_memoryPool.GetMemory(sizeof(uint32) * currentMesh->staticData.indexCount);
					memcpy(currentMesh->indices, &data[currentLocation], sizeof(uint32) * currentMesh->staticData.indexCount);
					currentLocation += sizeof(uint32) * currentMesh->staticData.indexCount;
				}

				//grab bones' vertex influences
				currentMesh->vertexBoneWeights = 0;
				currentMesh->vertexBoneIndices = 0;
				if (currentMesh->staticData.vertexBoneInfluenceCount > 0)
				{
					currentMesh->vertexBoneWeights = (MashVector4*)m_memoryPool.GetMemory(sizeof(MashVector4) * currentMesh->staticData.vertexBoneInfluenceCount);
					memcpy(currentMesh->vertexBoneWeights, &data[currentLocation], sizeof(mash::MashVector4) * currentMesh->staticData.vertexBoneInfluenceCount);
					currentLocation += sizeof(mash::MashVector4) * currentMesh->staticData.vertexBoneInfluenceCount;

					currentMesh->vertexBoneIndices = (MashVector4*)m_memoryPool.GetMemory(sizeof(MashVector4) * currentMesh->staticData.vertexBoneInfluenceCount);
					memcpy(currentMesh->vertexBoneIndices, &data[currentLocation], sizeof(mash::MashVector4) * currentMesh->staticData.vertexBoneInfluenceCount);
					currentLocation += sizeof(mash::MashVector4) * currentMesh->staticData.vertexBoneInfluenceCount;
				}
			}
		}

		loadData.modelMap[model->staticData.fileId] = sModelContainer(model);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadModel(MashDevice *pDevice,
		sLoadedData &loadData,
		sModelContainer *modelContainer,
		const MashArray<MashArray<MashMaterial*> > &entityLodMaterials, 
		const sLoadSceneSettings &loadSettings)
	{
		if (modelContainer->engineModel)
			return aMASH_OK;

		MashSceneManager *pSceneManager = pDevice->GetSceneManager();
        MashModel *pNewModel = pSceneManager->CreateModel();

		for(uint32 lod = 0; lod < modelContainer->modelData->staticData.lodCount; ++lod)
		{
			MashArray<MashMesh*> meshBuffer;
			for(uint32 m = 0; m < modelContainer->modelData->meshLodList[lod].subMeshCount; ++m)
			{
				sMesh *currentMesh = &modelContainer->modelData->meshLodList[lod].meshArray[m];

				/*
					The vertex type a mesh was created with may be different to the material
					assigned to the mesh. To handle this, we check if the vertex types are
					equal. If not, then the mesh is converted to work with the material.

					Also note, each mesh is unique. So if a mesh is converted there is no
					need to update the loaded file data.
				*/

				MashVertex *materialVertexDecl = entityLodMaterials[lod][m]->GetVertexDeclaration();
				const sVertexData *meshVertexDecl = &loadData.vertexMap[currentMesh->staticData.vertexFileId];
				MashMesh *pNewMesh = 0;
				bool setOriginalData = true;
				if (!materialVertexDecl->IsEqual(meshVertexDecl->elements, meshVertexDecl->elementCount, 0))
				{
					//vertex decls differ so the mesh needs to convert
					MashMeshBuilder::sMesh newMesh;
					newMesh.vertices = (uint8*)currentMesh->vertices;
					newMesh.vertexCount = currentMesh->staticData.vertexCount;
					newMesh.indices = (uint8*)currentMesh->indices;
					newMesh.indexCount = currentMesh->staticData.indexCount;
					newMesh.indexFormat = (eFORMAT)currentMesh->staticData.indexFormat;

					if (currentMesh->staticData.vertexBoneInfluenceCount > 0)
					{
						newMesh.boneWeightArray = currentMesh->vertexBoneWeights;
						newMesh.boneIndexArray = currentMesh->vertexBoneIndices;
					}
					else
					{
						newMesh.boneWeightArray = 0;
						newMesh.boneIndexArray = 0;
					}

					newMesh.currentVertexElements = meshVertexDecl->elements;
					newMesh.currentVertexElementCount = meshVertexDecl->elementCount;
					newMesh.primitiveType = (ePRIMITIVE_TYPE)currentMesh->staticData.primitiveType;

					uint32 meshFlags = MashMeshBuilder::aMESH_UPDATE_FILL_MESH | 
						MashMeshBuilder::aMESH_UPDATE_CHANGE_VERTEX_FORMAT;

					pNewMesh = pSceneManager->CreateStaticMesh();
                    
					if (pDevice->GetSceneManager()->GetMeshBuilder()->UpdateMeshEx(pNewMesh, &newMesh, materialVertexDecl, meshFlags) == aMASH_FAILED)
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							"Failed to covert mesh into the materials vertex format.", 
							"CMashSceneLoader::LoadModel");

						return aMASH_FAILED;
					}

					setOriginalData = false;
				}
				
				if (setOriginalData)
				{
					//no conversions needed, load with file data
                    pNewMesh = pSceneManager->CreateStaticMesh();
					
					pNewMesh->SetGeometry(currentMesh->vertices,
						currentMesh->staticData.vertexCount, //vertex count 
						materialVertexDecl,
						currentMesh->indices, //index buffer
						currentMesh->staticData.indexCount, //indice count
						(eFORMAT)currentMesh->staticData.indexFormat,//aFORMAT_R32_UINT,
						(ePRIMITIVE_TYPE)currentMesh->staticData.primitiveType,//aPRIMITIVE_TRIANGLE_LIST,
						currentMesh->staticData.primitiveCount,
						false);//iIndexCount / 3); //number of faces
				}

				pNewMesh->SetSaveInitialiseDataFlags(loadSettings.saveGeometryFlags);
				if (loadSettings.deleteMeshInitialiseDataOnLoad)
				{
					pNewMesh->DeleteInitialiseData();
				}

				//set bone vertex influences
				if (!loadSettings.deleteMeshInitialiseDataOnLoad && (currentMesh->staticData.vertexBoneInfluenceCount > 0))
				{
					pNewMesh->SetBoneWeights(currentMesh->vertexBoneWeights, currentMesh->staticData.vertexBoneInfluenceCount);
					pNewMesh->SetBoneIndices(currentMesh->vertexBoneIndices, currentMesh->staticData.vertexBoneInfluenceCount);
				}

				pNewMesh->SetBoundingBox(mash::MashAABB(currentMesh->staticData.boundsMin,
					currentMesh->staticData.boundsMax));

				if (currentMesh->staticData.triangleBufferFileId != -1)
					pNewMesh->SetTriangleBuffer(loadData.triangleBufferMap[currentMesh->staticData.triangleBufferFileId]);

				meshBuffer.PushBack(pNewMesh);
			}

			//append this lod to the model
			if (!meshBuffer.Empty())
				pNewModel->Append(&meshBuffer[0], meshBuffer.Size());

			//drop our copy
			MashArray<MashMesh*>::Iterator mbIter = meshBuffer.Begin();
			MashArray<MashMesh*>::Iterator mbIterEnd = meshBuffer.End();
			for(; mbIter != mbIterEnd; ++mbIter)
			{
				if ((*mbIter))
					(*mbIter)->Drop();
			}

			meshBuffer.Clear();
		}

		//set triangle collider
		if (modelContainer->modelData->staticData.triangleColliderFileId != -1)
			pNewModel->SetTriangleCollider(loadData.triangleColliderMap[modelContainer->modelData->staticData.triangleColliderFileId]);

		modelContainer->engineModel = pNewModel;

		return aMASH_OK;
	}

	void CMashSceneLoader::ReadEntityData(const uint8 *data, uint32 &location, sEntity *entity)
	{
		memcpy(&entity->skinFileId, &data[location], sizeof(uint32));
		location += sizeof(uint32);

		memcpy(&entity->modelFileId, &data[location], sizeof(uint32));
		location += sizeof(uint32);

		memcpy(&entity->lodCount, &data[location], sizeof(uint32));
		location += sizeof(uint32);

		memcpy(&entity->lodDistanceCount, &data[location], sizeof(uint32));
		location += sizeof(uint32);

		entity->lodDistances = 0;
		entity->lodMeshList = 0;
		if (entity->lodDistanceCount > 0)
		{
			entity->lodDistances = (uint32*)m_memoryPool.GetMemory(sizeof(uint32) * entity->lodCount);
			memcpy(entity->lodDistances, &data[location], sizeof(uint32) * entity->lodCount);
			location += sizeof(uint32) * entity->lodCount;
		}

		if (entity->lodCount > 0)
		{
			entity->lodMeshList = (sSubEntityLodData*)m_memoryPool.GetMemory(sizeof(sSubEntityLodData) * entity->lodCount);
			for(uint32 i = 0; i < entity->lodCount; ++i)
			{
				memcpy(&entity->lodMeshList[i].meshCount, &data[location], sizeof(uint32));
				location += sizeof(uint32);

				entity->lodMeshList[i].subEntities = 0;
				if (entity->lodMeshList[i].meshCount > 0)
				{
					entity->lodMeshList[i].subEntities = (sSubEntity*)m_memoryPool.GetMemory(sizeof(sSubEntity) * entity->lodMeshList[i].meshCount);
					memcpy(entity->lodMeshList[i].subEntities, &data[location], sizeof(sSubEntity) * entity->lodMeshList[i].meshCount);
					location += sizeof(sSubEntity) * entity->lodMeshList[i].meshCount;
				}
			}
		}
	}

	eMASH_STATUS CMashSceneLoader::LoadDummy(MashDevice *pDevice,
		sLoadedData &loadedData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		MashSceneManager *pSceneManager = pDevice->GetSceneManager();
		
		sNode loadedNodeData;
		ReadNodeData(loadedData, data, currentLocation, loadedNodeData);

		MashDummy *newDummy = pSceneManager->AddDummy(0, loadedData.stringMap[loadedNodeData.staticData.nodeNameStringId]);

		if (LoadNodeCommon(pDevice, loadedData, &loadedNodeData, newDummy) == aMASH_FAILED)
		{
			return aMASH_FAILED;
		}

		loadedData.sceneNodeMap[loadedNodeData.staticData.fileId] = sSceneNodeData(newDummy, loadedNodeData.staticData.parentFileId);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadCamera(MashDevice *pDevice,
		sLoadedData &loadedData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		MashSceneManager *pSceneManager = pDevice->GetSceneManager();
		
		sCamera loadedCameraData;
		ReadNodeData(loadedData, data, currentLocation, loadedCameraData.node);

		//read camera data
		memcpy(&loadedCameraData.cameraData, &data[currentLocation], sizeof(sCameraStatic));
		currentLocation += sizeof(sCameraStatic);

		MashCamera *newCamera = 0;
		switch(loadedCameraData.cameraData.cameraType)
		{
		case aCAMERA_TYPE_FIXED:
			{
				newCamera = pSceneManager->AddCamera(0, loadedData.stringMap[loadedCameraData.node.staticData.nodeNameStringId]);
				break;
			}
		default:
			return aMASH_FAILED;
		};

		if (LoadNodeCommon(pDevice, loadedData, &loadedCameraData.node, newCamera) == aMASH_FAILED)
		{
			return aMASH_FAILED;
		}

		newCamera->SetZNear(loadedCameraData.cameraData.nearClip);
		newCamera->SetZFar(loadedCameraData.cameraData.farClip);
		newCamera->SetFOV(loadedCameraData.cameraData.fov);
		newCamera->SetAspect(loadedCameraData.cameraData.autoAspect, loadedCameraData.cameraData.aspect);
		newCamera->Enable2D(loadedCameraData.cameraData.isOrtho);

		loadedData.sceneNodeMap[loadedCameraData.node.staticData.fileId] = sSceneNodeData(newCamera,
			loadedCameraData.node.staticData.parentFileId);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadParticle(MashDevice *pDevice,
		sLoadedData &loadedData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		MashSceneManager *pSceneManager = pDevice->GetSceneManager();
		
		sParticle loadedParticleData;
		ReadNodeData(loadedData, data, currentLocation, loadedParticleData.node);

		memcpy(&loadedParticleData.particleStatic, &data[currentLocation], sizeof(sParticleStatic));
		currentLocation += sizeof(sParticleStatic);

		mash::MashModel *model = 0;
		
		if (loadedParticleData.particleStatic.modelFileId != -1)
			model = loadedData.modelMap[loadedParticleData.particleStatic.modelFileId].engineModel;

		//Load the material
		MashMaterial *material = 0;
		MashParticleSystem *particleSystem = 0;
		/*
			A particle system will only have a material if it was a custom material
		*/
		if (loadedParticleData.particleStatic.isCustom && (loadedParticleData.particleStatic.materialStringFileId != -1))
			material = pDevice->GetRenderer()->GetMaterialManager()->FindMaterial(loadedData.stringMap[loadedParticleData.particleStatic.materialStringFileId].GetCString());

		sParticleSettings particleSettings;
		particleSettings.emitterVelocityWeight = loadedParticleData.particleStatic.emitterVelocityWeight;
		particleSettings.maxParticleCount = loadedParticleData.particleStatic.particleCount;
		particleSettings.particlesPerSecond = loadedParticleData.particleStatic.particlesPerSecond;
		particleSettings.minStartColour = sMashColour4(loadedParticleData.particleStatic.minStartColour);
		particleSettings.maxStartColour = sMashColour4(loadedParticleData.particleStatic.maxStartColour);
		particleSettings.minEndColour = sMashColour4(loadedParticleData.particleStatic.minEndColour);
		particleSettings.maxEndColour = sMashColour4(loadedParticleData.particleStatic.maxEndColour);
		particleSettings.minStartSize = loadedParticleData.particleStatic.minStartSize;
		particleSettings.maxStartSize = loadedParticleData.particleStatic.maxStartSize;
		particleSettings.minEndSize = loadedParticleData.particleStatic.minEndSize;
		particleSettings.maxEndSize = loadedParticleData.particleStatic.maxEndSize;
		particleSettings.minRotateSpeed = loadedParticleData.particleStatic.minRotateSpeed;
		particleSettings.maxRotateSpeed = loadedParticleData.particleStatic.maxRotateSpeed;
		particleSettings.minVelocity = loadedParticleData.particleStatic.minVelocity;
		particleSettings.maxVelocity = loadedParticleData.particleStatic.maxVelocity;
		particleSettings.gravity = loadedParticleData.particleStatic.gravity;
		particleSettings.minDuration = loadedParticleData.particleStatic.minDuration;
		particleSettings.maxDuration = loadedParticleData.particleStatic.maxDuration;
		particleSettings.softParticleScale = loadedParticleData.particleStatic.softParticleScale;
		particleSettings.startTime = loadedParticleData.particleStatic.startTime;
		
		if (material)
		{
			particleSystem = pSceneManager->AddParticleSystemCustom(0, 
				loadedData.stringMap[loadedParticleData.node.staticData.nodeNameStringId],
				particleSettings,
				material);
		}
		else
		{
			particleSystem = pSceneManager->AddParticleSystem(0, 
				loadedData.stringMap[loadedParticleData.node.staticData.nodeNameStringId],
				particleSettings,
				(ePARTICLE_TYPE)loadedParticleData.particleStatic.particleType, 
				(eLIGHTING_TYPE)loadedParticleData.particleStatic.particleLightingType, 
				loadedParticleData.particleStatic.isMaterialInstanced, 
				model);
		}

		if (LoadNodeCommon(pDevice, loadedData, &loadedParticleData.node, particleSystem) == aMASH_FAILED)
		{
			return aMASH_FAILED;
		}

		if (loadedParticleData.particleStatic.diffuseTextureFileId != -1)
		{

			mash::MashTexture *texture = pDevice->GetRenderer()->GetTexture(loadedData.stringMap[loadedParticleData.particleStatic.diffuseTextureFileId]);
			mash::MashTextureState *textureState = 0;
			if (loadedParticleData.particleStatic.diffuseSamplerFileId != -1)
				textureState = loadedData.samplerStateMap[loadedParticleData.particleStatic.diffuseSamplerFileId];

			particleSystem->SetDiffuseTexture(texture, textureState);
		}

		switch(loadedParticleData.particleStatic.emitterType)
		{
		case aPARTICLE_EMITTER_POINT:
			particleSystem->CreatePointEmitter();
			break;
		}

		if (loadedParticleData.particleStatic.isPlaying)
			particleSystem->PlayEmitter();
		else
			particleSystem->StopEmitter();

		loadedData.sceneNodeMap[loadedParticleData.node.staticData.fileId] = sSceneNodeData(particleSystem,
			loadedParticleData.node.staticData.parentFileId);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadLight(MashDevice *pDevice,
		sLoadedData &loadedData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		MashSceneManager *pSceneManager = pDevice->GetSceneManager();
		
		sLight loadedLightData;
		ReadNodeData(loadedData, data, currentLocation, loadedLightData.node);

		//read light data
		memcpy(&loadedLightData.lightData, &data[currentLocation], sizeof(sLightStatic));
		currentLocation += sizeof(sLightStatic);

		MashLight *newLight = pSceneManager->AddLight(0, loadedData.stringMap[loadedLightData.node.staticData.nodeNameStringId], (mash::eLIGHTTYPE)loadedLightData.lightData.type, (eLIGHT_RENDERER_TYPE)loadedLightData.lightData.rendererType,
                                                     loadedLightData.lightData.isMainForwardRenderedLight);

		if (LoadNodeCommon(pDevice, loadedData, &loadedLightData.node, newLight) == aMASH_FAILED)
		{
			return aMASH_FAILED;
		}

		newLight->SetDiffuse(loadedLightData.lightData.diffuse);
		newLight->SetAmbient(loadedLightData.lightData.ambient);
		newLight->SetSpecular(loadedLightData.lightData.specular);
		newLight->SetEnableLight(loadedLightData.lightData.enabled);
		newLight->SetShadowsEnabled(loadedLightData.lightData.shadowsEnabled);
		newLight->SetAttenuation(loadedLightData.lightData.atten0, loadedLightData.lightData.atten1, loadedLightData.lightData.atten2);
		newLight->SetRange(loadedLightData.lightData.range);
		newLight->SetInnerCone(loadedLightData.lightData.innerCone);
		newLight->SetOuterCone(loadedLightData.lightData.outerCone);
		newLight->SetFalloff(loadedLightData.lightData.falloff);
        
		loadedData.sceneNodeMap[loadedLightData.node.staticData.fileId] = sSceneNodeData(newLight,
			loadedLightData.node.staticData.parentFileId);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadNodeCommon(MashDevice *pDevice,
		sLoadedData &loadedData,
		const sNode *nodeData,
		MashSceneNode *node)
	{
		node->SetPosition(nodeData->staticData.translation);
		node->SetScale(nodeData->staticData.scale);
		node->SetOrientation(nodeData->staticData.rotation);

		eMASH_STATUS status = aMASH_OK;

		if (nodeData->staticData.animationBufferFileId >= 0)
		{
			MashAnimationBuffer *buffer = loadedData.animationBufferMap[nodeData->staticData.animationBufferFileId];
			node->SetAnimationBuffer(buffer);
			//buffer->Drop();//the node now owns a copy
		}

		if (nodeData->staticData.animationMixerFileId >= 0)
		{
			MashAnimationMixer *mixer = loadedData.animationMixerMap[nodeData->staticData.animationMixerFileId].engineAnimationMixer;
			node->SetAnimationMixer(mixer);
			//mixer->Drop();//the node now owns a copy
		}

		return status;
	}

	eMASH_STATUS CMashSceneLoader::LoadEntity(MashDevice *pDevice,
		sLoadedData &loadedData,
		const uint8 *data, 
		uint32 &currentLocation,
		const sLoadSceneSettings &loadSettings)
	{
		MashSceneManager *pSceneManager = pDevice->GetSceneManager();
		
		sEntity loadedEntityData;

		ReadNodeData(loadedData, data, currentLocation, loadedEntityData.node);

		ReadEntityData(data, currentLocation, &loadedEntityData);

		MashArray<MashArray<MashMaterial*> > entityLodMaterials(loadedEntityData.lodCount, 0);
		for(uint32 lod = 0; lod < loadedEntityData.lodCount; ++lod)
		{
			entityLodMaterials[lod].Resize(loadedEntityData.lodMeshList[lod].meshCount, 0);
			for(uint32 sub = 0; sub < loadedEntityData.lodMeshList[lod].meshCount; ++sub)
			{
				MashMaterial *material = 0;
				int32 materialStringId = loadedEntityData.lodMeshList[lod].subEntities[sub].materialNameStringId;
				if (!((materialStringId == -1) || (loadedData.stringMap[materialStringId] == "none")))
				{
					const int8 *matName = loadedData.stringMap[materialStringId].GetCString();
					material = pDevice->GetRenderer()->GetMaterialManager()->FindMaterial(matName);

					if (!material)
					{
						int8 buffer[256];
						mash::helpers::PrintToBuffer(buffer, 256, "Failed to load material '%s' for entity '%s'.", loadedData.stringMap[materialStringId].GetCString(), loadedData.stringMap[loadedEntityData.node.staticData.nodeNameStringId].GetCString());
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneLoader::LoadEntity");
					}
				}

				if (!material)
				{
					material = pDevice->GetRenderer()->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_DEFAULT_MESH);
					if (!material)
					{
						int8 buffer[256];
						mash::helpers::PrintToBuffer(buffer, 256, "Failed to load default material for entity '%s'.", loadedData.stringMap[loadedEntityData.node.staticData.nodeNameStringId].GetCString());
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneLoader::LoadEntity");
					}
				}

				entityLodMaterials[lod][sub] = material;
			}
		}

		MashEntity *pNewEntity = pSceneManager->AddEntity(0, loadedData.stringMap[loadedEntityData.node.staticData.nodeNameStringId]);
		if (LoadNodeCommon(pDevice, loadedData, &loadedEntityData.node, pNewEntity) == aMASH_FAILED)
		{
			return aMASH_FAILED;
		}

		sModelContainer *modelContainer = &loadedData.modelMap[loadedEntityData.modelFileId];
		if (!modelContainer->engineModel)
			LoadModel(pDevice, loadedData, modelContainer, entityLodMaterials, loadSettings);

		if (!modelContainer->engineModel)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
				"Engine model failed to load.",
				"CMashSceneLoader::LoadEntity");

			return aMASH_FAILED;
		}

		pNewEntity->SetModel(modelContainer->engineModel);

		uint32 lodCount = pNewEntity->GetLodCount();
		for(unsigned lod = 0; lod < lodCount; ++lod)
		{
			uint32 subEntityCount = pNewEntity->GetSubEntityCount(lod);
			for(uint32 sub = 0; sub < subEntityCount; ++sub)
			{
				pNewEntity->GetSubEntity(sub, lod)->SetMaterial(entityLodMaterials[lod][sub]);
			}
		}

		for(uint32 i = 0; i < loadedEntityData.lodDistanceCount; ++i)
			pNewEntity->SetLodDistance(i, loadedEntityData.lodDistances[i]);

		loadedData.sceneNodeMap[loadedEntityData.node.staticData.fileId] = sSceneNodeData(pNewEntity, loadedEntityData.node.staticData.parentFileId);

		if (loadedEntityData.skinFileId > -1)
		{
			pNewEntity->SetSkin(loadedData.skinMap[loadedEntityData.skinFileId].engineSkin);
		}
		
		if (loadedEntityData.lodDistances)
			m_memoryPool.FreeMemory(loadedEntityData.lodDistances);

		for(uint32 i = 0; i < loadedEntityData.lodCount; ++i)
		{
			if (loadedEntityData.lodMeshList[i].subEntities)
				m_memoryPool.FreeMemory(loadedEntityData.lodMeshList[i].subEntities);
		}

		if (loadedEntityData.lodMeshList)
			m_memoryPool.FreeMemory(loadedEntityData.lodMeshList);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadBone(MashDevice *pDevice,
		sLoadedData &loadData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		sBone newBone;
		ReadNodeData(loadData, data, currentLocation, newBone.nodeData);

		memcpy(&newBone.boneData, &data[currentLocation], sizeof(sBoneData));
		currentLocation += sizeof(sBoneData);

		MashBone *newBoneSceneNode = pDevice->GetSceneManager()->AddBone(0, loadData.stringMap[newBone.nodeData.staticData.nodeNameStringId].GetCString());

		if (!newBoneSceneNode)
			return aMASH_FAILED;

		newBoneSceneNode->SetWorldBindPose(newBone.boneData.bindPose, true);
		newBoneSceneNode->SetLocalBindPose(newBone.boneData.localBindTranslation, newBone.boneData.localBindRotation, newBone.boneData.localBindScale);
        
        
		if (LoadNodeCommon(pDevice, loadData, &newBone.nodeData, newBoneSceneNode) == aMASH_FAILED)
		{
			return aMASH_FAILED;
		}

		//save the scene node so we can link parents later
		loadData.sceneNodeMap[newBone.nodeData.staticData.fileId] = sSceneNodeData(newBoneSceneNode, newBone.nodeData.staticData.parentFileId);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::ReadSkin(MashDevice *pDevice, 
		sLoadedData &loadData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		sSkin newSkin;
		memcpy(&newSkin.skinFileId, &data[currentLocation], sizeof(int32));
		currentLocation += sizeof(int32);

		memcpy(&newSkin.boneCount, &data[currentLocation], sizeof(int32));
		currentLocation += sizeof(int32);

		newSkin.boneArray = 0;
		if (newSkin.boneCount > 0)
		{
			newSkin.boneArray = (sSkinBone*)m_memoryPool.GetMemory(sizeof(sSkinBone) * newSkin.boneCount);
			memcpy(newSkin.boneArray, &data[currentLocation], sizeof(sSkinBone) * newSkin.boneCount);
			currentLocation += sizeof(sSkinBone) * newSkin.boneCount;
		}

		loadData.skinMap[newSkin.skinFileId].skinData = newSkin;
		loadData.skinMap[newSkin.skinFileId].engineSkin = pDevice->GetSceneManager()->CreateSkin();

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadAnimationBuffer(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation)
	{
		int32 fileID = 0;
		memcpy(&fileID, &data[currentLocation], sizeof(int32));
		currentLocation += sizeof(int32);

		int32 keySetCount = 0;
		memcpy(&keySetCount, &data[currentLocation], sizeof(int32));
		currentLocation += sizeof(int32);

		//sAnimation
		sAnimation *animations = 0;
		eMASH_STATUS status = aMASH_FAILED;
		if (keySetCount > 0)
		{
			status = aMASH_OK;

			//animations = MASH_ALLOC_T_COMMON(sAnimation, keySetCount);
			animations = (sAnimation*)m_memoryPool.GetMemory(sizeof(sAnimation) * keySetCount);
			for(uint32 anim = 0; anim < keySetCount; ++anim)
			{
				memcpy(&animations[anim].staticData, &data[currentLocation], sizeof(sAnimationStatic));
				currentLocation += sizeof(sAnimationStatic);

				animations[anim].controllers = 0;
				if (animations[anim].staticData.controllerCount > 0)
				{
					animations[anim].controllers = (sAnimationController*)m_memoryPool.GetMemory(sizeof(sAnimationController) * animations[anim].staticData.controllerCount);
					for(uint32 con = 0; con < animations[anim].staticData.controllerCount; ++con)
					{
						memcpy(&animations[anim].controllers[con].controllerType, &data[currentLocation], sizeof(int32));
						currentLocation += sizeof(int32);

						memcpy(&animations[anim].controllers[con].keyCount, &data[currentLocation], sizeof(int32));
						currentLocation += sizeof(int32);

						animations[anim].controllers[con].transformKey = 0;
						int32 keyCount = animations[anim].controllers[con].keyCount;
						if (keyCount > 0)
						{
							switch(animations[anim].controllers[con].controllerType)
							{
							case aCONTROLLER_TRANSFORMATION:
								{
									animations[anim].controllers[con].transformKey = (sMashAnimationKeyTransform*)m_memoryPool.GetMemory(sizeof(sMashAnimationKeyTransform) * keyCount);
									memcpy(animations[anim].controllers[con].transformKey, &data[currentLocation], sizeof(sMashAnimationKeyTransform) * keyCount);
									currentLocation += sizeof(sMashAnimationKeyTransform) * keyCount;
									break;
								}
							default:
								{
									int8 buffer[256];
									mash::helpers::PrintToBuffer(buffer, 256, "Invalid animation controller type '%d'.", animations[anim].controllers[con].controllerType);
									MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneLoader::LoadAnimationBuffer");
									status = aMASH_FAILED;
								}
							};
						}
					}
				}
			}
		}

		//load the animations
		if (status == aMASH_OK)
		{
			MashControllerManager *controllerManager = pDevice->GetSceneManager()->GetControllerManager();
			MashAnimationBuffer *animationBuffer = controllerManager->CreateAnimationBuffer();

			for(uint32 anim = 0; anim < keySetCount; ++anim)
			{
				for(uint32 con = 0; con < animations[anim].staticData.controllerCount; ++con)
				{
					MashKeySetInterface *newKeySet = 0;
					sAnimationController *controllerData = &animations[anim].controllers[con];
					switch(controllerData->controllerType)
					{
					case aCONTROLLER_TRANSFORMATION:
						{
							MashTransformationKeySet *newTransformKeySet = controllerManager->CreateTransformationKeySet();
							newTransformKeySet->AddKeyList(controllerData->transformKey, controllerData->keyCount);
							newKeySet = newTransformKeySet;
							break;
						}
					}

					if (!newKeySet)
					{
						int8 buffer[256];
						mash::helpers::PrintToBuffer(buffer, 256, "Failed to load animation controller type '%d'.", animations[anim].controllers[con].controllerType);
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneLoader::LoadAnimationBuffer");
						status = aMASH_FAILED;
					}
					else
					{
						animationBuffer->AddAnimationKeySet(loadData.stringMap[animations[anim].staticData.nameStringId].GetCString(), (eMASH_CONTROLLER_TYPE)controllerData->controllerType, newKeySet);
						newKeySet->Drop();//the buffer now owns the key set
					}
				}
			}

			loadData.animationBufferMap[fileID] = animationBuffer;
		}

		for(uint32 anim = 0; anim < keySetCount; ++anim)
		{
			for(uint32 con = 0; con < animations[anim].staticData.controllerCount; ++con)
				m_memoryPool.FreeMemory(animations[anim].controllers[con].transformKey);
			
			if (animations[anim].controllers)
				m_memoryPool.FreeMemory(animations[anim].controllers);
		}

		if (animations)
			m_memoryPool.FreeMemory(animations);
		
		return status;
	}

	eMASH_STATUS CMashSceneLoader::ReadAnimationMixer(MashDevice *pDevice, 
		sLoadedData &loadData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		sAnimationMixer animationMixerData;
		memcpy(&animationMixerData.staticData, &data[currentLocation], sizeof(sAnimationMixerStatic));
		currentLocation += sizeof(sAnimationMixerStatic);

		animationMixerData.animationSets = 0;
		if (animationMixerData.staticData.animationSetCount > 0)
		{
			animationMixerData.animationSets = (sAnimationSet*)m_memoryPool.GetMemory(sizeof(sAnimationSet) * animationMixerData.staticData.animationSetCount);
			memcpy(animationMixerData.animationSets, &data[currentLocation], sizeof(sAnimationSet) * animationMixerData.staticData.animationSetCount);
			currentLocation += sizeof(sAnimationSet) * animationMixerData.staticData.animationSetCount;
		}
		
		memcpy(&animationMixerData.affectedNodeCount, &data[currentLocation], sizeof(int32));
		currentLocation += sizeof(int32);

		animationMixerData.affectNodeFileIds = 0;
		if (animationMixerData.affectedNodeCount > 0)
		{
			animationMixerData.affectNodeFileIds = (int32*)m_memoryPool.GetMemory(sizeof(int32) * animationMixerData.affectedNodeCount);
			memcpy(animationMixerData.affectNodeFileIds, &data[currentLocation], sizeof(int32) * animationMixerData.affectedNodeCount);
			currentLocation += sizeof(int32) * animationMixerData.affectedNodeCount;
		}

		animationMixerData.engineAnimationMixer = pDevice->GetSceneManager()->GetControllerManager()->CreateMixer();

		//we need to cache this data. It will be loaded later.
		loadData.animationMixerMap[animationMixerData.staticData.fileId] = animationMixerData;

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadAnimationMixer(MashDevice *pDevice, 
		sLoadedData &loadData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		MashControllerManager *controllerManager = pDevice->GetSceneManager()->GetControllerManager();

		std::map<int32, sAnimationMixer, std::less<int32>, animationMixerAlloc >::iterator iter = loadData.animationMixerMap.begin();
		std::map<int32, sAnimationMixer, std::less<int32>, animationMixerAlloc >::iterator iterEnd = loadData.animationMixerMap.end();
		for(; iter != iterEnd; ++iter)
		{
			/*
				Adds all affected scene nodes into this mixer. Note, this must be
				done after the nodes are loaded
			*/
			for(uint32 i = 0; i < iter->second.affectedNodeCount; ++i)
				controllerManager->AddAnimationsToMixer(iter->second.engineAnimationMixer, loadData.sceneNodeMap[iter->second.affectNodeFileIds[i]].node);

			//set each animation sets data
			for(uint32 i = 0; i < iter->second.staticData.animationSetCount; ++i)
			{
				const int8 *animationSetName = loadData.stringMap[iter->second.animationSets[i].animationNameFileId].GetCString();
				iter->second.engineAnimationMixer->SetBlendMode(animationSetName, (eANIMATION_BLEND_MODE)iter->second.animationSets[i].blendMode);
				iter->second.engineAnimationMixer->SetSpeed(animationSetName, iter->second.animationSets[i].speed);
				iter->second.engineAnimationMixer->SetWeight(animationSetName, iter->second.animationSets[i].weight);
				iter->second.engineAnimationMixer->SetWrapMode(animationSetName, (eANIMATION_WRAP_MODE)iter->second.animationSets[i].wrapMode);
				iter->second.engineAnimationMixer->SetFrameRate(iter->second.animationSets[i].fps);
			}
			
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadTriangleColliderData(MashDevice *pDevice, 
		sLoadedData &loadData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		sTriangleCollider colliderData;
		memcpy(&colliderData.staticData, &data[currentLocation], sizeof(sTriangleColliderStatic));
		currentLocation += sizeof(sTriangleColliderStatic);

		colliderData.bufferFileIds = 0;
		MashTriangleBuffer **bufferArray = 0;
		if (colliderData.staticData.bufferCount > 0)
		{
			colliderData.bufferFileIds = (int32*)m_memoryPool.GetMemory(sizeof(int32) * colliderData.staticData.bufferCount);
			memcpy(colliderData.bufferFileIds, &data[currentLocation], sizeof(int32) * colliderData.staticData.bufferCount);
			currentLocation += sizeof(int32) * colliderData.staticData.bufferCount;

			bufferArray = (MashTriangleBuffer**)m_memoryPool.GetMemory(sizeof(MashTriangleBuffer*) * colliderData.staticData.bufferCount);
			for(uint32 i = 0; i < colliderData.staticData.bufferCount; ++i)
				bufferArray[i] = loadData.triangleBufferMap[colliderData.bufferFileIds[i]];
		}

		MashTriangleCollider *collider = pDevice->GetSceneManager()->CreateTriangleCollider(bufferArray, colliderData.staticData.bufferCount, (eTRIANGLE_COLLIDER_TYPE)colliderData.staticData.colliderType, true);
		
		//deserialize any spacial/tree data
		collider->Deserialize(&data[currentLocation], currentLocation);

		loadData.triangleColliderMap[colliderData.staticData.fileId] = collider;

		if (colliderData.bufferFileIds)
			m_memoryPool.FreeMemory(colliderData.bufferFileIds);

		if (bufferArray)
			m_memoryPool.FreeMemory(bufferArray);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadTriangleBufferData(MashDevice *pDevice, 
		sLoadedData &loadData,
		const uint8 *data, 
		uint32 &currentLocation)
	{
		sTriangleBuffer triangleBuffer;
		memcpy(&triangleBuffer.staticData, &data[currentLocation], sizeof(sTriangleBufferStatic));
		currentLocation += sizeof(sTriangleBufferStatic);

		triangleBuffer.uniquePoints = 0;
		if (triangleBuffer.staticData.uniqueVertexCount > 0)
		{
			triangleBuffer.uniquePoints = (MashVector3*)m_memoryPool.GetMemory(sizeof(MashVector3) * triangleBuffer.staticData.uniqueVertexCount);
			memcpy(triangleBuffer.uniquePoints, &data[currentLocation], sizeof(mash::MashVector3) * triangleBuffer.staticData.uniqueVertexCount);
			currentLocation += sizeof(mash::MashVector3) * triangleBuffer.staticData.uniqueVertexCount;
		}

		triangleBuffer.indexList = 0;
		if (triangleBuffer.staticData.indexCount > 0)
		{
			triangleBuffer.indexList = (uint32*)m_memoryPool.GetMemory(sizeof(uint32) * triangleBuffer.staticData.indexCount);
			memcpy(triangleBuffer.indexList, &data[currentLocation], sizeof(uint32) * triangleBuffer.staticData.indexCount);
			currentLocation += sizeof(uint32) * triangleBuffer.staticData.indexCount;
		}

		triangleBuffer.normalList = 0;
		if (triangleBuffer.staticData.normalCount > 0)
		{
			triangleBuffer.normalList = (MashVector3*)m_memoryPool.GetMemory(sizeof(MashVector3) * triangleBuffer.staticData.normalCount);
			memcpy(triangleBuffer.normalList, &data[currentLocation], sizeof(mash::MashVector3) * triangleBuffer.staticData.normalCount);
			currentLocation += sizeof(mash::MashVector3) * triangleBuffer.staticData.normalCount;
		}

		triangleBuffer.normalIndexList = 0;
		if (triangleBuffer.staticData.normalIndexCount > 0)
		{
			triangleBuffer.normalIndexList = (uint32*)m_memoryPool.GetMemory(sizeof(uint32) * triangleBuffer.staticData.normalIndexCount);
			memcpy(triangleBuffer.normalIndexList, &data[currentLocation], sizeof(uint32) * triangleBuffer.staticData.normalIndexCount);
			currentLocation += sizeof(uint32) * triangleBuffer.staticData.normalIndexCount;
		}

		triangleBuffer.triangleRecordList = 0;
		triangleBuffer.triangleSkinningRecordList = 0;
		if (triangleBuffer.staticData.triangleCount > 0)
		{
			triangleBuffer.triangleRecordList = (sTriangleRecord*)m_memoryPool.GetMemory(sizeof(sTriangleRecord) * triangleBuffer.staticData.triangleCount);
			memcpy(triangleBuffer.triangleRecordList, &data[currentLocation], sizeof(sTriangleRecord) * triangleBuffer.staticData.triangleCount);
			currentLocation += sizeof(sTriangleRecord) * triangleBuffer.staticData.triangleCount;

			if (triangleBuffer.staticData.hasSkinningInfo == 1)
			{
				triangleBuffer.triangleSkinningRecordList = (sTriangleSkinnngRecord*)m_memoryPool.GetMemory(sizeof(sTriangleSkinnngRecord) * triangleBuffer.staticData.triangleCount);
				memcpy(triangleBuffer.triangleSkinningRecordList, &data[currentLocation], sizeof(sTriangleSkinnngRecord) * triangleBuffer.staticData.triangleCount);
				currentLocation += sizeof(sTriangleSkinnngRecord) * triangleBuffer.staticData.triangleCount;
			}
		}

		MashTriangleBuffer *newTriangleBuffer = pDevice->GetSceneManager()->CreateTriangleBuffer();

		newTriangleBuffer->Set(triangleBuffer.staticData.uniqueVertexCount, triangleBuffer.uniquePoints,
			triangleBuffer.staticData.indexCount, triangleBuffer.indexList, 
			triangleBuffer.staticData.normalCount, triangleBuffer.normalList,
			triangleBuffer.staticData.normalIndexCount, triangleBuffer.normalIndexList,
			triangleBuffer.triangleRecordList, triangleBuffer.triangleSkinningRecordList);

		loadData.triangleBufferMap[triangleBuffer.staticData.fileId] = newTriangleBuffer;

		if (triangleBuffer.uniquePoints)
			m_memoryPool.FreeMemory(triangleBuffer.uniquePoints);

		if (triangleBuffer.indexList)
			m_memoryPool.FreeMemory(triangleBuffer.indexList);

		if (triangleBuffer.triangleRecordList)
			m_memoryPool.FreeMemory(triangleBuffer.triangleRecordList);

		if (triangleBuffer.triangleSkinningRecordList)
			m_memoryPool.FreeMemory(triangleBuffer.triangleSkinningRecordList);

		if (triangleBuffer.normalList)
			m_memoryPool.FreeMemory(triangleBuffer.normalList);

		return aMASH_OK;
	}

	eMASH_STATUS CMashSceneLoader::LoadSETFile(MashDevice *pDevice, const int8 *fileName)
	{
		MashList<mash::MashSceneNode*> rootNodes;
		sLoadSceneSettings loadSettings;
		return LoadSETFile(pDevice, fileName, rootNodes, loadSettings);
	}

	eMASH_STATUS CMashSceneLoader::LoadSETFile(MashDevice *pDevice, const int8 *fileName, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings)
	{
		if (!fileName)
			return aMASH_FAILED;

		//clear any old data from the memory pool
		m_memoryPool.Clear();

		MashFileStream *pWriter = pDevice->GetFileManager()->CreateFileStream();
		
		eMASH_STATUS status = aMASH_OK;
		if (!pWriter->LoadFile(fileName, aFILE_IO_BINARY))
		{
			int8 buffer[256];
			mash::helpers::PrintToBuffer(buffer, 256, "Failed to load nss file '%s'.", fileName);
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneLoader::LoadSETFile");

			pWriter->Destroy();
			return aMASH_FAILED;
		}
		
		if (status == aMASH_FAILED)
			return aMASH_FAILED;

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashSceneLoader::SceneFileLoader",
					"Started to load scene file : %s.",
					fileName);

		if (pWriter->GetDataSizeInBytes() < sizeof(CMashSceneLoader::sFileHeader))
		{
			pWriter->Destroy();
			return aMASH_FAILED;
		}

		sLoadedData loadedData(&m_memoryPool);
		
		const uint8 *fileData = (const uint8*)pWriter->GetData();

		CMashSceneLoader::sFileHeader fileHeader;
		uint32 currentLocation = 0;
		memcpy(&fileHeader, &fileData[currentLocation], sizeof(CMashSceneLoader::sFileHeader));
		currentLocation += sizeof(CMashSceneLoader::sFileHeader);

		//load string map
		for(uint32 i = 0; i < fileHeader.stringCount; ++i)
		{
			uint32 id = 0;
			memcpy(&id, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);

			int32 stringLength = 0;
			memcpy(&stringLength, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);

			const int8 *string = (const int8*)&fileData[currentLocation];
			currentLocation += strlen(string) + 1;//null terminator

			loadedData.stringMap.insert(std::make_pair(id, MashStringc(string)));
		}

		MashVideo *renderer = pDevice->GetRenderer();

		//load rasterizer states
		for(uint32 i = 0; i < fileHeader.rasterizerStateCount; ++i)
		{
			int32 fileId = 0;
			memcpy(&fileId, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);
			
			sRasteriserStates state;
			memcpy(&state, &fileData[currentLocation], sizeof(sRasteriserStates));
			currentLocation += sizeof(sRasteriserStates);

			int32 rasterizerStateFileId = renderer->AddRasteriserState(state);
			loadedData.rasterizerStateMap.insert(std::make_pair(fileId, rasterizerStateFileId));
		}

		//load blend states
		for(uint32 i = 0; i < fileHeader.blendStateCount; ++i)
		{
			int32 fileId = 0;
			memcpy(&fileId, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);
			
			sBlendStates state;
			memcpy(&state, &fileData[currentLocation], sizeof(sBlendStates));
			currentLocation += sizeof(sBlendStates);

			int32 blendStateFileId = renderer->AddBlendState(state);
			loadedData.blendStateMap.insert(std::make_pair(fileId, blendStateFileId));
		}

		//load sampler states
		for(uint32 i = 0; i < fileHeader.samplerStateCount; ++i)
		{
			int32 fileId = 0;
			memcpy(&fileId, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);
			
			sSamplerState state;
			memcpy(&state, &fileData[currentLocation], sizeof(sSamplerState));
			currentLocation += sizeof(sSamplerState);

			loadedData.samplerStateMap.insert(std::make_pair(fileId, (MashTextureState*)renderer->AddSamplerState(state)));
		}

		//load vertex data
		for(uint32 i = 0; i < fileHeader.vertexCount; ++i)
		{
			int32 fileId = 0;
			memcpy(&fileId, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);

			sVertexData vertexData;
			memcpy(&vertexData.elementCount, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);
			
			if (vertexData.elementCount > 0)
			{
				vertexData.elements = (sMashVertexElement*)m_memoryPool.GetMemory(sizeof(sMashVertexElement) * vertexData.elementCount);
				memcpy(vertexData.elements, &fileData[currentLocation], sizeof(sMashVertexElement) * vertexData.elementCount);
				currentLocation += sizeof(sMashVertexElement) * vertexData.elementCount;
			}

			//needs to load with the techniques
			loadedData.vertexMap[fileId] = vertexData;
		}

		for(uint32 i = 0; i < fileHeader.triangleBufferCount; ++i)
		{
			LoadTriangleBufferData(pDevice, loadedData, fileData, currentLocation);
		}

		for(uint32 i = 0; i < fileHeader.triangleColliderCount; ++i)
		{
			LoadTriangleColliderData(pDevice, loadedData, fileData, currentLocation);
		}

		/*
			Models need to be loaded using the vertex program that will
			be used to render it. So we cache the data until one of
			the scene nodes need to load it.
		*/
		for(uint32 i = 0; i < fileHeader.modelCount; ++i)
		{
			ReadModelData(pDevice, loadedData, fileData, currentLocation);
		}
		
		for(uint32 i = 0; i < fileHeader.skinCount; ++i)
		{
			ReadSkin(pDevice, loadedData, fileData, currentLocation);
		}

		for(uint32 i = 0; i < fileHeader.animationBufferCount; ++i)
		{
			LoadAnimationBuffer(pDevice, loadedData, fileData, currentLocation);
		}
		
		/*
			Animation mixer data is cached and loaded after the scene nodes.
		*/
		for(uint32 i = 0; i < fileHeader.animationMixerCount; ++i)
		{
			ReadAnimationMixer(pDevice, loadedData, fileData, currentLocation);
		}

		for(uint32 i = 0; i < fileHeader.sceneNodeCount; ++i)
		{
			int32 nodeType = 0;
			memcpy(&nodeType, &fileData[currentLocation], sizeof(int32));
			currentLocation += sizeof(int32);

			switch(nodeType)
			{
			case aNODETYPE_BONE:
				{
					LoadBone(pDevice, loadedData, fileData, currentLocation);
					break;
				}
			case aNODETYPE_DUMMY:
				{
					LoadDummy(pDevice, loadedData, fileData, currentLocation);
					break;
				}
			case aNODETYPE_CAMERA:
				{
					LoadCamera(pDevice, loadedData, fileData, currentLocation);
					break;
				}
			case aNODETYPE_DECAL:
				{
					break;
				}
			case aNODETYPE_ENTITY:
				{
					LoadEntity(pDevice, loadedData, fileData, currentLocation, loadSettings);
					break;
				}
			case aNODETYPE_LIGHT:
				{
					LoadLight(pDevice, loadedData, fileData, currentLocation);
					break;
				}
			case aNODETYPE_PARTICLE_EMITTER:
				{
					LoadParticle(pDevice, loadedData, fileData, currentLocation);
					break;
				}
			default:
				{
					int8 buffer[256];
					mash::helpers::PrintToBuffer(buffer, 256, "unknown scene node type '%i'.", nodeType);
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneLoader::LoadSETFile");
					status = aMASH_FAILED;
				}
			};
		}

		for(uint32 i = 0; i < fileHeader.animationMixerCount; ++i)
		{
			LoadAnimationMixer(pDevice, loadedData, fileData, currentLocation);
		}

		pWriter->Destroy();

		mash::MashDummy *rootNode = 0;

		if (loadSettings.createRootNode)
		{
			/*
				Get file name, minus the extension
			*/
			MashStringc rootNodeName = "";
            GetFileName(fileName, rootNodeName);

			rootNode = pDevice->GetSceneManager()->AddDummy(0, rootNodeName.GetCString());

			rootNodes.PushBack(rootNode);
		}		

		//link up nodes to their parents
		std::map<int32, sSceneNodeData, std::less<int32>, sceneNodeAlloc >::iterator sceneIter = loadedData.sceneNodeMap.begin();
		std::map<int32, sSceneNodeData, std::less<int32>, sceneNodeAlloc >::iterator sceneEndIter = loadedData.sceneNodeMap.end();
		for(; sceneIter != sceneEndIter; ++sceneIter)
		{
			//is this a root node?
			if (sceneIter->second.parentFileId == -1)
			{
				if (rootNode)
					rootNode->AddChild(sceneIter->second.node);
				else
					rootNodes.PushBack(sceneIter->second.node);
			}
			else
			{
				loadedData.sceneNodeMap[sceneIter->second.parentFileId].node->AddChild(sceneIter->second.node);
			}
		}

		//set skin data
		std::map<int32, sSkinContainer, std::less<int32>,skinAlloc >::iterator skinIter = loadedData.skinMap.begin();
		std::map<int32, sSkinContainer, std::less<int32>,skinAlloc >::iterator skinIterEnd = loadedData.skinMap.end();
		for(; skinIter != skinIterEnd; ++skinIter)
		{
			for(uint32 bone = 0; bone < skinIter->second.skinData.boneCount; ++bone)
			{
				MashSceneNode *boneNode = loadedData.sceneNodeMap[skinIter->second.skinData.boneArray[bone].boneFileId].node;

				if (!boneNode || (boneNode->GetNodeType() != aNODETYPE_BONE))
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Skin contains an invalid bone.", 
						"CMashSceneLoader::LoadSETFile");
					status = aMASH_FAILED;
				}
				else
				{
					skinIter->second.engineSkin->AddBone((MashBone*)boneNode, skinIter->second.skinData.boneArray[bone].boneSkinId);
				}
			}
		}

		if (status == aMASH_OK)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
						"CMashSceneLoader::SceneFileLoader",
						"Scene load succeeded for file '%s'.",
						fileName);
		}
		else
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashSceneLoader::SceneFileLoader",
					"Scene load failed for file '%s'.",
					fileName);
		}

		return status;
	}

	void CMashSceneLoader::sLoadedData::DropAllData()
	{
		std::map<int32, sAnimationMixer, std::less<int32>, animationMixerAlloc >::iterator mixerIter = animationMixerMap.begin();
		std::map<int32, sAnimationMixer, std::less<int32>, animationMixerAlloc >::iterator mixerIterEnd = animationMixerMap.end();
		for(; mixerIter != mixerIterEnd; ++mixerIter)
		{
			if (mixerIter->second.engineAnimationMixer)
				mixerIter->second.engineAnimationMixer->Drop();
		}

		std::map<int32, sSkinContainer, std::less<int32>,skinAlloc >::iterator skinIter = skinMap.begin();
		std::map<int32, sSkinContainer, std::less<int32>,skinAlloc >::iterator skinIterEnd = skinMap.end();
		for(; skinIter != skinIterEnd; ++skinIter)
		{
			if (skinIter->second.engineSkin)
				skinIter->second.engineSkin->Drop();
		}

		std::map<int32, sModelContainer, std::less<int32>, modelAlloc >::iterator modelIter = modelMap.begin();
		std::map<int32, sModelContainer, std::less<int32>, modelAlloc >::iterator modelIterEnd = modelMap.end();
		for(; modelIter != modelIterEnd; ++modelIter)
		{
			if (modelIter->second.engineModel)
				modelIter->second.engineModel->Drop();
		}

		std::map<int32, MashAnimationBuffer*, std::less<int32>, animationBufferAlloc >::iterator animBufferIter = animationBufferMap.begin();
		std::map<int32, MashAnimationBuffer*, std::less<int32>, animationBufferAlloc >::iterator animBufferIterEnd = animationBufferMap.end();
		for(; animBufferIter != animBufferIterEnd; ++animBufferIter)
		{
			if (animBufferIter->second)
				animBufferIter->second->Drop();
		}

		std::map<int32, MashTriangleBuffer*, std::less<int32>, triangleBufferAlloc >::iterator triBufferIter = triangleBufferMap.begin();
		std::map<int32, MashTriangleBuffer*, std::less<int32>, triangleBufferAlloc >::iterator triBufferIterEnd = triangleBufferMap.end();
		for(; triBufferIter != triBufferIterEnd; ++triBufferIter)
		{
			if (triBufferIter->second)
				triBufferIter->second->Drop();
		}

		std::map<int32, MashTriangleCollider*, std::less<int32>, triangleColliderAlloc >::iterator triColliderIter = triangleColliderMap.begin();
		std::map<int32, MashTriangleCollider*, std::less<int32>, triangleColliderAlloc >::iterator triColliderIterEnd = triangleColliderMap.end();
		for(; triColliderIter != triColliderIterEnd; ++triColliderIter)
		{
			if (triColliderIter->second)
				triColliderIter->second->Drop();
		}

		sceneNodeMap.clear();
		stringMap.clear();
		rasterizerStateMap.clear();
		blendStateMap.clear();
		samplerStateMap.clear();
		vertexMap.clear();
		animationBufferMap.clear();
		animationMixerMap.clear();
		skinMap.clear();
		modelMap.clear();
		triangleBufferMap.clear();
		triangleColliderMap.clear();
	}
}