//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _MASH_SET_FILE_STREAM_H_
#define _MASH_SET_FILE_STREAM_H_

#include "MashCompileSettings.h"
#include "MashTypes.h"
#include "MashMatrix4.h"
#include "MashAnimationKey.h"
#include "MashTechnique.h"
#include "MashAnimationMixer.h"
#include "MashAnimationBuffer.h"
#include "MashTriangleBuffer.h"
#include "MashTriangleCollider.h"
#include <map>
#include "MashArray.h"
#include "MashString.h"
#include "MashList.h"
#include "CMashSTLMapAllocator.h"
#include "MashMemoryPool.h"

namespace mash
{
	class MashDevice;
	class MashFileManager;
	
	class MashBone;
	class MashEffect;
	class MashEffectCollection;
	class MashMaterial;
	class MashSkin;

	class MashSceneNode;
	class MashModel;

	const uint32 iMAX_STRING_LENGTH = 256;

	/*
		This class can be created without factory.
	*/	
	class _MASH_EXPORT CMashSceneLoader
	{
	private:
		struct sVertexData
		{
			int32 elementCount;
			sMashVertexElement *elements;
		};
		
		struct sAnimationSet
		{
			int32 animationNameFileId;
			int32 blendMode;
			f32 speed; 
			f32 weight;
			int32 wrapMode;
			int32 track;
			int32 fps;
		};

		struct sAnimationMixerStatic
		{
			int32 fileId;
			int32 animationSetCount;
		};

		struct sAnimationMixer
		{
			sAnimationMixerStatic staticData;
			sAnimationSet *animationSets;
			int32 affectedNodeCount;
			int32 *affectNodeFileIds;

			MashAnimationMixer *engineAnimationMixer;
		};

		
	public:
		enum eOBJECT_TYPE
		{
			OBTYPE_MESH,
			OBTYPE_CAMERA,
			OBTYPE_LIGHT,
			OBTYPE_DUMMY,
			OBTYPE_CTARGET,
			OBTYPE_LTARGET,
			OBTYPE_DIRLIGHT
		};

		/*
			TODO : Add these in. Max should save these out.
		*/
		struct sVertexInfluenceData
		{
			int32 boneID;
			f32 weight;
		};
		struct sVertexInfluence
		{
			int32 influenceCount;
			sVertexInfluenceData *data;
		};
		////////////////////////////////////////////////

		struct sFileHeader
		{
			f32 version;
			uint32 stringCount;
			uint32 triangleBufferCount;
			uint32 modelCount;
			uint32 sceneNodeCount;
			uint32 skinCount;
			uint32 animationBufferCount;
			uint32 animationMixerCount;
			uint32 vertexCount;
			uint32 blendStateCount;
			uint32 rasterizerStateCount;
			uint32 samplerStateCount;
			uint32 triangleColliderCount;
		};
		
		struct sDataInformation
		{
			int32 nameStringId;
			int32 startLocation;
			int32 endLocation;
		};

		struct sSceneNodeHeader
		{
			int8 sceneNodeType;
			int8 cameraType;
		};

		struct sAnimationStatic
		{
			int32 nameStringId;
			int32 controllerCount;
		};

		struct sAnimationController
		{
			int32 controllerType;
			int32 keyCount;

			union
			{
				sMashAnimationKeyTransform *transformKey;
			};
		};

		struct sAnimation
		{
			sAnimationStatic staticData;
			sAnimationController *controllers;
		};

        struct sNodeStatic
		{
			int32 fileId;
			int32 nodeNameStringId;
			int32 parentFileId;
			mash::MashVector3 translation;
			mash::MashVector3 scale;
			mash::MashQuaternion rotation;
			int32 animationBufferFileId;
			int32 animationMixerFileId;
			
		};

		struct sNode
		{
			sNodeStatic staticData;
			sAnimation *animations;
		};

		struct sTriangleColliderStatic
		{
			int32 fileId;
			int32 colliderType;
			int32 bufferCount;
		};

		struct sTriangleCollider
		{
			sTriangleColliderStatic staticData;
			int32 *bufferFileIds;
		};

		struct sTriangleBufferStatic
		{
			int32 fileId;
			int32 hasSkinningInfo;
			int32 triangleCount;
			int32 uniqueVertexCount;
			int32 indexCount;
			int32 normalCount;
			int32 normalIndexCount;
		};

		struct sTriangleBuffer
		{
			sTriangleBufferStatic staticData;
			mash::MashVector3 *uniquePoints;
			sTriangleRecord *triangleRecordList;
			sTriangleSkinnngRecord *triangleSkinningRecordList;
			uint32 *indexList;
			MashVector3 *normalList;
			uint32 *normalIndexList;
		};

		struct sMeshStatic
		{
			int32 meshNameStringId;//no longer used.
			int32 vertexStride;
			int32 vertexFileId;
			int32 vertexCount;
			int32 indexCount;
			int32 vertexBoneInfluenceCount;//should be the same as vertexCount
			int32 indexFormat;
			int32 primitiveType;
			int32 primitiveCount;
			mash::MashVector3 boundsMin;
			mash::MashVector3 boundsMax;

