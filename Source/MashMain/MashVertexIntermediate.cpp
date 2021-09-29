//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashVertexIntermediate.h"
#include "MashHelper.h"
#include "MashDevice.h"
namespace mash
{
	MashVertexIntermediate::MashVertexIntermediate(const sMashVertexElement *pMashVertexDecl,
				uint32 iElementCount,
				uint32 iVertexSizeInBytes):MashVertex(), m_mashVertexDecl(0),
    m_elementCount(iElementCount), m_streamCount(0)
	{
		m_mashVertexDecl = MASH_ALLOC_T_COMMON(sMashVertexElement, iElementCount);
		memcpy(m_mashVertexDecl, pMashVertexDecl, sizeof(sMashVertexElement) * iElementCount);

		if (m_elementCount == 1)
		{
			m_streamCount = 1;
			m_streamSizes.PushBack(mash::helpers::GetVertexDeclTypeSize(m_mashVertexDecl[0].type));
		}
		else if (m_elementCount > 1)
		{
			uint32 iLastStream = 0;
			for(uint32 i = 1; i < m_elementCount; ++i, ++iLastStream)
			{
				if (m_mashVertexDecl[i].stream != m_mashVertexDecl[iLastStream].stream)
				{
					m_streamSizes.PushBack(m_mashVertexDecl[iLastStream].stride + mash::helpers::GetVertexDeclTypeSize(m_mashVertexDecl[iLastStream].type));
					++m_streamCount;
				}
			}

			m_streamSizes.PushBack(m_mashVertexDecl[iLastStream].stride + mash::helpers::GetVertexDeclTypeSize(m_mashVertexDecl[iLastStream].type));
			++m_streamCount;
		}
	}

	MashVertexIntermediate::~MashVertexIntermediate()
	{
		if (m_mashVertexDecl)
		{
			MASH_FREE(m_mashVertexDecl);
		}
	}

	uint32 MashVertexIntermediate::GetStreamSizeInBytes(uint32 iStream)const
	{
		if ((iStream < 0) || (iStream >= m_streamSizes.Size()))
			return 0;

		return m_streamSizes[iStream];
	}

	eMASH_STATUS MashVertexIntermediate::GetElementStride(uint32 iStream, eVERTEX_DECLUSAGE usage, uint32 &out)const
	{
		for(uint32 i = 0; i < m_elementCount; ++i)
		{
			if ((m_mashVertexDecl[i].stream == iStream) && (m_mashVertexDecl[i].usage == usage))
			{
				out = m_mashVertexDecl[i].stride;
				return aMASH_OK;
			}
		}

		return aMASH_FAILED;
	}

	const sMashVertexElement* MashVertexIntermediate::GetElement(uint32 stream, eVERTEX_DECLUSAGE usage)const
	{
		for(uint32 i = 0; i < m_elementCount; ++i)
		{
			if ((m_mashVertexDecl[i].stream == stream) && (m_mashVertexDecl[i].usage == usage))
			{
				return &m_mashVertexDecl[i];
			}
		}

		return 0;
	}

	bool MashVertexIntermediate::Contains(eVERTEX_DECLUSAGE check, uint32 iStream)const
	{
		for(uint32 i = 0; i < m_elementCount; ++i)
		{
			if ((m_mashVertexDecl[i].stream == iStream) && (m_mashVertexDecl[i].usage == check))
				return true;
		}

		return false;
	}

	bool MashVertexIntermediate::IsEqual(const MashVertex *pVertex, int32 stream)const
	{
		return IsEqual(pVertex->GetVertexElements(), pVertex->GetVertexElementCount());
	}

	bool MashVertexIntermediate::IsEqual(const sMashVertexElement *vertexDecl, uint32 iElementsCount, int32 stream)const
	{
		if (!vertexDecl || !iElementsCount)
			return false;

		if (stream == -1)
		{
			if (iElementsCount != m_elementCount)
				return false;

			for(uint32 i = 0; i < m_elementCount; ++i)
			{
				if ((m_mashVertexDecl[i].stream != vertexDecl[i].stream) ||
					(m_mashVertexDecl[i].stride != vertexDecl[i].stride) ||
					(m_mashVertexDecl[i].type != vertexDecl[i].type) ||
					(m_mashVertexDecl[i].usage != vertexDecl[i].usage) ||
					(m_mashVertexDecl[i].usageIndex != vertexDecl[i].usageIndex))
				{
					return false;
				}
			}
		}
		else
		{
			/*
				This isnt too simple unfortunatly.
			*/

			uint32 thisStreamStart, thisStreamEnd;
			uint32 otherStreamStart, otherStreamEnd;
            mash::helpers::GetVertexStreamStartEndIndex(stream, m_mashVertexDecl, m_elementCount, thisStreamStart, thisStreamEnd);
			mash::helpers::GetVertexStreamStartEndIndex(stream, vertexDecl, iElementsCount, otherStreamStart, otherStreamEnd);

			if (thisStreamStart == m_elementCount || otherStreamStart == iElementsCount)
				return false;

			if ((thisStreamEnd - thisStreamStart) != (otherStreamEnd - otherStreamStart))
				return false;

			/*
				At this point we have determined both vertex streams have the
				same number of elements.
			*/
			do
			{
				if ((m_mashVertexDecl[thisStreamStart].stride != vertexDecl[otherStreamStart].stride) ||
					(m_mashVertexDecl[thisStreamStart].type != vertexDecl[otherStreamStart].type) ||
					(m_mashVertexDecl[thisStreamStart].usage != vertexDecl[otherStreamStart].usage) ||
					(m_mashVertexDecl[thisStreamStart].usageIndex != vertexDecl[otherStreamStart].usageIndex))
				{
					return false;
				}
				++thisStreamStart;
				++otherStreamStart;

			}while(thisStreamStart < thisStreamEnd);
		}

		return true;
	}
}