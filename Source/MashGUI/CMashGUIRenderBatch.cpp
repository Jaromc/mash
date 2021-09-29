//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIRenderBatch.h"
#include "MashGUIComponent.h"
#include "MashTechnique.h"
#include "MashTechniqueInstance.h"
#include "MashMaterialManager.h"
#include "MashVertexBuffer.h"
#include "MashVertex.h"
#include "MashVideo.h"
#include "MashGUIManager.h"
#include "MashLog.h"
namespace mash
{
	CMashGUIRenderBatch::CMashGUIRenderBatch(MashGUIManager *pGUIManager, 
		MashVideo *pRenderer,
		int32 iMaxVertexCount):
		m_pVertexDeclaration(0),
		m_GUIManager(pGUIManager),
		m_pRenderer(pRenderer),
		m_meshBuffer(0),m_pVertexPtr(0),
		m_iCurrentReservedVertex(6),
		m_iCurrentVertexCount(0),
		m_iMaxVertexCount(iMaxVertexCount),
		m_pMaterial(0),
		m_pAlphaMaskThreshholdEffectHandler(0),
		m_pBaseColourEffectHandler(0),
		m_baseTexture(0),
		m_isTransparent(false),
		m_alphaMaskThreshold(1.0f)
	{

	}

	CMashGUIRenderBatch::~CMashGUIRenderBatch()
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

		if (m_baseTexture)
		{
			m_baseTexture->Drop();
			m_baseTexture = 0;
		}

		if (m_pAlphaMaskThreshholdEffectHandler)
		{
			m_pAlphaMaskThreshholdEffectHandler->Drop();
			m_pAlphaMaskThreshholdEffectHandler = 0;
		}

