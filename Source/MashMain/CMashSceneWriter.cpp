//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashSceneWriter.h"
#include "MashKeySet.h"
#include "MashBone.h"
#include "MashCamera.h"
#include "MashLight.h"
#include "MashMesh.h"
#include "MashModel.h"
#include "MashSubEntity.h"
#include "MashEntity.h"
#include "MashShadowCaster.h"
#include "MashVertex.h"
#include "MashTexture.h"
#include "MashVideo.h"
#include "MashParticleSystem.h"
#include "MashParticleEmitter.h"
#include "MashSceneManager.h"
#include "MashFileStream.h"
#include "MashFileManager.h"
#include "MashDevice.h"
#include "MashHelper.h"
#include "MashLog.h"
#include <assert.h>
namespace mash
{
	int32 CMashSceneWriter::sFileOutputData::GetSceneNodeFileID(MashSceneNode *node)
	{
		std::map<MashSceneNode*, int32>::iterator iter = sceneNodeMap.find(node);
		if (iter != sceneNodeMap.end())
			return iter->second;

		return -1;
	}

	int32 CMashSceneWriter::sFileOutputData::MapString(const int8 *s)
	{
		static int32 stringCounter = 0;
		MashStringc newString;
		if (s)
			newString = s;

		std::map<MashStringc, int32>::iterator iter = stringMap.find(newString);
		if (iter != stringMap.end())
			return iter->second;
		
		int32 newId = stringCounter++;
		stringMap.insert(std::make_pair(newString, newId));
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapAnimationBuffer(MashAnimationBuffer *buffer)
	{
		if (!buffer)
			return -1;

		std::map<MashAnimationBuffer*, int32>::iterator iter = animationBufferFileIds.find(buffer);
		if (iter != animationBufferFileIds.end())
			return iter->second;

		int32 newId = nextFileID++;
		animationBufferFileIds.insert(std::make_pair(buffer, newId));
		writer->WriteAnimationBuffer(newId, buffer, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapAnimationMixer(MashAnimationMixer *mixer)
	{
		if (!mixer)
			return -1;

		std::map<MashAnimationMixer*, int32>::iterator iter = animationMixerFileIds.find(mixer);
		if (iter != animationMixerFileIds.end())
			return iter->second;

		int32 newId = nextFileID++;
		animationMixerFileIds.insert(std::make_pair(mixer, newId));
		writer->WriteAnimationMixer(newId, mixer, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapSkin(MashSkin *skin)
	{
		if (!skin)
			return -1;

		std::map<MashSkin*, int32>::iterator iter = skinMap.find(skin);
		if (iter != skinMap.end())
			return iter->second;

		int32 newId = nextFileID++;
		skinMap.insert(std::make_pair(skin, newId));
		writer->WriteSkin(newId, skin, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapModel(MashModel *model)
	{
		if (!model)
			return -1;

		std::map<MashModel*, int32>::iterator iter = modelMap.find(model);
		if (iter != modelMap.end())
			return iter->second;

		int32 newId = nextFileID++;
		modelMap.insert(std::make_pair(model, newId));
		writer->WriteModel(newId, model, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapSamplerState(MashTextureState *state)
	{
		if (!state)
			return -1;

		std::map<MashTextureState*, int32>::iterator iter = sampelrStateFileIds.find(state);
		if (iter != sampelrStateFileIds.end())
			return iter->second;

		int32 newId = nextFileID++;
		sampelrStateFileIds.insert(std::make_pair(state, newId));
		writer->WriteSamplerState(newId, state, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapRasterizerState(int32 state, MashVideo *renderer)
	{
		std::map<int32, int32>::iterator iter = rasterizerStateFileIds.find(state);
		if (iter != rasterizerStateFileIds.end())
			return iter->second;

		int32 newId = nextFileID++;
		rasterizerStateFileIds.insert(std::make_pair(state, newId));
		writer->WriteRasterizerState(newId, state, renderer, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapBlendState(int32 state, MashVideo *renderer)
	{
		std::map<int32, int32>::iterator iter = blendStateFileIds.find(state);
		if (iter != blendStateFileIds.end())
			return iter->second;

		int32 newId = nextFileID++;
		blendStateFileIds.insert(std::make_pair(state, newId));
		writer->WriteBlendState(newId, state, renderer, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapVertex(MashVertex *vertex)
	{
		if (!vertex)
			return -1;

		std::map<MashVertex*, int32>::iterator iter = vertexFileIds.find(vertex);
		if (iter != vertexFileIds.end())
			return iter->second;

		int32 newId = nextFileID++;
		vertexFileIds.insert(std::make_pair(vertex, newId));
		writer->WriteVertex(newId, vertex, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapTriangleBuffer(MashTriangleBuffer *triangleBuffer)
	{
		if (!triangleBuffer)
			return -1;

		std::map<MashTriangleBuffer*, int32>::iterator iter = triangleBufferMap.find(triangleBuffer);
		if (iter != triangleBufferMap.end())
			return iter->second;

		int32 newId = nextFileID++;
		triangleBufferMap.insert(std::make_pair(triangleBuffer, newId));
		writer->WriteTriangleBuffer(newId, triangleBuffer, *this);
		return newId;
	}

	int32 CMashSceneWriter::sFileOutputData::MapTriangleCollider(MashTriangleCollider *triangleCollider)
	{
		if (!triangleCollider)
			return -1;

		std::map<MashTriangleCollider*, int32>::iterator iter = triangleColliderMap.find(triangleCollider);
		if (iter != triangleColliderMap.end())
			return iter->second;

		int32 newId = nextFileID++;
		triangleColliderMap.insert(std::make_pair(triangleCollider, newId));
		writer->WriteTriangleCollider(newId, triangleCollider, *this);
		return newId;
	}

	void CMashSceneWriter::WriteTriangleCollider(int32 fileID, MashTriangleCollider *triangleCollider, sFileOutputData &outputData)
	{
		//file id
		outputData.triangleColliderData.Append(&fileID, sizeof(int32));
		//collider type
		outputData.triangleColliderData.AppendInt(triangleCollider->GetColliderType());
		//buffer count
		outputData.triangleColliderData.AppendInt(triangleCollider->GetTriangleBufferCount());

		//output the file id of each buffer
		MashArray<MashTriangleBuffer*>::ConstIterator iter = triangleCollider->GetTriangleBufferCollection().Begin();
		MashArray<MashTriangleBuffer*>::ConstIterator iterEnd = triangleCollider->GetTriangleBufferCollection().End();
		for(; iter != iterEnd; ++iter)
			outputData.triangleColliderData.AppendInt(outputData.MapTriangleBuffer(*iter));

		//output any internal trees/partitioning data
		triangleCollider->Serialize(outputData.triangleColliderData);
	}

	void CMashSceneWriter::WriteTriangleBuffer(int32 fileID, MashTriangleBuffer *triangleBuffer, sFileOutputData &outputData)
	{
		//file id
		outputData.triangleBufferData.Append(&fileID, sizeof(int32));
		//has skinning info
		if (!triangleBuffer->GetTriangleSkinningList().Empty())
			outputData.triangleBufferData.AppendInt(1);
		else
			outputData.triangleBufferData.AppendInt(0);
		//triangle count
		outputData.triangleBufferData.AppendInt(triangleBuffer->GetTriangleCount());
		//unique point count
		outputData.triangleBufferData.AppendInt(triangleBuffer->GetVertexList().Size());
		//index count
		outputData.triangleBufferData.AppendInt(triangleBuffer->GetIndexList().Size());
		//normal count
		outputData.triangleBufferData.AppendInt(triangleBuffer->GetNormalList().Size());
		//normal index count
		outputData.triangleBufferData.AppendInt(triangleBuffer->GetNormalIndexList().Size());

		//unique points
		if (!triangleBuffer->GetVertexList().Empty())
			outputData.triangleBufferData.Append(&triangleBuffer->GetVertexList()[0], sizeof(mash::MashVector3) * triangleBuffer->GetVertexList().Size());

		//indices
		if (!triangleBuffer->GetIndexList().Empty())
			outputData.triangleBufferData.Append(&triangleBuffer->GetIndexList()[0], sizeof(uint32) * triangleBuffer->GetIndexList().Size());

		//normals
		if (!triangleBuffer->GetNormalList().Empty())
			outputData.triangleBufferData.Append(&triangleBuffer->GetNormalList()[0], sizeof(mash::MashVector3) * triangleBuffer->GetNormalList().Size());

		//normal indices
		if (!triangleBuffer->GetNormalIndexList().Empty())
			outputData.triangleBufferData.Append(&triangleBuffer->GetNormalIndexList()[0], sizeof(uint32) * triangleBuffer->GetNormalIndexList().Size());

		//triangle list
		if (!triangleBuffer->GetTriangleList().Empty())
			outputData.triangleBufferData.Append(&triangleBuffer->GetTriangleList()[0], sizeof(sTriangleRecord) * triangleBuffer->GetTriangleList().Size());

		//triangle skinning list
		if (!triangleBuffer->GetTriangleSkinningList().Empty())
			outputData.triangleBufferData.Append(&triangleBuffer->GetTriangleSkinningList()[0], sizeof(sTriangleSkinnngRecord) * triangleBuffer->GetTriangleSkinningList().Size());
	}

	void CMashSceneWriter::WriteAnimationMixer(int32 fileID, MashAnimationMixer *mixer, sFileOutputData &outputData)
	{
		//file id
		outputData.animationMixerData.Append(&fileID, sizeof(int32));
		
		MashArray<MashStringc> animationNames;
		mixer->GetAnimationNames(animationNames);
		
		//animation set count
		int32 animationCount = animationNames.Size();
		outputData.animationMixerData.Append(&animationCount, sizeof(int32));
		for(int32 i = 0; i < animationCount; ++i)
		{
			const int8 *animationName = animationNames[i].GetCString();
			//animation name
			outputData.animationMixerData.AppendInt(outputData.MapString(animationName));
			//blend mode
			outputData.animationMixerData.AppendInt(mixer->GetBlendMode(animationName));
			//speed
			outputData.animationMixerData.AppendFloat(mixer->GetSpeed(animationName));
			//weight
			outputData.animationMixerData.AppendFloat(mixer->GetWeight(animationName));
			//wrap mode
			outputData.animationMixerData.AppendInt(mixer->GetWrapMode(animationName));
			//track
			outputData.animationMixerData.AppendInt(mixer->GetTrack(animationName));
			//fps
			outputData.animationMixerData.AppendInt(mixer->GetFrameRate());
		}

		/*
			Write nodes that are contained within this mixer. Note that the nodes must also
			be contained within the nodes being written to the current file.
		*/
		MashArray<MashSceneNode*> affectSceneNodes;
		mixer->GetAffectedSceneNodes(affectSceneNodes);
		int32 affectedNodeCount = affectSceneNodes.Size();

		//write affected node count
		outputData.animationMixerData.Append(&affectedNodeCount, sizeof(int32));
		for(int32 i = 0; i < affectedNodeCount; ++i)
		{
			int32 sceneNodeFileID = outputData.GetSceneNodeFileID(affectSceneNodes[i]);
			//write affected node file id
			if (sceneNodeFileID != -1)
				outputData.animationMixerData.Append(&sceneNodeFileID, sizeof(int32));
			else
			{
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, "CMashSceneWriter::WriteAnimationMixer",
                                    "A node '%s' is affected by a exported animation mixer but it's not contained in the exported scene.", affectSceneNodes[i]->GetNodeName().GetCString());
			}
		}
	}

	void CMashSceneWriter::WriteAnimationBuffer(int32 fileID, MashAnimationBuffer *buffer, sFileOutputData &outputData)
	{
		//file id
		outputData.animationBufferData.Append(&fileID, sizeof(int32));

		//key set count
		outputData.animationBufferData.AppendInt(buffer->GetAnimationKeySets().size());

		std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator iter = buffer->GetAnimationKeySets().begin();
		std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator endIter = buffer->GetAnimationKeySets().end();
		for(; iter != endIter; ++iter)
		{
			//animation name
			int32 stringId = outputData.MapString(iter->first.GetCString());
			outputData.animationBufferData.Append(&stringId, sizeof(int32));

			//controller count
			int32 controllerCount = iter->second.Size();
			outputData.animationBufferData.Append(&controllerCount, sizeof(int32));

			for(int32 controller = 0; controller < controllerCount; ++controller)
			{
				//controller type
				outputData.animationBufferData.Append(&iter->second[controller].controllerType, sizeof(int32));

				//key count
				int32 keyCount = iter->second[controller].keySet->GetKeyCount();
				outputData.animationBufferData.Append(&keyCount, sizeof(int32));

				if (keyCount > 0)
				{
					switch(iter->second[controller].controllerType)
					{
					case aCONTROLLER_TRANSFORMATION:
						{
							outputData.animationBufferData.Append(((MashTransformationKeySet*)iter->second[controller].keySet)->GetKeyArray().Pointer(), sizeof(sMashAnimationKeyTransform) * keyCount);
							break;
						}
					default:
						{
							int8 buffer[256];
							mash::helpers::PrintToBuffer(buffer, 256, "Invalid animation controller type '%d'.", iter->second[controller].controllerType);
							MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneWriter::WriteAnimationBuffer");
						}
					};
				}
			}
		}
	}

	void CMashSceneWriter::WriteNodeData(int32 fileId,
		MashSceneNode *node,
		sFileOutputData &outputData)
	{
		outputData.sceneNodeData.AppendInt(node->GetNodeType());
		outputData.sceneNodeData.Append(&fileId, sizeof(int32));

		//node name
		outputData.sceneNodeData.AppendInt(outputData.MapString(node->GetNodeName().GetCString()));
		//parent id
		outputData.sceneNodeData.AppendInt(outputData.GetSceneNodeFileID(node->GetParent()));
		//local position
		outputData.sceneNodeData.Append(node->GetLocalTransformState().translation.v, sizeof(MashVector3));
		//local scale
		outputData.sceneNodeData.Append(node->GetLocalTransformState().scale.v, sizeof(MashVector3));
		//local rotation
		outputData.sceneNodeData.Append(node->GetLocalTransformState().orientation.v, sizeof(MashQuaternion));

		//animation buffer
		if (node->GetAnimationBuffer())
		{
			outputData.sceneNodeData.AppendInt(outputData.MapAnimationBuffer((MashAnimationBuffer*)node->GetAnimationBuffer()));
		}
		else
		{
			int32 noBuffer = -1;
			outputData.sceneNodeData.Append(&noBuffer, sizeof(int32));
		}

		//animation mixer		
		if (node->GetAnimationMixer())
		{
			outputData.sceneNodeData.AppendInt(outputData.MapAnimationMixer(node->GetAnimationMixer()));
		}
		else
		{
			int32 noMixer = -1;
			outputData.sceneNodeData.Append(&noMixer, sizeof(int32));
		}
	}

	void CMashSceneWriter::WriteCameraData(MashSceneNode *node, sFileOutputData &outputData)
	{
		MashCamera *camera = (MashCamera*)node;
		//camera type
		outputData.sceneNodeData.AppendInt(camera->GetCameraType());
		//camera target
		outputData.sceneNodeData.Append(camera->GetTarget().v, sizeof(MashVector3));
		//near
		outputData.sceneNodeData.AppendFloat(camera->GetNear());
		//far
		outputData.sceneNodeData.AppendFloat(camera->GetFar());
		//fov
		outputData.sceneNodeData.AppendFloat(camera->GetFOV());
		//aspect
		int32 autoAspectEnabled = (camera->GetAutoAspectEnabled())?1:0;
		outputData.sceneNodeData.AppendInt(autoAspectEnabled);
		outputData.sceneNodeData.AppendFloat(camera->GetAspect());
		//ortho
		int32 is2dEnabled = (camera->Get2DEnabled())?1:0;
		outputData.sceneNodeData.AppendInt(is2dEnabled);
	}

	void CMashSceneWriter::WriteParticleData(MashSceneNode *node, mash::MashVideo *renderer, const sSaveSceneSettings &saveData, sFileOutputData &outputData)
	{
		MashParticleSystem *particleSystem = (MashParticleSystem*)node;
		outputData.sceneNodeData.AppendInt(particleSystem->GetParticleType());
		outputData.sceneNodeData.AppendInt(particleSystem->GetParticleLightingType());
		outputData.sceneNodeData.AppendInt((int32)particleSystem->IsPlaying());

		MashParticleEmitter *emitter = particleSystem->GetCurrentEmitter();
		if (!emitter)
			outputData.sceneNodeData.AppendInt(-1);
		else
			outputData.sceneNodeData.AppendInt(emitter->GetEmitterType());

		outputData.sceneNodeData.AppendInt(particleSystem->GetMaxParticleCount());
		outputData.sceneNodeData.AppendInt(particleSystem->GetParticlesPerSecond());
		outputData.sceneNodeData.AppendUnsignedInt(particleSystem->GetMinStartColour().ToColour().colour);
		outputData.sceneNodeData.AppendUnsignedInt(particleSystem->GetMaxStartColour().ToColour().colour);
		outputData.sceneNodeData.AppendUnsignedInt(particleSystem->GetMinEndColour().ToColour().colour);
		outputData.sceneNodeData.AppendUnsignedInt(particleSystem->GetMaxEndColour().ToColour().colour);
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMinStartSize());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMaxStartSize());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMinEndSize());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMaxEndSize());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMinRotateSpeed());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMaxRotateSpeed());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMinDuration());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetMaxDuration());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetSoftParticleScale());
		outputData.sceneNodeData.AppendFloat(particleSystem->GetEmitterVelocityWeight());
		outputData.sceneNodeData.AppendInt(particleSystem->GetStartTime());
		outputData.sceneNodeData.Append(&particleSystem->GetMinStartVelocity().v, sizeof(f32) * 3);
		outputData.sceneNodeData.Append(&particleSystem->GetMaxStartVelocity().v, sizeof(f32) * 3);
		outputData.sceneNodeData.Append(&particleSystem->GetGravity().v, sizeof(f32) * 3);
		outputData.sceneNodeData.AppendInt(particleSystem->IsCustomParticleSystem());
		outputData.sceneNodeData.AppendInt(particleSystem->IsParticleMaterialInstanced());

		if (!particleSystem->IsCustomParticleSystem())
			outputData.sceneNodeData.AppendInt(-1);
		else
		{
			outputData.sceneNodeData.AppendInt(outputData.MapString(particleSystem->GetMaterial()->GetMaterialName().GetCString()));	
		}

		

		const sTexture *texture = particleSystem->GetDiffuseTexture();
		if (!texture)
		{
			outputData.sceneNodeData.AppendInt(-1);
			outputData.sceneNodeData.AppendInt(-1);
		}
		else
		{
			if (!texture->state)
				outputData.sceneNodeData.AppendInt(-1);
			else
				outputData.sceneNodeData.AppendInt(outputData.MapSamplerState(texture->state));

			if (!texture->texture)
				outputData.sceneNodeData.AppendInt(-1);
			else
				outputData.sceneNodeData.AppendInt(outputData.MapString(texture->texture->GetName().GetCString()));			
		}

		mash::MashModel *model = particleSystem->GetModel();
		if (!model)
			outputData.sceneNodeData.AppendInt(-1);
		else
			outputData.sceneNodeData.AppendInt(outputData.MapModel(model));	
	}

	void CMashSceneWriter::WriteLightData(MashSceneNode *node, sFileOutputData &outputData)
	{
		MashLight *light = (MashLight*)node;
		int32 tempBool = 0;
		//light type
		outputData.sceneNodeData.AppendInt(light->GetLightType());
		//light on/off
		tempBool = light->IsLightEnabled();
		outputData.sceneNodeData.Append(&tempBool, sizeof(int32));
		//shadows enabled
		tempBool = light->IsShadowsEnabled();
		outputData.sceneNodeData.Append(&tempBool, sizeof(int32));
        //renderer type
        int32 tempInt = light->GetLightRendererType();
        outputData.sceneNodeData.Append(&tempInt, sizeof(int32));
		//is main forward rendered light
		tempBool = light->IsMainForwardRenderedLight();
		outputData.sceneNodeData.Append(&tempBool, sizeof(int32));
		//ambient colour
		outputData.sceneNodeData.Append(light->GetLightData()->ambient.v, sizeof(sMashColour4));
		//diffuse
		outputData.sceneNodeData.Append(light->GetLightData()->diffuse.v, sizeof(sMashColour4));
		//specular
		outputData.sceneNodeData.Append(light->GetLightData()->specular.v, sizeof(sMashColour4));
		//direction
		outputData.sceneNodeData.Append(light->GetLightData()->direction.v, sizeof(mash::MashVector3));
		//atten0
		outputData.sceneNodeData.AppendFloat(light->GetLightData()->atten.x);
		//atten1
		outputData.sceneNodeData.AppendFloat(light->GetLightData()->atten.y);
		//atten2
		outputData.sceneNodeData.AppendFloat(light->GetLightData()->atten.z);
		//range
		outputData.sceneNodeData.AppendFloat(light->GetLightData()->range);
		//outter cone
		outputData.sceneNodeData.AppendFloat(light->GetLightData()->outerCone);
		//inner cone
		outputData.sceneNodeData.AppendFloat(light->GetLightData()->innerCone);
		//falloff
		outputData.sceneNodeData.AppendFloat(light->GetLightData()->falloff);
	}

	void CMashSceneWriter::WriteEntityData(MashSceneNode *node, mash::MashVideo *renderer, const sSaveSceneSettings &saveData, sFileOutputData &outputData)
	{
		MashEntity *entity = (MashEntity*)node;
		//skeleton id
		outputData.sceneNodeData.AppendInt(outputData.MapSkin(entity->GetSkin()));
		//model file id
		outputData.sceneNodeData.AppendInt(outputData.MapModel(entity->GetModel()));
		
		//sub entity count
		int32 lodCount = entity->GetLodCount();
		outputData.sceneNodeData.AppendInt(lodCount);

		//lod distance count
		outputData.sceneNodeData.AppendInt(entity->GetLodDistances().Size());
		
		if (!entity->GetLodDistances().Empty())
			outputData.sceneNodeData.Append(&entity->GetLodDistances()[0], entity->GetLodDistances().Size() * sizeof(int32));

		for(uint32 lod = 0; lod < lodCount; ++lod)
		{
			//sub entity count for this lod
			int32 subEntityCount = entity->GetSubEntityCount(lod);
			outputData.sceneNodeData.AppendInt(subEntityCount);

			for(uint32 se = 0; se < subEntityCount; ++se)
			{
				MashSubEntity *subEntity = entity->GetSubEntity(se, lod);

				//is active
				outputData.sceneNodeData.AppendInt(subEntity->GetIsActive());

				MashMaterial *material = subEntity->GetMaterial();

				//write material name
				if (material)
					outputData.sceneNodeData.AppendInt(outputData.MapString(material->GetMaterialName().GetCString()));
				else
					outputData.sceneNodeData.AppendInt(-1);
			}
		}
	}

	void CMashSceneWriter::WriteBlendState(int32 fileID, int32 state, MashVideo *renderer, sFileOutputData &outputData)
	{
		//file id
		outputData.blendStatesData.Append(&fileID, sizeof(int32));

		const sBlendStates *blendState = renderer->GetBlendState(state);
		if (blendState)
			outputData.blendStatesData.Append(blendState, sizeof(sBlendStates));
		else
		{
			//default state
            sBlendStates bState;
			outputData.blendStatesData.Append(&bState, sizeof(sBlendStates));
		}
	}

	void CMashSceneWriter::WriteRasterizerState(int32 fileID, int32 state, MashVideo *renderer, sFileOutputData &outputData)
	{
		//file id
		outputData.rasterizerStatesData.Append(&fileID, sizeof(int32));

		const sRasteriserStates *rasterizerState = renderer->GetRasterizerState(state);
		if (rasterizerState)
			outputData.rasterizerStatesData.Append(rasterizerState, sizeof(sRasteriserStates));
		else
		{
			//default state
            sRasteriserStates rState;
			outputData.rasterizerStatesData.Append(&rState, sizeof(sRasteriserStates));
		}
	}

	void CMashSceneWriter::WriteSamplerState(int32 fileID, MashTextureState *state, sFileOutputData &outputData)
	{
		//file id
		outputData.samplerStatesData.Append(&fileID, sizeof(int32));
		//state
		outputData.samplerStatesData.Append(state->GetSamplerState(), sizeof(sSamplerState));
	}

	void CMashSceneWriter::WriteVertex(int32 fileID, MashVertex *vertex, sFileOutputData &outputData)
	{
		//file id
		outputData.vertexData.Append(&fileID, sizeof(int32));
		//elm count
		outputData.vertexData.AppendInt(vertex->GetVertexElementCount());
		//elms
		if (vertex->GetVertexElementCount() > 0)
			outputData.vertexData.Append(vertex->GetVertexElements(), sizeof(sMashVertexElement) * vertex->GetVertexElementCount());
	}

	void CMashSceneWriter::WriteModel(int32 fileID, MashModel *model, sFileOutputData &outputData)
	{
		//file id
		outputData.modelData.Append(&fileID, sizeof(int32));
		//name
        int32 noName = -1;
        outputData.modelData.AppendInt(noName);
		//lod count
		outputData.modelData.AppendInt(model->GetLodCount());
		//triangle collider id
		outputData.modelData.AppendInt(outputData.MapTriangleCollider(model->GetTriangleCollider()));

		for(uint32 lod = 0; lod < model->GetLodCount(); ++lod)
		{
			//mesh count for this lod
			outputData.modelData.AppendInt(model->GetMeshCount(lod));

			//output mesh data
			for(uint32 meshIndex = 0; meshIndex < model->GetMeshCount(lod); ++meshIndex)
			{
				MashMesh *mesh = model->GetMesh(meshIndex, lod);

				const MashVertex *vertex = mesh->GetVertexDeclaration();
				//name
                outputData.modelData.AppendInt(noName);
				//vertex stride
				outputData.modelData.AppendInt(vertex->GetStreamSizeInBytes(0));
				//vertex decl
				outputData.modelData.AppendInt(outputData.MapVertex((MashVertex*)vertex));
				//vertex count
				if (!mesh->GetRawVertices().Empty())
					outputData.modelData.AppendInt(mesh->GetVertexCount());
				else
					outputData.modelData.AppendInt(0);

				//index count
				if (!mesh->GetRawIndices().Empty())
					outputData.modelData.AppendInt(mesh->GetIndexCount());
				else
					outputData.modelData.AppendInt(0);

				//vertex bone influence count (same as vertex count)
				if (mesh->GetBoneIndices() && mesh->GetBoneWeights())
					outputData.modelData.AppendInt(mesh->GetVertexCount());
				else
				{
					int32 none = 0;
					outputData.modelData.AppendInt(none);
				}

				//index format
				outputData.modelData.AppendInt(mesh->GetIndexFormat());
				//primitive type
				outputData.modelData.AppendInt(mesh->GetPrimitiveType());
				//primtive count
				outputData.modelData.AppendInt(mesh->GetPrimitiveCount());

				//bb min
				outputData.modelData.Append(mesh->GetBoundingBox().min.v, sizeof(mash::MashVector3));
				//bb max
				outputData.modelData.Append(mesh->GetBoundingBox().max.v, sizeof(mash::MashVector3));

				//triangle buffer
				if (mesh->GetTriangleBuffer())
					outputData.modelData.AppendInt(outputData.MapTriangleBuffer(mesh->GetTriangleBuffer()));
				else
					outputData.modelData.AppendInt(-1);

				//vertex data
				if (!mesh->GetRawVertices().Empty())
					outputData.modelData.Append(mesh->GetRawVertices().Pointer(), mesh->GetRawVertices().GetCurrentSize());
				//index data
				if (!mesh->GetRawIndices().Empty())
					outputData.modelData.Append(mesh->GetRawIndices().Pointer(), mesh->GetRawIndices().GetCurrentSize());

				//bone data
				if (mesh->GetBoneIndices() && mesh->GetBoneWeights())
				{
					outputData.modelData.Append(mesh->GetBoneWeights()[0].v, sizeof(MashVector4) * mesh->GetVertexCount());
					outputData.modelData.Append(mesh->GetBoneIndices()[0].v, sizeof(MashVector4) * mesh->GetVertexCount());
				}
			}
		}
	}

	void CMashSceneWriter::WriteSkin(int32 fileID, MashSkin *skin, sFileOutputData &outputData)
	{
		//file id
		outputData.skeletonData.Append(&fileID, sizeof(int32));

		//bone count
		outputData.skeletonData.AppendInt(skin->GetBones().Size());
		
		//bones that make up this skin
		MashArray<MashSkin::sBone>::ConstIterator boneIter = skin->GetBones().Begin();
		MashArray<MashSkin::sBone>::ConstIterator boneIterEnd = skin->GetBones().End();
		for(; boneIter != boneIterEnd; ++boneIter)
		{
			//write affected node file id
			int32 sceneNodeFileID = outputData.GetSceneNodeFileID(boneIter->node);
			outputData.skeletonData.AppendInt(sceneNodeFileID);
			outputData.skeletonData.AppendInt(boneIter->id);

			if (sceneNodeFileID == -1)
			{
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, "CMashSceneWriter::WriteSkeleton",
                                    "A bone '%s' is contained in an exported skeleton but it's not contained in the exported scene.", boneIter->node->GetNodeName().GetCString());
			}
		}
	}

	void CMashSceneWriter::WriteBoneData(mash::MashSceneNode *node, sFileOutputData &outputData)
	{
		MashBone *boneNode = (MashBone*)node;

		outputData.sceneNodeData.Append(boneNode->GetInverseWorldBindPose().v, sizeof(mash::MashMatrix4));
		
		outputData.sceneNodeData.Append(boneNode->GetLocalBindPosition().v, sizeof(MashVector3));
		//scale
		outputData.sceneNodeData.Append(boneNode->GetLocalBindScale().v, sizeof(MashVector3));
		//rotation
		outputData.sceneNodeData.Append(boneNode->GetLocalBindRotation().v, sizeof(MashQuaternion));
	}

	void CMashSceneWriter::WriteSceneNode(int32 fileId,
		MashDevice *pDevice, 
		MashSceneNode *root,
		sFileOutputData &outputData, 
		const sSaveSceneSettings &saveData)
	{
		eNODE_TYPE nodeType = (eNODE_TYPE)root->GetNodeType();
		switch(nodeType)
		{
		case aNODETYPE_BONE:
			{
				WriteNodeData(fileId, root, outputData);
				WriteBoneData(root, outputData);
				break;
			}
		case aNODETYPE_DUMMY:
			{
				WriteNodeData(fileId, root, outputData);
				break;
			}
		case aNODETYPE_CAMERA:
			{
				WriteNodeData(fileId, root, outputData);
				WriteCameraData(root, outputData);
				break;
			}
		case aNODETYPE_DECAL:
			{
				break;
			}
		case aNODETYPE_ENTITY:
			{
				WriteNodeData(fileId, root, outputData);
				WriteEntityData(root, pDevice->GetRenderer(), saveData, outputData);
				break;
			}
		case aNODETYPE_LIGHT:
			{
				WriteNodeData(fileId, root, outputData);
				WriteLightData(root, outputData);
				break;
			}
		case aNODETYPE_PARTICLE_EMITTER:
			{
				WriteNodeData(fileId, root, outputData);
				WriteParticleData(root, pDevice->GetRenderer(), saveData, outputData);
				break;
			}
		default:
			{
				int8 buffer[256];
				mash::helpers::PrintToBuffer(buffer, 256, "unknown scene node type '%i'.", nodeType);
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashSceneWriter::_SaveScene");
			}
		};
	}

	void CMashSceneWriter::FillSceneNodeMapFromTree(MashSceneNode *root, sFileOutputData &outputData)
	{
		outputData.sceneNodeMap.insert(std::make_pair(root, outputData.nextFileID++));

		MashList<MashSceneNode*>::ConstIterator iter = root->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator iterEnd = root->GetChildren().End();
		for(; iter != iterEnd; ++iter)
			FillSceneNodeMapFromTree(*iter, outputData);
	}

	eMASH_STATUS CMashSceneWriter::SaveScene(MashDevice *pDevice, const MashStringc &filename, const MashList<mash::MashSceneNode*> &rootNodes, const sSaveSceneSettings &saveData)
	{
		sFileOutputData outputData(this);

		MashList<MashSceneNode*>::ConstIterator rootNodesIter = rootNodes.Begin();
		MashList<MashSceneNode*>::ConstIterator rootNodesIterEnd = rootNodes.End();
		for(; rootNodesIter != rootNodesIterEnd; ++rootNodesIter)
			FillSceneNodeMapFromTree(*rootNodesIter, outputData);

		std::map<MashSceneNode*, int32>::iterator iter = outputData.sceneNodeMap.begin();
		std::map<MashSceneNode*, int32>::iterator iterEnd = outputData.sceneNodeMap.end();
		for(; iter != iterEnd; ++iter)
			WriteSceneNode(iter->second, pDevice, iter->first, outputData, saveData);
			
		return _SaveScene(pDevice, filename, outputData, saveData);
	}

	eMASH_STATUS CMashSceneWriter::_SaveScene(MashDevice *pDevice, const MashStringc &filename, sFileOutputData &outputData, const sSaveSceneSettings &saveData)
	{
		sFileHeader fileHeader;
		fileHeader.version = 0.0f;
		fileHeader.stringCount = outputData.stringMap.size();
		fileHeader.modelCount = outputData.modelMap.size();
		fileHeader.sceneNodeCount = outputData.sceneNodeMap.size();
		fileHeader.animationBufferCount = outputData.animationBufferFileIds.size();
		fileHeader.animationMixerCount = outputData.animationMixerFileIds.size();
		fileHeader.vertexCount = outputData.vertexFileIds.size();
		fileHeader.blendStateCount = outputData.blendStateFileIds.size();
		fileHeader.rasterizerStateCount = outputData.rasterizerStateFileIds.size();
		fileHeader.samplerStateCount = outputData.sampelrStateFileIds.size();
		fileHeader.skinCount = outputData.skinMap.size();
		fileHeader.triangleBufferCount = outputData.triangleBufferMap.size();
		fileHeader.triangleColliderCount = outputData.triangleColliderMap.size();

		MashFileStream *pWriter = pDevice->GetFileManager()->CreateFileStream();
		
		pWriter->AppendToStream(&fileHeader, sizeof(sFileHeader));

		//string map
		std::map<MashStringc, int32>::iterator stringIter = outputData.stringMap.begin();
		std::map<MashStringc, int32>::iterator stringIterEnd = outputData.stringMap.end();
		for(; stringIter != stringIterEnd; ++stringIter)
		{
			//file id
			pWriter->AppendToStream(&stringIter->second, sizeof(int32));
			//string length
			int32 stringLength = stringIter->first.Size() + 1;
			pWriter->AppendToStream(&stringLength, sizeof(int32));
			//string
			pWriter->AppendToStream(stringIter->first.GetCString(), stringIter->first.Size() + 1);
		}
		
		pWriter->AppendToStream(outputData.rasterizerStatesData.Pointer(), outputData.rasterizerStatesData.GetCurrentSize());
		pWriter->AppendToStream(outputData.blendStatesData.Pointer(), outputData.blendStatesData.GetCurrentSize());
		pWriter->AppendToStream(outputData.samplerStatesData.Pointer(), outputData.samplerStatesData.GetCurrentSize());
		pWriter->AppendToStream(outputData.vertexData.Pointer(), outputData.vertexData.GetCurrentSize());
		pWriter->AppendToStream(outputData.triangleBufferData.Pointer(), outputData.triangleBufferData.GetCurrentSize());
		pWriter->AppendToStream(outputData.triangleColliderData.Pointer(), outputData.triangleColliderData.GetCurrentSize());
		pWriter->AppendToStream(outputData.modelData.Pointer(), outputData.modelData.GetCurrentSize());
		pWriter->AppendToStream(outputData.skeletonData.Pointer(), outputData.skeletonData.GetCurrentSize());
		pWriter->AppendToStream(outputData.animationBufferData.Pointer(), outputData.animationBufferData.GetCurrentSize());
		pWriter->AppendToStream(outputData.animationMixerData.Pointer(), outputData.animationMixerData.GetCurrentSize());
		pWriter->AppendToStream(outputData.sceneNodeData.Pointer(), outputData.sceneNodeData.GetCurrentSize());

		pWriter->SaveFile(filename.GetCString(), aFILE_IO_BINARY);
		
		pWriter->Destroy();
		
		return aMASH_OK;
	}
}