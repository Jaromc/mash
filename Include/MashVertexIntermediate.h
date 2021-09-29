//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_VERTEX_INTERMEDIATE_H_
#define _MASH_VERTEX_INTERMEDIATE_H_

#include "MashVertex.h"
#include "MashArray.h"

namespace mash
{
    /*!
        Implimented by API classes. Access these methods through MashVertex.
    */
	class MashVertexIntermediate : public MashVertex
	{
	protected:
		MashVertexIntermediate(const sMashVertexElement *mashVertexDecl,
			uint32 elementCount,
			uint32 vertexSizeInBytes);

		virtual ~MashVertexIntermediate();
	protected:
		sMashVertexElement *m_mashVertexDecl;
		uint32 m_elementCount;
		uint32 m_streamCount;
		MashArray<uint32> m_streamSizes;
	public:

		uint32 GetStreamCount()const;
		uint32 GetStreamSizeInBytes(uint32 iStream)const;

		bool Contains(eVERTEX_DECLUSAGE check, uint32 stream = 0)const;
		bool IsEqual(const sMashVertexElement *vertexDecl, uint32 elementCount, int32 stream = -1)const;
		bool IsEqual(const MashVertex *vertex, int32 stream = -1)const;

		const sMashVertexElement* GetVertexElements()const;
		uint32 GetVertexElementCount()const;	

		eMASH_STATUS GetElementStride(uint32 stream, eVERTEX_DECLUSAGE usage, uint32 &out)const;
		const sMashVertexElement* GetElement(uint32 stream, eVERTEX_DECLUSAGE usage)const;
	};

	inline uint32 MashVertexIntermediate::GetStreamCount()const
	{
		return m_streamCount;
	}

	inline const sMashVertexElement* MashVertexIntermediate::GetVertexElements()const
	{
		return m_mashVertexDecl;
	}

	inline uint32 MashVertexIntermediate::GetVertexElementCount()const
	{
		return m_elementCount;
	}
}

#endif