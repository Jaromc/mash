//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_COLLADA_LOADER_H_
#define _C_MASH_COLLADA_LOADER_H_

#include "MashEnum.h"
#include "MashTypes.h"
#include "MashReferenceCounter.h"
#include "MashArray.h"
#include "MashString.h"
#include "MashList.h"
#include <map>
#include "MashXMLReader.h"
#include "MashMatrix4.h"
#include "MashMemoryPool.h"
#include "CMashSTLMapAllocator.h"

namespace mash
{
	class MashSceneNode;
	class MashMesh;
	class MashModel;

	class MashSkin;
	class MashAnimationBuffer;
	class MashDevice;

	const uint32 g_maxColladaAccessorParams = 16;

	class CMashColladaLoader : public MashReferenceCounter
	{
	private:
		enum eFILE_UP_AXIS
		{
			aFILE_UP_AXIS_X,
			aFILE_UP_AXIS_Y,
			aFILE_UP_AXIS_Z
		};

		enum eARRAY_DATA_SOURCE_TYPE
		{
			aARRAY_DATA_SOURCE_FLOAT,
			aARRAY_DATA_SOURCE_INT,
			aARRAY_DATA_SOURCE_BOOL,
			aARRAY_DATA_SOURCE_NAME
		};

		enum eEXTENDED_INPUT_SEMANTICS
		{
			aEXT_INPUT_SEMANTIC_JOINT = aVERTEX_DECLUSAGE_COUNT,
			aEXT_INPUT_SEMANTIC_INV_BIND_MATRIX,
			aEXT_INPUT_SEMANTIC_WEIGHT,

			aEXT_INPUT_SEMANTIC_COUNT
		};

		enum eANIM_INPUT_SEMANTICS
		{
			aANIM_INPUT_SEMANTIC_INPUT,
			aANIM_INPUT_SEMANTIC_OUTPUT,
			aANIM_INPUT_SEMANTIC_INTERPOLATION,

			aANIM_INPUT_SEMANTIC_COUNT
		};

		enum eCOLLADA_NODE_TYPE
		{
			aCOLLADA_NODE_TYPE_NODE,
			aCOLLADA_NODE_TYPE_JOINT
		};

		enum eANIM_CHANNEL_TARGET
		{
			aANIM_CHANNEL_TRANSLATION_X,
			aANIM_CHANNEL_TRANSLATION_Y,
			aANIM_CHANNEL_TRANSLATION_Z,
			aANIM_CHANNEL_ROTATION_X,
			aANIM_CHANNEL_ROTATION_Y,
			aANIM_CHANNEL_ROTATION_Z,
			aANIM_CHANNEL_SCALE_X,
			aANIM_CHANNEL_SCALE_Y,
			aANIM_CHANNEL_SCALE_Z,
			aANIM_CHANNEL_MATRIX,

			aANIM_CHANNEL_COUNT
		};

		//bit flags
		enum eNAME_FLAGS
		{
			aNAME_FLAG_LOD = 1
		};

		struct sFileData
		{
			eFILE_UP_AXIS upAxis;
		};

		struct sVariableArray
		{
			union
			{
				f32 *f;
				int32 *i;
				bool *b;
				const int8 *s;
			};

			int32 count;
			eARRAY_DATA_SOURCE_TYPE type;
		};

		struct sArrayDataSource
		{
			
			sVariableArray varaibleArray;
			
			/*
				The number of elements per object.
				Eg, 3 elements in a vector3
			*/
			int32 stride;
			/*
				The number of elements to skip before reading
				the first element of the source array.
			*/
			int32 offset;
			/*
				The number of objects to read after the offset.
				Eg, 34 vector3s
			*/
			int32 count;

			/*
				Will be true when this array has been converted into
				the correct co-ordinate system. Only needed for
				positional data
			*/
			bool isAxisConverted;

			const int8 *id;
			/*
				Simple bit value to state if we should skip
				an element. This will be == stride && < g_maxColladaAccessorParams
			*/
			int8 paramValid[g_maxColladaAccessorParams];
		};