			int32 triangleBufferFileId;
		};

		struct sBoneData
		{
			mash::MashMatrix4 bindPose;
			mash::MashVector3 localBindTranslation;
			mash::MashVector3 localBindScale;
			mash::MashQuaternion localBindRotation;
		};

		struct sBone
		{
			sNode nodeData;
			sBoneData boneData;
		};

		struct sSkinBone
		{
			int32 boneFileId;
			int32 boneSkinId;
		};

		struct sSkin
		{
			int32 skinFileId;
			int32 boneCount;
			sSkinBone *boneArray;
		};

		struct sMesh
		{
			sMeshStatic staticData;
			int8 *vertices;
			int8 *indices;
			mash::MashVector4 *vertexBoneWeights;
			mash::MashVector4 *vertexBoneIndices;
		};

		struct sMeshContainer
		{
			int32 subMeshCount;
			sMesh *meshArray;
		};

		struct sModelStatic
		{
			int32 fileId;
			int32 modelNameStringId;

			int32 lodCount;
			int32 triangleColliderFileId;
		};

		struct sModel
		{
			sModelStatic staticData;
			sMeshContainer *meshLodList;
		};

		struct sModelContainer
		{
			sModel *modelData;
			mash::MashModel *engineModel;

			sModelContainer():modelData(0), engineModel(0){}
			sModelContainer(sModel *_modelData):modelData(_modelData), engineModel(0){}
		};

		struct sSkinContainer
		{
			sSkin skinData;
			MashSkin *engineSkin;

			sSkinContainer():engineSkin(0){}
		};

		struct sLightStatic
		{
			int32 type;
			int32 enabled;
			int32 shadowsEnabled;
			int32 rendererType;
			int32 isMainForwardRenderedLight;
			sMashColour4 ambient;
			sMashColour4 diffuse;
			sMashColour4 specular;
			mash::MashVector3 dir;
			f32 atten0;
			f32 atten1;
			f32 atten2;
			f32 range;
			f32 outerCone;
			f32 innerCone;
			f32 falloff;	
		};

		struct sLight
		{
			sNode node;
			sLightStatic lightData;
		};

		struct sParticleStatic
		{
			int32 particleType;
			int32 particleLightingType;
			int32 isPlaying;
			int32 emitterType;
			int32 particleCount;
			int32 particlesPerSecond;
			uint32 minStartColour;
			uint32 minEndColour;
			uint32 maxStartColour;
			uint32 maxEndColour;
			f32 minStartSize;
			f32 maxStartSize;
			f32 minEndSize;
			f32 maxEndSize;
			f32 minRotateSpeed;
			f32 maxRotateSpeed;
			f32 minDuration;
			f32 maxDuration;
			f32 softParticleScale;
			f32 emitterVelocityWeight;
			int32 startTime;
			mash::MashVector3 minVelocity;
			mash::MashVector3 maxVelocity;
			mash::MashVector3 gravity;
			int32 isCustom;
			int32 isMaterialInstanced;
			int32 materialStringFileId;
			int32 diffuseSamplerFileId;
			int32 diffuseTextureFileId;
			int32 modelFileId;
		};

		struct sParticle
		{
			sNode node;
			sParticleStatic particleStatic;
		};

		struct sCameraStatic
		{
			int32 cameraType;
			mash::MashVector3 target;
			
			f32 nearClip;
			f32 farClip;
			f32 fov;
			int32 autoAspect;
			f32 aspect;
			int32 isOrtho;
		};

		struct sCamera
		{
			sNode node;
			sCameraStatic cameraData;	
		};

		struct sSubEntity
		{
			int32 isActive;
			int32 materialNameStringId;
		};

		struct sSubEntityLodData
		{
			int32 meshCount;
			
			//sub entity data
			sSubEntity *subEntities;
		};

		struct sEntity
		{
			sNode node;
			int32 skinFileId;
			int32 modelFileId;
			int32 lodCount;
			int32 lodDistanceCount;
			uint32 *lodDistances;
			sSubEntityLodData *lodMeshList;
		};

        struct sSceneNodeData
		{
			mash::MashSceneNode *node;
			int32 parentFileId;

			sSceneNodeData():node(0),
				parentFileId(-1){}

			sSceneNodeData(mash::MashSceneNode *_node,
				int32 _parentFileId):node(_node),
				parentFileId(_parentFileId){}
		};

		typedef MashMemoryPool<sMashMemoryPoolError, sMashAllocAllocatorFunctor, sMashFreeDeallocatorFunctor> MemPoolType;

		typedef CMashSTLMapAllocator<std::pair<const int32, sSceneNodeData>, MemPoolType> sceneNodeAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, MashStringc>, MemPoolType> stringAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, int32>, MemPoolType> rasterizerStateAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, int32>, MemPoolType> blendStateAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, MashTextureState*>, MemPoolType> samplerStateAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, sVertexData>, MemPoolType> vertexAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, MashAnimationBuffer*>, MemPoolType> animationBufferAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, sAnimationMixer>, MemPoolType> animationMixerAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, sSkinContainer>, MemPoolType> skinAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, sModelContainer>, MemPoolType> modelAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, MashTriangleBuffer*>, MemPoolType> triangleBufferAlloc;
		typedef CMashSTLMapAllocator<std::pair<const int32, MashTriangleCollider*>, MemPoolType> triangleColliderAlloc;

