//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _MASH_SET_FILE_WRITER_H_
#define _MASH_SET_FILE_WRITER_H_

#include "CMashSceneLoader.h"
#include "MashGenericArray.h"
#include "MashAnimationMixer.h"
#include "MashMaterial.h"
#include "MashSkin.h"
#include "MashAnimationBuffer.h"
#include "MashTechniqueInstance.h"
#include "MashList.h"
namespace mash
{
	class CMashSceneWriter : public CMashSceneLoader
	{
	private:

		struct sFileOutputData
		{
		private:
			CMashSceneWriter *writer;
		public:
			std::map<MashStringc, int32> stringMap;
			MashGenericArray modelData;
			MashGenericArray skeletonData;
			MashGenericArray sceneNodeData;
			MashGenericArray animationBufferData;
			MashGenericArray animationMixerData;
			MashGenericArray samplerStatesData;
			MashGenericArray rasterizerStatesData;
			MashGenericArray blendStatesData;
			MashGenericArray vertexData;
			MashGenericArray triangleBufferData;
			MashGenericArray triangleColliderData;

			int32 nextFileID;

			//scene nodes need to be preprocessed
			std::map<MashSceneNode*, int32> sceneNodeMap;

			std::map<MashModel*, int32> modelMap;
			std::map<MashSkin*, int32> skinMap;
			std::map<MashAnimationBuffer*, int32> animationBufferFileIds;
			std::map<MashAnimationMixer*, int32> animationMixerFileIds;
			std::map<MashVertex*, int32> vertexFileIds;

			std::map<MashTextureState*, int32> sampelrStateFileIds;
			std::map<int32, int32> rasterizerStateFileIds;
			std::map<int32, int32> blendStateFileIds;
			std::map<MashTriangleBuffer*, int32> triangleBufferMap;
			std::map<MashTriangleCollider*, int32> triangleColliderMap;

			int32 GetSceneNodeFileID(MashSceneNode *node);
			int32 MapString(const int8 *s);
			int32 MapAnimationBuffer(MashAnimationBuffer *buffer);
			int32 MapAnimationMixer(MashAnimationMixer *mixer);
			int32 MapSkin(MashSkin *skin);
			int32 MapModel(MashModel *model);
			int32 MapSamplerState(MashTextureState *state);
			int32 MapRasterizerState(int32 state, MashVideo *renderer);
			int32 MapBlendState(int32 state, MashVideo *renderer);
			int32 MapVertex(MashVertex *vertex);
			int32 MapTriangleBuffer(MashTriangleBuffer *triangleBuffer);
			int32 MapTriangleCollider(MashTriangleCollider *triangleCollider);

			sFileOutputData(CMashSceneWriter *_writer):nextFileID(0), writer(_writer){}
		};

		eMASH_STATUS _SaveScene(MashDevice *pDevice, 
			const MashStringc &filename,
			sFileOutputData &outputData, 
			const sSaveSceneSettings &saveData);

		
		void WriteTriangleCollider(int32 fileID, MashTriangleCollider *triangleCollider, sFileOutputData &outputData);
		void WriteTriangleBuffer(int32 fileID, MashTriangleBuffer *triangleBuffer, sFileOutputData &outputData);
		void WriteAnimationMixer(int32 fileID, MashAnimationMixer *mixer, sFileOutputData &outputData);
		void WriteAnimationBuffer(int32 fileID, MashAnimationBuffer *buffer, sFileOutputData &outputData);
		void WriteNodeData(int32 fileId, MashSceneNode *node, sFileOutputData &outputData);
		void WriteCameraData(MashSceneNode *node, sFileOutputData &outputData);
		void WriteParticleData(MashSceneNode *node, mash::MashVideo *renderer, const sSaveSceneSettings &saveData, sFileOutputData &outputData);
		void WriteLightData(MashSceneNode *node, sFileOutputData &outputData);
		void WriteEntityData(MashSceneNode *node, mash::MashVideo *renderer, const sSaveSceneSettings &saveData, sFileOutputData &outputData);
		void WriteBoneData(mash::MashSceneNode *node, sFileOutputData &outputData);
		void WriteSamplerState(int32 fileID, MashTextureState *state, sFileOutputData &outputData);
		void WriteRasterizerState(int32 fileID, int32 state, MashVideo *renderer, sFileOutputData &outputData);
		void WriteBlendState(int32 fileID, int32 state, MashVideo *renderer, sFileOutputData &outputData);
		void WriteVertex(int32 fileID, MashVertex *vertex, sFileOutputData &outputData);
		void WriteModel(int32 fileID, MashModel *model, sFileOutputData &outputData);
		void WriteSkin(int32 fileID, MashSkin *skin, sFileOutputData &outputData);

		void WriteSceneNode(int32 fileId, MashDevice *pDevice, MashSceneNode *root, sFileOutputData &outputData, const sSaveSceneSettings &saveData);

		void FillSceneNodeMapFromTree(MashSceneNode *root, sFileOutputData &outputData);

	public:
		CMashSceneWriter(){}
		~CMashSceneWriter(){}

		eMASH_STATUS SaveScene(MashDevice *pDevice, const MashStringc &filename, const MashList<mash::MashSceneNode*> &rootNodes, const sSaveSceneSettings &saveData);
	};
}

#endif