		struct sSource
		{
			const int8 *id;
			sArrayDataSource arrayData;
		};
		
		struct sInputData
		{
			uint32 usage;
			int32 indexOffset;
			sSource *sourceArray;//TODO : Change this to an iterator
		};

		struct sAnimSampler
		{
			sSource *inputSource[aANIM_INPUT_SEMANTIC_COUNT];
		};

		struct sAnimChannel
		{
			sAnimSampler *samplers[aANIM_CHANNEL_COUNT];
		};

		/*
			Equivalent to MashMesh
		*/
		struct sSubMesh
		{
			MashList<sInputData> vertexLayout;
			sVariableArray triangleIndices;
			int32 triangleCount;
			const int8 *materialName;

			sSubMesh(){}
			~sSubMesh(){}
		};

		/*
			Equivalent to MashModel
		*/
		struct sMesh
		{
			sSubMesh *meshArray;
			int32 meshCount;
			MashStringc meshName;
			int32 lodIndex;
			const int8 *id;

			sMesh *nextLod;
			sMesh *prevLod;

			MashModel *engineModel;
		};

		struct sSkinController
		{
			const int8 *id;
			const int8 *geomOwner;
			int32 vertexWeightCount;//should be equal to a meshes vertex count
			mash::MashMatrix4 bindShapeMatrix;
			MashList<sInputData> jointData;
			MashList<sInputData> weightData;
			//the number of elements should equal the number of POSITION vectors
			sVariableArray vertexBoneCounts;
			sVariableArray vertexBoneWeightIndices;

			MashSkin *engineSkin;
		};

		struct sNode
		{
			const int8 *id;
			const int8 *nameFromFile;
			MashStringc decodedName;
			const int8 *jointName;
			const int8 *controllerName;//will have '#' at [0]
			const int8 *skeletonRootNodeName;//will have '#' at [0]
			//TODO : Move these into a union
			const int8 *geometryName;//will have '#' at [0]
			const int8 *lightName;//will have '#' at [0]
			const int8 *cameraName;//will have '#' at [0]
			mash::MashMatrix4 localTransform;
			mash::MashMatrix4 geometryOffsetTransform;
			eCOLLADA_NODE_TYPE nodeType;

			mash::MashSceneNode *engineNode;

			//will be set to the end of the node map if its a root node
			sNode *parentNode;
			sNode *nextSibling;
		};

		struct sLightData
		{
			mash::eLIGHTTYPE lightType;
		};
		
		typedef MashMemoryPool<sMashMemoryPoolError, sMashAllocAllocatorFunctor, sMashFreeDeallocatorFunctor> MemPoolType;

		typedef CMashSTLMapAllocator<std::pair<const MashStringc, sLightData>, MemPoolType> lightAlloc;
		typedef CMashSTLMapAllocator<std::pair<const MashStringc, sMesh*>, MemPoolType> meshAlloc;
		typedef CMashSTLMapAllocator<std::pair<const MashStringc, sSource>, MemPoolType> sourceAlloc;
		typedef CMashSTLMapAllocator<std::pair<const MashStringc, sSkinController>, MemPoolType> skinControllerAlloc;
		typedef CMashSTLMapAllocator<std::pair<const MashStringc, sAnimSampler>, MemPoolType> animSampleAlloc;
		typedef CMashSTLMapAllocator<std::pair<const MashStringc, sAnimChannel>, MemPoolType> animChannelAlloc;

