//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_TRIANGLE_BUFFER_H_
#define _C_MASH_TRIANGLE_BUFFER_H_

#include "MashTriangleBuffer.h"
#include "MashVertex.h"
#include <map>
namespace mash
{
	class CMashTriangleBuffer : public MashTriangleBuffer
	{
		struct sIndexingStruct
		{
			MashVector3 vec;
			uint32 index;

			bool operator < (const sIndexingStruct &other)
			{
				return (vec.x + vec.y + vec.z) < (other.vec.x + other.vec.y + other.vec.z);
			}

			bool operator == (const sIndexingStruct &other)
			{
				return (vec == other.vec);
			}
		};

	private:
		MashArray<sTriangleRecord> m_triangles;
		MashArray<sTriangleSkinnngRecord> m_triangleSkinningData;
		MashArray<mash::MashVector3> m_vertexList;
		MashArray<mash::MashVector3> m_normalList;
		MashArray<uint32> m_indexList;
		MashArray<uint32> m_normalIndexList;

		void GenerateAdjacencyList();
	public:
		CMashTriangleBuffer():MashTriangleBuffer(){}
		virtual ~CMashTriangleBuffer(){}

		eMASH_STATUS Set(const void *pVertexBuffer,
			uint32 iNumVertices,
			const MashVertex *pVertexType,
			const void *pIndexBuffer,
			uint32 iNumIndices,
			eFORMAT eIndexFormat);

		const MashArray<mash::MashVector3>& GetVertexList()const;
		const MashArray<uint32>& GetIndexList()const;
		const MashArray<sTriangleRecord>& GetTriangleList()const;
		const MashArray<sTriangleSkinnngRecord>& GetTriangleSkinningList()const;
		const MashArray<mash::MashVector3>& GetNormalList()const;
		const MashArray<uint32>& GetNormalIndexList()const;

		uint32 GetTriangleCount()const;
		const mash::MashVector3& GetPoint(uint32 triangle, uint32 point)const;
		const mash::MashVector3& GetNormal(uint32 triangle, uint32 point)const;
		uint32 GetNormalIndex(uint32 triangle, uint32 point)const;
		void EstimateSmoothNormal(uint32 triangle, uint32 point, MashVector3 &out)const;
		uint32 GetIndex(uint32 triangle, uint32 point)const;

		void Set(uint32 uniquePointCount, const mash::MashVector3 *uniquePoints,
			uint32 indexCount, uint32 *indexList,
			uint32 normalCount, const mash::MashVector3 *normalList,
			uint32 normalIndexCount, uint32 *normalIndexList,
			const sTriangleRecord *triangleRecordList, const sTriangleSkinnngRecord *triangleSkinningRecordList);
	};

	inline const MashArray<uint32>& CMashTriangleBuffer::GetNormalIndexList()const
	{
		return m_normalIndexList;
	}

	inline const MashArray<mash::MashVector3>& CMashTriangleBuffer::GetNormalList()const
	{
		return m_normalList;
	}

	inline uint32 CMashTriangleBuffer::GetTriangleCount()const 
	{
		return m_triangles.Size();
	}

	inline const MashArray<mash::MashVector3>& CMashTriangleBuffer::GetVertexList()const
	{
		return m_vertexList;
	}

	inline const MashArray<uint32>& CMashTriangleBuffer::GetIndexList()const
	{
		return m_indexList;	
	}

	inline const MashArray<sTriangleRecord>& CMashTriangleBuffer::GetTriangleList()const
	{
		return m_triangles;
	}

	inline const MashArray<sTriangleSkinnngRecord>& CMashTriangleBuffer::GetTriangleSkinningList()const
	{
		return m_triangleSkinningData;
	}
}

#endif