		struct sLoadedData
		{
		public:
			std::map<int32, sSceneNodeData, std::less<int32>, sceneNodeAlloc > sceneNodeMap;
			std::map<int32, MashStringc, std::less<int32>, stringAlloc > stringMap;
			std::map<int32, int32, std::less<int32>, rasterizerStateAlloc > rasterizerStateMap;
			std::map<int32, int32, std::less<int32>, blendStateAlloc > blendStateMap;
			std::map<int32, MashTextureState*, std::less<int32>, samplerStateAlloc > samplerStateMap;
			std::map<int32, sVertexData, std::less<int32>, vertexAlloc > vertexMap;
			std::map<int32, MashAnimationBuffer*, std::less<int32>, animationBufferAlloc > animationBufferMap;
			std::map<int32, sAnimationMixer, std::less<int32>, animationMixerAlloc > animationMixerMap;
			std::map<int32, sSkinContainer, std::less<int32>,skinAlloc > skinMap;
			std::map<int32, sModelContainer, std::less<int32>, modelAlloc > modelMap;
			std::map<int32, MashTriangleBuffer*, std::less<int32>, triangleBufferAlloc > triangleBufferMap;
			std::map<int32, MashTriangleCollider*, std::less<int32>, triangleColliderAlloc > triangleColliderMap;

			
			sLoadedData(CMashSceneLoader::MemPoolType *pool):sceneNodeMap(std::less<int32>(), sceneNodeAlloc(pool)),
				stringMap(std::less<int32>(), stringAlloc(pool)),
				rasterizerStateMap(std::less<int32>(), rasterizerStateAlloc(pool)),
				blendStateMap(std::less<int32>(), blendStateAlloc(pool)),
				samplerStateMap(std::less<int32>(), samplerStateAlloc(pool)),
				vertexMap(std::less<int32>(), vertexAlloc(pool)),
				animationBufferMap(std::less<int32>(), animationBufferAlloc(pool)),
				animationMixerMap(std::less<int32>(), animationMixerAlloc(pool)),
				skinMap(std::less<int32>(), skinAlloc(pool)),
				modelMap(std::less<int32>(), modelAlloc(pool)),
				triangleBufferMap(std::less<int32>(), triangleBufferAlloc(pool)),
				triangleColliderMap(std::less<int32>(), triangleColliderAlloc(pool)){}

			~sLoadedData(){DropAllData();}
			
			void DropAllData();
		};

		MashBone* FindBone(MashArray<MashBone*> &bones, const int8 *sName)const;

		eMASH_STATUS ReadNodeData(sLoadedData &loadedData,
			const uint8 *data, 
			uint32 &currentLocation,
			sNode &node);

		eMASH_STATUS LoadTriangleColliderData(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadTriangleBufferData(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadNodeCommon(MashDevice *pDevice,
			sLoadedData &loadData,
			const sNode *nodeData,
			MashSceneNode *node);

		eMASH_STATUS LoadBone(MashDevice *pDevice,
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS ReadModelData(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadModel(MashDevice *pDevice,
			sLoadedData &loadData,
			sModelContainer *modelContainer,
			const MashArray<MashArray<MashMaterial*> > &entityLodMaterials, 
			const sLoadSceneSettings &loadSettings);

		eMASH_STATUS ReadSkin(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadAnimationBuffer(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS ReadAnimationMixer(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadAnimationMixer(MashDevice *pDevice, 
			sLoadedData &loadData,
			const uint8 *data, 
			uint32 &currentLocation);

		MashModel* ReadModelData(MashDevice *pDevice, const sFileHeader *fileHeader, const uint8 *data, const int8 *modelName, MashEffect *vertexProgram);

		void ReadEntityData(const uint8 *data, uint32 &location, sEntity *entity);

		eMASH_STATUS LoadDummy(MashDevice *pDevice,
			sLoadedData &loadedData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadCamera(MashDevice *pDevice,
			sLoadedData &loadedData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadLight(MashDevice *pDevice,
			sLoadedData &loadedData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadParticle(MashDevice *pDevice,
			sLoadedData &loadedData,
			const uint8 *data, 
			uint32 &currentLocation);

		eMASH_STATUS LoadEntity(MashDevice *pDevice, 
			sLoadedData &loadedData,
			const uint8 *data, 
			uint32 &currentLocation,
			const sLoadSceneSettings &loadSettings);

		MemPoolType m_memoryPool;		
	public:
		CMashSceneLoader();
		~CMashSceneLoader();

		eMASH_STATUS LoadSETFile(MashDevice *pDevice, const int8 *fileName, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings);
		eMASH_STATUS LoadSETFile(MashDevice *pDevice, const int8 *fileName);
	};
}

#endif