		if (m_pBaseColourEffectHandler)
		{
			m_pBaseColourEffectHandler->Drop();
			m_pBaseColourEffectHandler = 0;
		}
	}

	eMASH_STATUS CMashGUIRenderBatch::Initialise()
	{
		MashMaterialManager *skinManager = m_pRenderer->GetMaterialManager();
		if (m_pMaterial)
			m_pMaterial->Drop();

		m_pMaterial = skinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_GUI_SPRITE);

		if (!m_pMaterial)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"GUI sprite material failed to load.", 
						"CMashGUIRenderBatch::Initialise");

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
						"CMashGUIRenderBatch::Initialise");

			return aMASH_FAILED;
		}

		m_pVertexDeclaration->Grab();

		ResizeBuffers();

		if (m_pAlphaMaskThreshholdEffectHandler)
			m_pAlphaMaskThreshholdEffectHandler->Drop();

		if (m_pBaseColourEffectHandler)
			m_pBaseColourEffectHandler->Drop();

		m_pAlphaMaskThreshholdEffectHandler = (MashParamGUIAlphaMaskThreshold*)skinManager->GetAutoParameterByName("autoGUIAlphaMaskThreshold");
		m_pAlphaMaskThreshholdEffectHandler->Grab();

		m_pBaseColourEffectHandler = (MashParamGUIBaseColour*)skinManager->GetAutoParameterByName("autoGUIBaseColour");
		m_pBaseColourEffectHandler->Grab();

		//alpha blend state
		sBlendStates alphaBlend;
		alphaBlend.blendingEnabled = true;
		alphaBlend.srcBlend = aBLEND_SRC_ALPHA;
		alphaBlend.destBlend = aBLEND_INV_SRC_ALPHA;
		alphaBlend.blendOp = aBLENDOP_ADD;
		alphaBlend.srcBlendAlpha = aBLEND_ZERO;
		alphaBlend.destBlendAlpha = aBLEND_ZERO;
		alphaBlend.blendOpAlpha = aBLENDOP_ADD;
		alphaBlend.colourWriteMask = aCOLOUR_WRITE_ALL;
		m_alphaBlendState = m_pRenderer->AddBlendState(alphaBlend);

		//solid blend state
		sBlendStates solidBlend;
		solidBlend.blendingEnabled = false;
		solidBlend.srcBlend = aBLEND_ONE;
		solidBlend.destBlend = aBLEND_ZERO;
		solidBlend.blendOp = aBLENDOP_ADD;
		solidBlend.srcBlendAlpha = aBLEND_ZERO;
		solidBlend.destBlendAlpha = aBLEND_ZERO;
		solidBlend.blendOpAlpha = aBLENDOP_ADD;
		solidBlend.colourWriteMask = aCOLOUR_WRITE_ALL;
		m_solidBlendState = m_pRenderer->AddBlendState(solidBlend);

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIRenderBatch::UnlockBuffers()
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

	eMASH_STATUS CMashGUIRenderBatch::LockBuffers()
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

	eMASH_STATUS CMashGUIRenderBatch::ResizeBuffers()
	{
		if (UnlockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to unlock GUI buffer.", 
						"CMashGUILineBatch::ResizeBuffers");

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
			if (m_meshBuffer->ResizeVertexBuffers(0, m_iCurrentReservedVertex * sizeof(mash::MashVertexGUI::sMashVertexGUI), aUSAGE_DYNAMIC, false) == aMASH_FAILED)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to resize vertex buffer.", 
                                 "CMashGUIRenderBatch::ResizeBuffers");
                
				return aMASH_FAILED;
            }
		}

		if (!m_meshBuffer)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Mesh buffer failed to be created for GUI buffer.", 
                             "CMashGUIRenderBatch::ResizeBuffers");
            
			return aMASH_FAILED;
        }

		m_iCurrentVertexCount = 0;
        
        return aMASH_OK;
	}

	eMASH_STATUS CMashGUIRenderBatch::Draw(const mash::MashVertexGUI::sMashVertexGUI *pVertices,
			uint32 iVertexCount,
			const MashGUISkin *skin,
			const sGUIOverrideTransparency &transparencyOverride)
	{
		if (skin->GetTexture() != m_baseTexture || 
			skin->isTransparent != m_isTransparent || 
			skin->alphaMaskThreshold != m_alphaMaskThreshold || 
			(transparencyOverride != m_transparencyOverride))
		{
			//render all the buffers so that render layer order is maintained
			m_GUIManager->FlushBuffers();

			if (skin->GetTexture())
				skin->GetTexture()->Grab();

			if (m_baseTexture)
				m_baseTexture->Drop();

			m_baseTexture = skin->GetTexture();

			m_isTransparent = skin->isTransparent;
			m_alphaMaskThreshold = skin->alphaMaskThreshold;
			m_transparencyOverride = transparencyOverride;
		}

		uint32 iSizeNeeded = m_iCurrentVertexCount + iVertexCount;
		if (iSizeNeeded > m_iCurrentReservedVertex)
		{
			m_GUIManager->FlushBuffers();

			//check if the buffer can grow
			if (m_iCurrentVertexCount < m_iMaxVertexCount)
			{
				m_iCurrentReservedVertex = iSizeNeeded * 2;

				if (m_iCurrentReservedVertex > m_iMaxVertexCount)
					m_iCurrentReservedVertex = m_iMaxVertexCount;

				//resize the buffers
				if (ResizeBuffers() == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Mesh buffer resize failed.", 
						"CMashGUIRenderBatch::Draw");

					return aMASH_FAILED;
				}
			}
		}

		if (LockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to lock GUI buffer.", 
						"CMashGUIRenderBatch::Draw");

			return aMASH_FAILED;
		}

		memcpy(&m_pVertexPtr[m_iCurrentVertexCount * sizeof(mash::MashVertexGUI::sMashVertexGUI)],
			pVertices, sizeof(mash::MashVertexGUI::sMashVertexGUI) * iVertexCount);

		m_iCurrentVertexCount += iVertexCount;

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIRenderBatch::Flush()
	{
		if (UnlockBuffers() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Failed to unlock GUI buffer.", 
						"CMashGUIRenderBatch::Flush");

			return aMASH_FAILED;
		}

		eMASH_STATUS status = aMASH_OK;

		if (m_iCurrentVertexCount > 0)
		{
			MashTechniqueInstance *activeTechnique = m_pMaterial->GetActiveTechnique();
			if (activeTechnique)
			{
				activeTechnique->SetTexture(0, m_baseTexture);

				if (m_transparencyOverride.enableOverrideTransparency)
				{
					m_pAlphaMaskThreshholdEffectHandler->SetValue(m_transparencyOverride.alphaMaskThreshold);

					if (m_transparencyOverride.alphaValue < 255)
						activeTechnique->GetTechnique()->SetBlendStateIndex(m_alphaBlendState);
					else
						activeTechnique->GetTechnique()->SetBlendStateIndex(m_solidBlendState);
				}
				else
				{
					m_pAlphaMaskThreshholdEffectHandler->SetValue(m_alphaMaskThreshold);
					if (m_isTransparent)
						activeTechnique->GetTechnique()->SetBlendStateIndex(m_alphaBlendState);
					else
						activeTechnique->GetTechnique()->SetBlendStateIndex(m_solidBlendState);
				}
			}

			if (m_pMaterial->OnSet())
			{
				m_pRenderer->DrawVertexList(m_meshBuffer, m_iCurrentVertexCount,
					m_iCurrentVertexCount / 3, aPRIMITIVE_TRIANGLE_LIST);
			}

			m_iCurrentVertexCount = 0;
		}

		return status;
	}
}