		void ReadLibraryLights(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, std::map<MashStringc, sLightData, std::less<MashStringc>, lightAlloc > &lightMap);
		void ReadLibraryGeometry(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, std::map<MashStringc, sMesh*, std::less<MashStringc>, meshAlloc > &modelMap, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap);
		void ReadLibraryControllers(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, MashDevice *device, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, std::map<MashStringc, sSkinController, std::less<MashStringc>, skinControllerAlloc > &skinControllers);
		void ReadLibraryVisualScene(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, MashList<sNode> &nodes);
		void ReadLibraryAnimation(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap, std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc > &animChannelMap);
		void ReadAnimationData(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis,  std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap, std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc > &animChannelMap, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap);
		eMASH_STATUS ReadMeshData(MashXMLReader *xmlNode, sMesh *mesh, const int8 *siblingName, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, MashList<sInputData> &vertexInputs);
		MashAnimationBuffer* CreateAnimationBuffer(MashDevice *device, sAnimChannel *animChannel, sNode *node, uint32 frameRate);
		void GetAnimChannelData(MashXMLReader *xmlNode, eFILE_UP_AXIS upAxis, std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap, std::map<MashStringc, sAnimChannel, std::less<MashStringc>, animChannelAlloc > &animChannelMap);
		void GetAnimSamplerData(MashXMLReader *xmlNode, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, std::map<MashStringc, sAnimSampler, std::less<MashStringc>, animSampleAlloc > &animSamplerMap);
		const int8* RemoveStringHash(const int8 *s);
		eANIM_CHANNEL_TARGET ConvertFloatArrayIfNeeded(eFILE_UP_AXIS upAxis, eANIM_CHANNEL_TARGET currentTarget, sArrayDataSource *arrayData);
		void ConvertVectorArrayAxisIfNeeded(eFILE_UP_AXIS upAxis, sArrayDataSource *arrayData);
		void ConvertTexCoordArrayAxisIfNeeded(sArrayDataSource *arrayData);
		mash::MashModel* CreateModel(MashDevice *device, eFILE_UP_AXIS upAxis, sMesh* model, sSkinController *skinController, const mash::MashMatrix4 &geometryOffsetTransform, const sLoadSceneSettings &loadSettings);
		bool ReadNodeElements(MashXMLReader *xmlReader, eFILE_UP_AXIS upAxis, sNode *newNode, bool geometryOffsetNodeEntered, MashList<sNode> &nodesOut);
		void GetNodeData(MashXMLReader *xmlNode, eFILE_UP_AXIS upAxis, sNode *parentNode, MashList<sNode> &nodesOut);
		void ConvertVector3(eFILE_UP_AXIS upAxis, f32 *out);
		void ConvertTexCoord2(f32 *out);
		void ConvertMatrix(eFILE_UP_AXIS upAxis, mash::MashMatrix4 &mat);
		eMASH_STATUS GetSourceElements(MashXMLReader *xmlNode, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap);
		void GetFloatsFromArray(const int8 *str, f32 *floatAry, uint32 floatCount);
		void GetRotationAxisAngleFromArray(const int8 *str, eFILE_UP_AXIS upAxis, mash::MashMatrix4 &out);
		//assumes the int8 array contains 3 f32 values
		void GetVec3FromArray(const int8 *str, eFILE_UP_AXIS upAxis, f32 *out);
		void GetTexCoord2FromArray(const int8 *str, f32 *out);
		//assumes the int8 array contains 16 f32 values
		void GetMatrixFromArray(const int8 *str, eFILE_UP_AXIS upAxis, mash::MashMatrix4 &out);
		void ConvertTextArrayToElements(const int8 *str, sVariableArray &out, uint32 writeOffset = 0, uint32 *elementsWritten = 0);
		void GetInputSemanticData(MashXMLReader *xmlNode, std::map<MashStringc, sSource, std::less<MashStringc>, sourceAlloc > &sourceMap, MashList<sInputData> &out, MashList<sInputData> *vertexInputs);
		void DecodeName(const int8 *str, MashStringc &nameOut, int32 &bitFlagOut, int32 &flagIdOut);

		MemPoolType m_memoryPool;
	public:
		CMashColladaLoader();
		~CMashColladaLoader(){}

		eMASH_STATUS Load(MashDevice *device, const MashStringc &filename, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings);
	};
}

#endif