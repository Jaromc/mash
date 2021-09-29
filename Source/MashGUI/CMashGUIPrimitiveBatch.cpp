//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIPrimitiveBatch.h"
#include "MashGUIComponent.h"
#include "MashVertex.h"
#include "MashVertexBuffer.h"
#include "MashMaterialManager.h"
#include "MashVideo.h"
#include "MashGUIManager.h"
#include "MashLog.h"
namespace mash
{
	CMashGUIPrimitiveBatch::CMashGUIPrimitiveBatch(MashGUIManager *pGUIManager, 
		MashVideo *pRenderer,
		int32 iMaxVertexCount):
		m_pVertexDeclaration(0),
		m_GUIManager(pGUIManager),
		m_pRenderer(pRenderer),
		m_meshBuffer(0),m_pVertexPtr(0),
		m_iCurrentReservedVertex(6),
		m_iCurrentVertexCount(0),
		m_iMaxVertexCount(iMaxVertexCount),
		m_currentColour(1.0f, 1.0f, 1.0f, 1.0f),
		m_pMaterial(0)
	{
	}

	CMashGUIPrimitiveBatch::~CMashGUIPrimitiveBatch()
	{
		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}

		if (m_pVertexDeclaration)
		{
			m_pVertexDeclaration->Drop();
			m_pVertexDeclaration = 0;
		}

		if (m_pMaterial)
		{
			m_pMaterial->Drop();
			m_pMaterial = 0;
		}
	}

	eMASH_STATUS CMashGUIPrimitiveBatch::Initialise()
	{
		if (m_pMaterial)
			m_pMaterial->Drop();

		m_pMaterial = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GUI_LINE);

		if (!m_pMaterial)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"GUI line material failed to load.", 
						"CMashGUIPrimitiveBatch::Initialise");

			return aMASH_FAILED;
		}

		m_pMaterial->Grab();

		if (m_pVertexDeclaration)
			m_pVertexDeclaration->Drop();

		m_pVertexDeclaration = m_pMaterial->GetVertexDeclaration();

		if (!m_pVertexDeclaration)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to get the vertex declaration.", 
						"CMashGUIPrimitiveBatch::Initialise");

			return aMASH_FAILED;
		}

		m_pVertexDeclaration->Grab();

		ResizeBuffers();

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIPrimitiveBatch::UnlockBuffers()
	{
		if (m_pVertexPtr)
		{
			if (m_meshBuffer->GetVertexBuffer()->Unlock() == aMASH_FAILED)
			{
				return aMASH_FAILED;
			}

			m_pVertexPtr = 0;
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIPrimitiveBatch::LockBuffers()
	{
		if (!m_pVertexPtr)
		{
			if (m_meshBuffer->GetVertexBuffer()->Lock(mash::aLOCK_WRITE_DISCARD, (void**)(&m_pVertexPtr)) == aMASH_FAILED)
			{
				return aMASH_FAILED;
			}

			m_iCurrentVertexCount = 0;
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIPrimitiveBatch::ResizeBuffers()
	{
		if (UnlockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to unlock GUI buffer.", 
						"CMashGUIPrimitiveBatch::ResizeBuffers");

			return aMASH_FAILED;
		}

		if (!m_meshBuffer)
		{
			sVertexStreamInit streamData;
			streamData.data = 0;
			streamData.dataSizeInBytes = m_iCurrentReservedVertex * m_pVertexDeclaration->GetStreamSizeInBytes(0);
			streamData.usage = aUSAGE_DYNAMIC;

			m_meshBuffer = m_pRenderer->CreateMeshBuffer(&streamData, 1, m_pVertexDeclaration);
		}
		else
		{
			m_pVertexPtr = 0;
			if (m_meshBuffer->ResizeVertexBuffers(0, m_iCurrentReservedVertex * sizeof(mash::MashVertexColour::sMashVertexColour), aUSAGE_DYNAMIC, false) == aMASH_FAILED)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to resize vertex buffer.", 
                                 "CMashGUIPrimitiveBatch::ResizeBuffers");
                
				return aMASH_FAILED;
            }
		}

		if (!m_meshBuffer)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Mesh buffer failed to be created for GUI buffer.", 
                             "CMashGUIPrimitiveBatch::ResizeBuffers");
            
			return aMASH_FAILED;
        }

		m_iCurrentVertexCount = 0;
        
        return aMASH_OK;
	}

	eMASH_STATUS CMashGUIPrimitiveBatch::Draw(const mash::MashVertexColour::sMashVertexColour *vertices, uint32 vertexCount)
	{
		int32 iNewCount = m_iCurrentVertexCount+vertexCount;
		if (iNewCount > m_iCurrentReservedVertex)
		{
			//render all the buffers so that render layer order is maintained
			m_GUIManager->FlushBuffers();

			//check if the buffer can grow
			if (iNewCount < m_iMaxVertexCount)
			{
				m_iCurrentReservedVertex = iNewCount * 2;

				if (m_iCurrentReservedVertex > m_iMaxVertexCount)
					m_iCurrentReservedVertex = m_iMaxVertexCount;

				//resize the buffers
				if (ResizeBuffers() == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Mesh buffer resize failed.", 
						"CMashGUIPrimitiveBatch::Draw");

					return aMASH_FAILED;
				}
			}
		}

		if (LockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to lock GUI buffer.", 
						"CMashGUIPrimitiveBatch::Draw");

			return aMASH_FAILED;
		}

		memcpy(&m_pVertexPtr[m_iCurrentVertexCount * sizeof(mash::MashVertexColour::sMashVertexColour)],
			vertices, sizeof(mash::MashVertexColour::sMashVertexColour) * vertexCount);

		m_iCurrentVertexCount += vertexCount;
        
        return aMASH_OK;
	}

	eMASH_STATUS CMashGUIPrimitiveBatch::Draw(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour)
	{
		const int32 iVertexCount = 6;

		mash::MashVector3 positions[4];
		positions[0] = mash::MashVector3(rect.left, rect.top, 1.0f);
		positions[1] = mash::MashVector3(rect.right, rect.top, 1.0f);
		positions[2] = mash::MashVector3(rect.right, rect.bottom, 1.0f);
		positions[3] = mash::MashVector3(rect.left, rect.bottom, 1.0f);

		mash::MashVertexColour::sMashVertexColour verts[6];
		verts[0] = mash::MashVertexColour::sMashVertexColour(positions[0], colour);
		verts[1] = mash::MashVertexColour::sMashVertexColour(positions[1], colour);
		verts[2] = mash::MashVertexColour::sMashVertexColour(positions[3], colour);

		verts[3] = mash::MashVertexColour::sMashVertexColour(positions[1], colour);
		verts[4] = mash::MashVertexColour::sMashVertexColour(positions[2], colour);
		verts[5] = mash::MashVertexColour::sMashVertexColour(positions[3], colour);

		Draw(verts, iVertexCount);

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIPrimitiveBatch::Flush()
	{
		if (UnlockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to unlock GUI buffer.", 
						"CMashGUIPrimitiveBatch::Flush");

			return aMASH_FAILED;
		}

		eMASH_STATUS status = aMASH_OK;

		if (m_iCurrentVertexCount > 0)
		{
			if (m_pMaterial->OnSet() == aMASH_OK)
			{
				status = m_pRenderer->DrawVertexList(m_meshBuffer, m_iCurrentVertexCount,
					m_iCurrentVertexCount / 3, aPRIMITIVE_TRIANGLE_LIST);
			}

			m_iCurrentVertexCount = 0;
		}

		return status;
	}
}