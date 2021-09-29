//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashPrimitiveBatch.h"
#include "MashVertexBuffer.h"
#include "MashSkin.h"
#include "MashMesh.h"
#include "MashVertex.h"
#include "MashVector3.h"
#include "MashMaterial.h"
#include "MashVideo.h"
#include "MashLog.h"

namespace mash
{
	CMashGeometryBatch::CMashGeometryBatch(MashVideo *pRenderer):MashGeometryBatch(),
		m_pRenderer(pRenderer),
		m_iVertexCount(0), m_iPrimitiveType(aPRIMITIVE_TRIANGLE_LIST), 
		m_bInitialised(false), m_iCurrentBufferSizeInBytes(0),
		m_meshBuffer(0), m_pMaterial(0), m_commitNeeded(false)
	{
	
	}

	CMashGeometryBatch::~CMashGeometryBatch()
	{
		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}

		if (m_pMaterial)
		{
			m_pMaterial->Drop();
			m_pMaterial = 0;
		}
	}

	eMASH_STATUS CMashGeometryBatch::Initialise(MashMaterial *pMaterial, ePRIMITIVE_TYPE type, eBATCH_TYPE eBatchType)
	{
		m_pMaterial = pMaterial;
		if (!m_pMaterial)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							"Material pointer is null",
							"CMashGeometryBatch::Initialise");

			return aMASH_FAILED;
		}	

		m_pMaterial->Grab();

		m_iPrimitiveType = type;
		m_eBatchType = eBatchType;
		m_cachedPoints.Clear();

		m_bInitialised = true;

		return aMASH_OK;
	}

	eMASH_STATUS CMashGeometryBatch::AddPoints(const uint8 *pPoints, uint32 iCount)
	{
		m_commitNeeded = true;

        m_cachedPoints.Append(pPoints, iCount * m_pMaterial->GetVertexDeclaration()->GetStreamSizeInBytes(0));

        m_iVertexCount += iCount;

		return aMASH_OK;
	}

	int32 CMashGeometryBatch::GetPrimitiveCount()const
	{
		if (m_iVertexCount == 0)
			return 0;

		switch(m_iPrimitiveType)
		{
		case aPRIMITIVE_LINE_LIST:
			return m_iVertexCount / 2;
		case aPRIMITIVE_POINT_LIST:
			return m_iVertexCount;
		case aPRIMITIVE_TRIANGLE_LIST:
			return m_iVertexCount / 3;
		case aPRIMITIVE_LINE_STRIP:
			return m_iVertexCount - 1;
		case aPRIMITIVE_TRIANGLE_STRIP:
			return math::Max<int32>(0, m_iVertexCount - 2);
		}

		return 0;
	}

	eMASH_STATUS CMashGeometryBatch::Flush()
	{
		if (m_iVertexCount == 0)
			return aMASH_OK;

		if (!m_bInitialised)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							"This batch has not been initialised",
							"CMashGeometryBatch::Flush");

			return aMASH_FAILED;
		}

		if (m_commitNeeded)
		{
			if (!m_meshBuffer)
			{
				sVertexStreamInit streamData;
				streamData.dataSizeInBytes = m_cachedPoints.GetCurrentSize();

				if (m_eBatchType == CMashGeometryBatch::aDYNAMIC)
				{
					streamData.usage = aUSAGE_DYNAMIC;
					streamData.data = m_cachedPoints.Pointer();
				}
				else
				{
					streamData.usage = aUSAGE_STATIC;
					streamData.data = m_cachedPoints.Pointer();
				}

				m_meshBuffer = m_pRenderer->CreateMeshBuffer(&streamData, 1, m_pMaterial->GetVertexDeclaration());

                if (m_eBatchType == CMashGeometryBatch::aSTATIC)
                    m_cachedPoints.DeleteData();

				m_iCurrentBufferSizeInBytes = streamData.dataSizeInBytes;
			}
			else if (m_eBatchType == CMashGeometryBatch::aDYNAMIC)//only update dynamic buffers
			{
				uint32 sizeNeeded = m_cachedPoints.GetCurrentSize();
				if (sizeNeeded > m_iCurrentBufferSizeInBytes)
				{
					if (m_meshBuffer->ResizeVertexBuffers(0, sizeNeeded, aUSAGE_DYNAMIC, false) == aMASH_FAILED)
						return aMASH_FAILED;

					m_iCurrentBufferSizeInBytes = sizeNeeded;
				}

				int8 *vertexPtr = 0;
				if (m_meshBuffer->GetVertexBuffer()->Lock(mash::aLOCK_WRITE_DISCARD, (void**)(&vertexPtr)) == aMASH_FAILED)
				{
					return aMASH_FAILED;
				}

				memcpy(vertexPtr, m_cachedPoints.Pointer(), m_cachedPoints.GetCurrentSize());

				if (m_meshBuffer->GetVertexBuffer()->Unlock() == aMASH_FAILED)
				{
					return aMASH_FAILED;
				}

				m_cachedPoints.Clear();//flush the buffer
			}

			m_commitNeeded = false;
		}

		eMASH_STATUS status = aMASH_OK;

		if (m_pMaterial->OnSet() == aMASH_OK)
		{
			status = m_pRenderer->DrawVertexList(m_meshBuffer, m_iVertexCount,
				GetPrimitiveCount(),
				m_iPrimitiveType);
		}

		if (m_eBatchType == aDYNAMIC)
		{
			m_iVertexCount = 0;
		}

		return status;
	}
}