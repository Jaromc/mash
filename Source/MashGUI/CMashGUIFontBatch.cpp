//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIFontBatch.h"
#include "MashGUIComponent.h"
#include "MashTechniqueInstance.h"
#include "MashMaterialManager.h"
#include "MashVertexBuffer.h"
#include "MashVertex.h"
#include "MashVideo.h"
#include "MashGUIManager.h"
#include "MashLog.h"
namespace mash
{
	CMashGUIFontBatch::CMashGUIFontBatch(MashGUIManager *pGUIManager, 
		MashVideo *pRenderer):
		m_pVertexDeclaration(0),
		m_GUIManager(pGUIManager),
		m_pRenderer(pRenderer),
		m_pVertexPtr(0),
		m_iCurrentReservedVertex(6),
		m_iCurrentVertexCount(0),
		m_pCurrentTexture(0),
		m_currentColour(1.0f, 1.0f, 1.0f, 1.0f),
		m_pMaterial(0),
		m_pFontColourEffectHandler(0),
		m_meshBuffer(0)
	{
	}

	CMashGUIFontBatch::~CMashGUIFontBatch()
	{
		if (m_pVertexDeclaration)
		{
			m_pVertexDeclaration->Drop();
			m_pVertexDeclaration = 0;
		}

		if (m_meshBuffer)
		{
			m_meshBuffer->Drop();
			m_meshBuffer = 0;
		}

		if (m_pCurrentTexture)
		{
			m_pCurrentTexture->Drop();
			m_pCurrentTexture = 0;
		}

		if (m_pFontColourEffectHandler)
		{
			m_pFontColourEffectHandler->Drop();
			m_pFontColourEffectHandler = 0;
		}

		if (m_pMaterial)
		{
			m_pMaterial->Drop();
			m_pMaterial = 0;
		}
	}

	eMASH_STATUS CMashGUIFontBatch::Initialise()
	{
		if (m_pMaterial)
			m_pMaterial->Drop();

		m_pMaterial = m_pRenderer->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GUI_FONT);

		if (!m_pMaterial)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"GUI font material failed to load.", 
						"CMashGUIFontBatch::Initialise");

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
						"CMashGUIFontBatch::Initialise");

			return aMASH_FAILED;
		}

		m_pVertexDeclaration->Grab();

		ResizeBuffers();

		if (m_pFontColourEffectHandler)
			m_pFontColourEffectHandler->Drop();

		m_pFontColourEffectHandler = (MashParamGUIFontColour*)m_pRenderer->GetMaterialManager()->GetAutoParameterByName("autoGUIFontColour");
		m_pFontColourEffectHandler->Grab();

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIFontBatch::UnlockBuffers()
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

	eMASH_STATUS CMashGUIFontBatch::LockBuffers()
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

	eMASH_STATUS CMashGUIFontBatch::ResizeBuffers()
	{
		if (UnlockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to unlock GUI buffer.", 
						"CMashGUIFontBatch::ResizeBuffers");

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
			if (m_meshBuffer->ResizeVertexBuffers(0, m_iCurrentReservedVertex * sizeof(mash::MashVertexPosTex::sMashVertexPosTex), aUSAGE_DYNAMIC, false) == aMASH_FAILED)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to resize vertex buffer.", 
                                 "CMashGUIFontBatch::ResizeBuffers");
                
				return aMASH_FAILED;
            }
		}

		if (!m_meshBuffer)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Mesh buffer failed to be created for GUI buffer.", 
                             "CMashGUIFontBatch::ResizeBuffers");
            
			return aMASH_FAILED;
        }

		m_iCurrentVertexCount = 0;
        
        return aMASH_OK;
	}

	eMASH_STATUS CMashGUIFontBatch::Draw(const mash::MashVertexPosTex::sMashVertexPosTex *pVertices, 
			int32 iVertexCount, 
			mash::MashTexture *pTexture,
			const mash::sMashColour &fontColour)
	{

		if (!pTexture)
			return aMASH_FAILED;
	
		mash::sMashColour4 newColour(fontColour);
	
		if ((!m_pCurrentTexture) || (pTexture != m_pCurrentTexture) || (m_currentColour != newColour))
		{
			//render all the buffers so that render layer order is maintained
			m_GUIManager->FlushBuffers();

			if (m_pCurrentTexture)
				m_pCurrentTexture->Drop();

			m_pCurrentTexture = pTexture;
			m_pCurrentTexture->Grab();
			m_currentColour = newColour;
		}

		int32 iNewCount = m_iCurrentVertexCount+iVertexCount;
		if (m_iCurrentVertexCount+iVertexCount > m_iCurrentReservedVertex)
		{
			//render all the buffers so that render layer order is maintained
			m_GUIManager->FlushBuffers();

			//check if the buffer can grow
			//if (iNewCount < m_iMaxVertexCount)
			{
				m_iCurrentReservedVertex = iNewCount * 2;
				//resize the buffers
				if (ResizeBuffers() == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Mesh buffer resize failed.", 
						"CMashGUIFontBatch::Draw");

					return aMASH_FAILED;
				}
			}
		}

		if (LockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to lock GUI buffer.", 
						"CMashGUIFontBatch::Draw");

			return aMASH_FAILED;
		}

		memcpy(&m_pVertexPtr[m_iCurrentVertexCount * sizeof(mash::MashVertexPosTex::sMashVertexPosTex)],
			pVertices, sizeof(mash::MashVertexPosTex::sMashVertexPosTex) * iVertexCount);

		m_iCurrentVertexCount += iVertexCount;

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIFontBatch::Flush()
	{
		if (UnlockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to unlock GUI buffer.", 
						"CMashGUIFontBatch::Draw");

			return aMASH_FAILED;
		}

		eMASH_STATUS status = aMASH_OK;

		if (m_iCurrentVertexCount > 0)
		{

			MashTechniqueInstance *activeTechnique = m_pMaterial->GetActiveTechnique();
			if (activeTechnique)
			{
				activeTechnique->SetTexture(0, m_pCurrentTexture);
				m_pFontColourEffectHandler->SetValue(m_currentColour);

				if (m_pMaterial->OnSet() == aMASH_OK)
				{
					m_pRenderer->DrawVertexList(m_meshBuffer, m_iCurrentVertexCount,
						m_iCurrentVertexCount / 3, aPRIMITIVE_TRIANGLE_LIST);
				}

				m_iCurrentVertexCount = 0;
			}
		}

		return status;
	}
}