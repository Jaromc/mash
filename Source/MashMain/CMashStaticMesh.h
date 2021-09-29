//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_STATIC_MESH_H_
#define _C_MASH_STATIC_MESH_H_

#include "MashStaticMesh.h"
#include "MashAABB.h"
#include "MashGenericArray.h"

namespace mash
{
	class MashVideo;
	class MashSceneManager;

	class CMashStaticMesh : public MashStaticMesh
	{
	protected:

		/*
			TODO : Add varialbies to hold the raw vertex and index data. This
			will be needed when serializing and only in editors. In game mode
			these buffers shouuld be empty. However, this behaviour can be
			changed by a simple define at engine creation.
		*/

		MashSceneManager *m_pSceneManager;
		MashVideo *m_pRenderer;
		int32 m_iVertexCount;
		int32 m_iIndexCount;
		eFORMAT m_eIndexFormat;
		uint32 m_iPrimitiveCount;
		ePRIMITIVE_TYPE m_ePrimitiveType;
		mash::MashAABB m_boundingBox;

		uint8 m_saveInitialiseDataFlags;
		mash::MashVector4 *m_boneVertexWeights;
		mash::MashVector4 *m_boneVertexIndices;

		MashMeshBuffer *m_meshBuffer;

		MashGenericArray m_rawVertices;
		MashGenericArray m_rawIndices;

		MashTriangleBuffer *m_triangleBuffer;

		//deletes all buffers from memory
		void ClearBuffers();
		//void DeleteRawData();
	public:
		CMashStaticMesh(MashVideo *pRenderer, MashSceneManager *pSceneManager);
		virtual ~CMashStaticMesh();

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

		MashMeshBuffer* GetMeshBuffer()const;

		const MashGenericArray& GetRawVertices()const;
		const MashGenericArray& GetRawIndices()const;

		void SetSaveInitialiseDataFlags(uint8 flags);
		uint8 GetSaveInitialiseDataFlags()const;
		void DeleteInitialiseData();
		void SetBoundingBox(const mash::MashAABB &boundingBox);
		void _SetVertexDeclaration(MashVertex *vertex);
	};

	inline uint8 CMashStaticMesh::GetSaveInitialiseDataFlags()const
	{
		return m_saveInitialiseDataFlags;
	}

	inline void CMashStaticMesh::SetSaveInitialiseDataFlags(uint8 flags)
	{
		m_saveInitialiseDataFlags = flags;
	}

	inline MashTriangleBuffer* CMashStaticMesh::GetTriangleBuffer()const
	{
		return m_triangleBuffer;
	}

	inline void CMashStaticMesh::SetBoundingBox(const mash::MashAABB &boundingBox) 
	{
		m_boundingBox = boundingBox;
	}

	inline const MashGenericArray& CMashStaticMesh::GetRawVertices()const
	{
		return m_rawVertices;
	}

	inline const MashGenericArray& CMashStaticMesh::GetRawIndices()const
	{
		return m_rawIndices;
	}

	inline const mash::MashVector4* CMashStaticMesh::GetBoneIndices()const
	{
		return m_boneVertexIndices;
	}

	inline const mash::MashVector4* CMashStaticMesh::GetBoneWeights()const
	{
		return m_boneVertexWeights;
	}

	inline eFORMAT CMashStaticMesh::GetIndexFormat()const
	{
		return m_eIndexFormat;
	}

	inline MashMeshBuffer* CMashStaticMesh::GetMeshBuffer()const
	{
		return m_meshBuffer;
	}

	inline uint32 CMashStaticMesh::GetVertexCount()const
	{
		return m_iVertexCount;
	}

	inline uint32 CMashStaticMesh::GetIndexCount()const
	{
		return m_iIndexCount;
	}

	inline uint32 CMashStaticMesh::GetPrimitiveCount()const
	{
		return m_iPrimitiveCount;
	}

	inline ePRIMITIVE_TYPE CMashStaticMesh::GetPrimitiveType()const
	{
		return m_ePrimitiveType;
	}

	inline const mash::MashAABB& CMashStaticMesh::GetBoundingBox()const
	{
		return m_boundingBox;
	}
}

#endif