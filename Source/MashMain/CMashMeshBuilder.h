//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_MESH_BUILDER_H_
#define _C_MASH_MESH_BUILDER_H_

#include "MashCompileSettings.h"
#include "MashTypes.h"
#include "MashEnum.h"
#include "MashLog.h"
#include "MashVertex.h"
#include "MashMeshBuilder.h"
#include "CMashIndexingHashTable.h"

namespace mash
{
	class MashMesh;
	class MashModel;
	class MashVector3;
	class MashMatrix4;
	class MashAABB;
	class MashVideo;
	class MashVideo;
	class MashSkin;
	class MashGenericArray;

    /*
		This is a basterdised singleton class. Initialise() must be called before using.
	*/
	class CMashMeshBuilder : public MashMeshBuilder
	{
	private:
        struct sPosValue
		{
			sPosValue *next;
			uint32 index;
            
			sPosValue():index(mash::math::MaxUInt32()), next(0){}
			~sPosValue()
			{
				if (next)
				{
					MASH_DELETE_T(sPosValue,  next);
					next = 0;
				}
			}
		};

		mash::MashVideo *m_pRenderDevice;

		void GenerateIndicesIfNeeded(const uint8 *indices, 
			uint32 indexCount,
			eFORMAT indexFormat,
			uint32 vertexCount,
			MashArray<uint32> &indicesOut)const;

		void GenerateNormalsIfNeeded(const mash::MashGenericArray &vertices,
			uint32 vertexCount,
			const MashArray<uint32> &indices,
			const MashVertex *vertex,
			MashArray<mash::MashVector3> &normalsOut)const;

		bool GetVertexElementData(const mash::MashVertex *vertex, eVERTEX_DECLUSAGE elm, uint32 maxSize, uint32 &location, uint32 &size)const;

        //slow function
		void WeldVertices(const MashVertexMeshConversion::sMashVertexMeshConversion *vertices,
			const mash::MashVector4 *boneWeights,
			const mash::MashVector4 *boneIndices,
			uint32 vertexCount, 
			const MashArray<uint32> &indices,
			f32 smoothNormalMergeTolerance,
			MashVertexMeshConversion::sMashVertexMeshConversion *mergedVerticesOut,
			mash::MashVector4 *mergedBoneWeightsOut,
			mash::MashVector4 *mergedBoneIndicesOut,
			uint32 &mergedVertexCountOut,
			MashArray<uint32> &mergedIndicesOut)const;

		void ExtractUniquePositions(const mash::MashGenericArray &vertices,
			uint32 vertexCount, 
			const MashArray<uint32> &indices,
			const MashVertex *vertex,
			MashArray<mash::MashVector3> &positionsOut,
			MashArray<uint32> &indicesOut)const;

		void UpdateNormals(const uint8 *vertices,
			uint32 vertexCount,
			const MashArray<uint32> &indices,
			const MashVertex *vertex)const;

		void UpdateTangents(const uint8 *vertices,
			uint32 vertexCount,
			const MashArray<uint32> &indices,
			const MashVertex *vertex)const;

		eMASH_STATUS ChangeVertexFormat(const uint8 *vertices, 
			uint32 vertexCount,
			const mash::MashVector4 *boneWeights,
			const mash::MashVector4 *boneIndices,
			const sMashVertexElement *currentVertexElements,
			uint32 currentVertexElementCount,
			const MashVertex *pToVertx,
			uint32 &flagsOut,
			uint8 **generatedVertices)const;

		static uint32 PositionHashingFunction(const mash::MashVector3 &item);
		static uint32 MeshConvHashingFunction(const MashVertexMeshConversion::sMashVertexMeshConversion &item);

		struct sMeshVertCmp
		{
			bool operator()(const MashVertexMeshConversion::sMashVertexMeshConversion &a, 
				const MashVertexMeshConversion::sMashVertexMeshConversion &b)
			{
				return (a == b);
			}

			sMeshVertCmp(){}
		};

	public:

		CMashMeshBuilder(mash::MashVideo *pRenderDevice);
		virtual ~CMashMeshBuilder(){}

	public:

		virtual eMASH_STATUS CreateCube(MashMesh *mesh, 
			uint32 iWidth, 
			uint32 iHeight, 
			uint32 iDepth, 
			MashVertex *pVertexFormat,
			const mash::MashVector3 &facing = mash::MashVector3(0.0f, 1.0f, 0.0f),
			const mash::MashVector3 &translation = mash::MashVector3(0.0f, 0.0f, 0.0f));

		virtual eMASH_STATUS CreateSphere(MashMesh *mesh, 
			f32 fRadius, 
			uint32 iTessellateLevel, 
			MashVertex *pVertexFormat,
			const mash::MashVector3 &translation = mash::MashVector3(0.0f, 0.0f, 0.0f));

		virtual eMASH_STATUS CreatePlane(MashMesh *mesh,
			uint32 iTessellateLevel,
			uint32 iWidth,
			uint32 iHeight,
			MashVertex *pVertexFormat,
			const mash::MashVector3 &facing = mash::MashVector3(0.0f, 1.0f, 0.0f),
			const mash::MashVector3 &translation = mash::MashVector3(0.0f, 0.0f, 0.0f));

		eMASH_STATUS UpdateMesh(MashMesh *mesh, uint32 flags, MashVertex *toVertex = 0, f32 weldVertexNormalTolerance = mash::math::MaxFloat())const;
		eMASH_STATUS UpdateMeshEx(MashMesh *destinationMesh, sMesh *mesh, MashVertex *toVertex, uint32 flags, f32 weldVertexNormalTolerance = mash::math::MaxFloat())const;

		eMASH_STATUS CalculateBoundingBox(const MashVertex *pVertexDecl, 
			const void *pVertexList,
			uint32 iVertexCount,
			mash::MashAABB &boundingBox)const;
	};
}

#endif