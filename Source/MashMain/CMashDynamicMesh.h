//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_DYNAMIC_MESH_H_
#define _C_MASH_DYNAMIC_MESH_H_

#include "MashDynamicMesh.h"
#include "MashString.h"
#include "MashAABB.h"
#include "MashGenericArray.h"

namespace mash
{
	class MashVideo;
	class MashSceneManager;

	class CMashDynamicMesh : public MashDynamicMesh
	{
	protected:
		MashSceneManager *m_pSceneManager;
		MashVideo *m_pRenderer;
		uint32 m_iVertexCount;
		uint32 m_iIndexCount;
		eFORMAT m_eIndexFormat;
		uint32 m_iPrimitiveCount;
		ePRIMITIVE_TYPE m_ePrimitiveType;
		mash::MashAABB m_boundingBox;

		MashMeshBuffer *m_meshBuffer;
		MashGenericArray m_rawVertices;
		MashGenericArray m_rawIndices;

		eMASH_STATUS LockBuffersWrite(eBUFFER_LOCK eFlag, void **pVertexBufferOut, void **pIndexBufferOut);
		eMASH_STATUS UnLockBuffers();

		uint8 m_saveInitialiseDataFlags;

		uint32 m_iTotalVertexBufferSizeInBytes;
		uint32 m_iTotalIndexBufferSizeInBytes;
		uint16 m_iIndexFormatSize;

		mash::MashVector4 *m_boneVertexWeights;
		mash::MashVector4 *m_boneVertexIndices;

		MashTriangleBuffer *m_triangleBuffer;
		bool m_bIsLocked;
		bool m_bIsInitialised;
	public:
		CMashDynamicMesh(MashVideo *pRenderer, MashSceneManager *pSceneManager);
		virtual ~CMashDynamicMesh();

		MashMesh* Clone()const;
		MashVertex* GetVertexDeclaration()const;
		uint32 GetVertexCount()const;
		uint32 GetIndexCount()const;
		uint32 GetPrimitiveCount()const;
		ePRIMITIVE_TYPE GetPrimitiveType()const;
		const mash::MashAABB& GetBoundingBox()const;
		eFORMAT GetIndexFormat()const;

		void SetBoneWeights(const mash::MashVector4 *weights, uint32 count);
		void SetBoneIndices(const mash::MashVector4 *indices, uint32 count);

		const mash::MashVector4* GetBoneIndices()const;
		const mash::MashVector4* GetBoneWeights()const;

		MashTriangleBuffer* GetTriangleBuffer()const;
		void SetTriangleBuffer(MashTriangleBuffer *buffer);

		eMASH_STATUS SetGeometry(const void *pVertexBuffer,
				uint32 vertexCount,
				const MashVertex *pVertexType,
				const void *pIndexBuffer,
				uint32 indexCount,
				eFORMAT eIndexFormat,
				ePRIMITIVE_TYPE ePrimitiveType,
				uint32 primitiveCount,
				bool calculateBoundingBox);

		eMASH_STATUS AppendGeometry(const void *pVertexBuffer,
				uint32 vertexCount,
				const MashVertex *pVertexType,
				const void *pIndexBuffer,
				eFORMAT eIndexFormat,
				uint32 indexCount,
				ePRIMITIVE_TYPE ePrimitiveType,
				uint32 primitiveCount,
				bool calculateBoundingBox);

		eMASH_STATUS SetReservedBufferSize(uint32 iVertexBufferSizeInBytes, uint32 iIndexBufferSizeInBytes);

		void ClearGeometry();

		void SetSaveInitialiseDataFlags(uint8 flags);
		uint8 GetSaveInitialiseDataFlags()const;
		void DeleteInitialiseData();

		MashMeshBuffer* GetMeshBuffer()const;

		const MashGenericArray& GetRawVertices()const;
		const MashGenericArray& GetRawIndices()const;

		void SetBoundingBox(const mash::MashAABB &boundingBox);
		void _SetVertexDeclaration(MashVertex *vertex);
	};

	inline uint8 CMashDynamicMesh::GetSaveInitialiseDataFlags()const
	{
		return m_saveInitialiseDataFlags;
	}

	inline void CMashDynamicMesh::SetSaveInitialiseDataFlags(uint8 flags)
	{
		m_saveInitialiseDataFlags = flags;
	}

	inline MashTriangleBuffer* CMashDynamicMesh::GetTriangleBuffer()const
	{
		return m_triangleBuffer;
	}

	inline void CMashDynamicMesh::SetBoundingBox(const mash::MashAABB &boundingBox) 
	{
		m_boundingBox = boundingBox;
	}

	inline const mash::MashVector4* CMashDynamicMesh::GetBoneIndices()const
	{
		return m_boneVertexIndices;
	}

	inline const mash::MashVector4* CMashDynamicMesh::GetBoneWeights()const
	{
		return m_boneVertexWeights;
	}

	inline const MashGenericArray& CMashDynamicMesh::GetRawVertices()const
	{
		return m_rawVertices;
	}

	inline const MashGenericArray& CMashDynamicMesh::GetRawIndices()const
	{
		return m_rawIndices;
	}

	inline eFORMAT CMashDynamicMesh::GetIndexFormat()const
	{
		return m_eIndexFormat;
	}

	inline MashMeshBuffer* CMashDynamicMesh::GetMeshBuffer()const
	{
		return m_meshBuffer;
	}

	inline uint32 CMashDynamicMesh::GetVertexCount()const
	{
		return m_iVertexCount;
	}

	inline uint32 CMashDynamicMesh::GetIndexCount()const
	{
		return m_iIndexCount;
	}

	inline uint32 CMashDynamicMesh::GetPrimitiveCount()const
	{
		return m_iPrimitiveCount;
	}

	inline ePRIMITIVE_TYPE CMashDynamicMesh::GetPrimitiveType()const
	{
		return m_ePrimitiveType;
	}

	inline const mash::MashAABB& CMashDynamicMesh::GetBoundingBox()const
	{
		return m_boundingBox;
	}
}

#endif