//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashVideoIntermediate.h"
#include "CMashRenderInfo.h"
#include "MashSceneManager.h"
#include "MashMeshBuffer.h"
#include "CMashPrimitiveBatch.h"
#include "MashHelper.h"
#include "MashTechniqueInstance.h"
#include "MashVertexBuffer.h"
#include "MashVertex.h"
#include "MashMaterialManager.h"
#include "MashMaterialDependentResource.h"
#include "MashLog.h"
#include "MashMesh.h"
#include "MashRectangle2.h"
#include "MashFileManager.h"

namespace mash
{
	MashVideoIntermediate::MashVideoIntermediate():MashVideo(),
        m_currentRenderSurface(0),
		m_FillColour(0.0f, 0.0f, 0.0f, 0.0f), 
		m_fsMeshBuffer(0), m_FSVertexCount(0), m_renderInfo(0), 
		m_textureIDCounter(0),  
		m_viewPort(), m_defaultBlendState(0),
		m_defaultRasterizerState(0),m_currentRasterizerState(-1),
		m_currentBlendState(-1), m_pFileManager(0), m_sceneManager(0),
		m_drawTextureMaterial(0), m_currentDrawCount(0),
		m_currentFrameTechniqueChangeCount(0), m_dynamicFsMeshBuffer(0), m_lockRasterizerState(false), m_lockBlendState(false),m_drawTextureTransMaterial(0)
	{
	}

	MashVideoIntermediate::~MashVideoIntermediate()
	{
 		MashArray<MashVertex*>::Iterator vertexIter = m_vertexTypes.Begin();
		MashArray<MashVertex*>::Iterator vertexIterEnd = m_vertexTypes.End();
		for(; vertexIter != vertexIterEnd; ++vertexIter)
			(*vertexIter)->Drop();

		m_vertexTypes.Clear();
        
        if (m_currentRenderSurface)
        {
            m_currentRenderSurface->Drop();
            m_currentRenderSurface = 0;
        }

		if (m_fsMeshBuffer)
		{
			m_fsMeshBuffer->Drop();
			m_fsMeshBuffer = 0;
		}

		if (m_dynamicFsMeshBuffer)
		{
			m_dynamicFsMeshBuffer->Drop();
			m_dynamicFsMeshBuffer = 0;
		}

		if (m_skinManager)
		{
			m_skinManager->Drop();
			m_skinManager = 0;
		}

		if (m_renderInfo)
		{
			m_renderInfo->Drop();
			m_renderInfo = 0;
		}

		RemoveAllTexturesFromCache();
	}

	eMASH_STATUS MashVideoIntermediate::_InitialiseCommon(MashFileManager *pFileManager)
	{
		m_pFileManager = pFileManager;

		m_renderInfo = MASH_NEW_COMMON CMashRenderInfo();

		//create default blend state
		sBlendStates solidBlend;
		solidBlend.blendingEnabled = false;
		solidBlend.srcBlend = aBLEND_ONE;
		solidBlend.destBlend = aBLEND_ZERO;
		solidBlend.blendOp = aBLENDOP_ADD;
		solidBlend.srcBlendAlpha = aBLEND_ONE;
		solidBlend.destBlendAlpha = aBLEND_ZERO;
		solidBlend.blendOpAlpha = aBLENDOP_ADD;
		solidBlend.colourWriteMask = aCOLOUR_WRITE_ALL;
		m_defaultBlendState = AddBlendState(solidBlend);

		//create default rasterizer state
		sRasteriserStates rasterizerState;
		rasterizerState.cullMode = aCULL_CCW;
		rasterizerState.fillMode = aFILL_SOLID;
		rasterizerState.depthBias = 0;
		rasterizerState.depthBiasClamp = 0.0f;
		rasterizerState.slopeScaledDepthBias = 0.0f;
		rasterizerState.depthTestingEnable = true;
		rasterizerState.depthWritingEnabled = true;
		rasterizerState.depthComparison = aZCMP_LESS_EQUAL;
		m_defaultRasterizerState = AddRasteriserState(rasterizerState);

		/*
			Although this material may never actually be used, the vertex shader is needed to setup
			the full screen quad buffer.
		*/
		if (m_skinManager)
		{
			m_drawTextureMaterial = m_skinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_DRAW_TEXTURE);

			if (!m_drawTextureMaterial)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to load core draw texture material",
					"MashVideoIntermediate::_InitialiseCommon");

				return aMASH_FAILED;
			}
            
            m_drawTextureTransMaterial = m_skinManager->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_DRAW_TRANS_TEXTURE);
            
            if (!m_drawTextureTransMaterial)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                 "Failed to load core draw texture transparent material",
                                 "MashVideoIntermediate::_InitialiseCommon");
                
				return aMASH_FAILED;
			}
		}

		MashVertexPosTex::sMashVertexPosTex fullScreenQuadVerts[6] = {
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, 1.0f, 1.0f), mash::MashVector2(1.0f, 0.0f)),
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, -1.0f, 1.0f), mash::MashVector2(0.0f, 1.0f)),
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, 1.0f, 1.0f), mash::MashVector2(0.0f, 0.0f)),
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, 1.0f, 1.0f), mash::MashVector2(1.0f, 0.0f)),
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, -1.0f, 1.0f), mash::MashVector2(1.0f, 1.0f)),
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, -1.0f, 1.0f), mash::MashVector2(0.0f, 1.0f))};

		const MashVertex *fsVertexDeclaration = m_drawTextureMaterial->GetVertexDeclaration();

		if (!fsVertexDeclaration)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to create vertex declaration for generic full screen quad",
					"MashVideoIntermediate::_InitialiseCommon");

			return aMASH_FAILED;
		}

		sVertexStreamInit streamData;
		streamData.data = fullScreenQuadVerts;
		streamData.dataSizeInBytes = sizeof(MashVertexPosTex::sMashVertexPosTex) * 6;
		streamData.usage = aUSAGE_STATIC;

		//create fullscreen quad
		m_fsMeshBuffer = CreateMeshBuffer(&streamData, 1,fsVertexDeclaration);
		m_FSVertexCount = 6;

		//create dynamic fullscreen quad for texture manip
		streamData.usage = aUSAGE_DYNAMIC;
		m_dynamicFsMeshBuffer = CreateMeshBuffer(&streamData, 1,fsVertexDeclaration);

		sSamplerState drawTextureSampler;
		drawTextureSampler.type = aSAMPLER2D;
		drawTextureSampler.filter = aFILTER_MIN_MAG_LINEAR;
		drawTextureSampler.uMode = aTEXTURE_ADDRESS_CLAMP;
		drawTextureSampler.vMode = aTEXTURE_ADDRESS_CLAMP;

		m_drawTextureSampler = AddSamplerState(drawTextureSampler);

		drawTextureSampler.filter = aFILTER_MIN_MAG_MIP_LINEAR;
		m_drawTextureHasMipmapSampler = AddSamplerState(drawTextureSampler);

		return aMASH_OK;
	}

	void MashVideoIntermediate::_OnViewportChange()
	{
		if (m_sceneManager)
			m_sceneManager->_OnViewportChange();
	}

	void MashVideoIntermediate::_SetSceneManager(mash::MashSceneManager *pSceneManager)
	{
		m_sceneManager = pSceneManager;
	}

	eMASH_STATUS MashVideoIntermediate::SetRenderTarget(MashRenderSurface *pRenderTarget, int32 iSurface)
	{
		if (pRenderTarget == 0)
		{
			if (m_currentRenderSurface)
            {
				m_currentRenderSurface->OnDismount();
                m_currentRenderSurface->Drop();
            }

			m_currentRenderSurface = 0;

			return aMASH_OK;
		}


		{
            if (m_currentRenderSurface != pRenderTarget)
            {
                pRenderTarget->Grab();
                
                if (m_currentRenderSurface)
                {
                    m_currentRenderSurface->OnDismount();
                    m_currentRenderSurface->Drop();
                }
            }
			
			//set this before the on set so that values can be grabbed from it
			m_currentRenderSurface = pRenderTarget;

			if (pRenderTarget->OnSet(iSurface) == aMASH_FAILED)
			{
				m_currentRenderSurface = 0;
                pRenderTarget->Drop();
				return aMASH_FAILED;
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS MashVideoIntermediate::OnPreResolutionChangeIntermediate()
	{
		MashList<MashRenderSurface*>::Iterator surfaceIter = m_renderSurfaces.Begin();
		MashList<MashRenderSurface*>::Iterator surfaceIterEnd = m_renderSurfaces.End();
		for(; surfaceIter != surfaceIterEnd; ++surfaceIter)
		{
			if ((*surfaceIter)->OnPreResize() == aMASH_FAILED)
				return aMASH_FAILED;
		}

		//shaders need to be reset
		m_renderInfo->SetTechnique(0);
		SetVertexFormat(0);
		return aMASH_OK;
	}

	eMASH_STATUS MashVideoIntermediate::OnPostResolutionChangeIntermediate(uint32 width, uint32 height)
	{
		eMASH_STATUS status = aMASH_OK;

		MashList<MashRenderSurface*>::Iterator surfaceIter = m_renderSurfaces.Begin();
		MashList<MashRenderSurface*>::Iterator surfaceIterEnd = m_renderSurfaces.End();
		for(; surfaceIter != surfaceIterEnd; ++surfaceIter)
		{
			if ((*surfaceIter)->OnPostResize(width, height) == aMASH_FAILED)
				status = aMASH_FAILED;
		}

		if (m_sceneManager)
			m_sceneManager->_OnPostResolutionChange();

		return status;
	}

	eMASH_STATUS MashVideoIntermediate::BeginRender()
	{
		/*
			Here we just reset anything in the render blackboard where some objects
			depend on a pointer being NULL to signify a value has changed, or a new
			frame has begun. This tends to be cheaper than constant notifications
			to some classes.
		*/

		m_renderInfo->SetSkin(0);
		m_currentDrawCount = 0;
		m_currentFrameTechniqueChangeCount = 0;

		return aMASH_OK;
	}

	eMASH_STATUS MashVideoIntermediate::EndRender()
	{
		//flush geometry buffers
		m_sceneManager->FlushGeometryBuffers();		

		return aMASH_OK;
	}

	void MashVideoIntermediate::SetFillColour(const sMashColour4 &fillColour)
	{
		m_FillColour = fillColour;
	}

	void MashVideoIntermediate::GenerateUniqueTextureName(MashStringc &out)
	{
		int8 buffer[100];
		int32 iCounter = 0;//stop inifinate loops
		do
		{
			out = "__texture";
			out += mash::helpers::NumberToString(buffer, 100, m_textureIDCounter++);
		}while(FindTexture(out) && (iCounter++ < 1000));
	}

	eMASH_STATUS MashVideoIntermediate::ValidateTextureName(const MashStringc &in, MashStringc &out)
	{
		out = in;
		if (out == "")
			GenerateUniqueTextureName(out);

		if (FindTexture(out))
			return aMASH_FAILED;

		return aMASH_OK;
	}

	mash::MashRenderInfo* MashVideoIntermediate::GetRenderInfo()const
	{
		return m_renderInfo;
	}

	MashTexture* MashVideoIntermediate::FindTexture(const MashStringc &sName)
	{
		std::map<MashStringc, MashTexture*>::const_iterator iter = m_textures.find(sName);
		if(iter != m_textures.end())
			return iter->second;
		
		return 0;
	}

	MashTexture* MashVideoIntermediate::GetTexture(const MashStringc &fileName)
	{
		MashTexture *pTexture = FindTexture(fileName);
		if (pTexture)
			return pTexture;

		return LoadTextureFromFile(fileName);
	}

	void MashVideoIntermediate::RemoveAllTexturesFromCache()
	{
		std::map<MashStringc, MashTexture*>::const_iterator iter = m_textures.begin();
		std::map<MashStringc, MashTexture*>::const_iterator end = m_textures.end();
		for(; iter != end; ++iter)
		{
			iter->second->Drop();
		}

		m_textures.clear();
		m_textureIDCounter = 0;
	}

	bool MashVideoIntermediate::RemoveTextureFromCache(mash::MashTexture *pTexture)
	{
		if (!pTexture)
			return true;

		std::map<MashStringc, MashTexture*>::iterator iter = m_textures.find(pTexture->GetName());
		if(iter != m_textures.end())
		{
			m_textures.erase(iter);
			return pTexture->Drop();
		}

		return false;
	}

	void MashVideoIntermediate::_RemoveRenderSurface(MashRenderSurface *surface)
	{
		if (!surface)
			return;

		m_renderSurfaces.Erase(surface);
	}

	eMASH_STATUS MashVideoIntermediate::DrawFullScreenQuad()
	{
		return DrawVertexList(m_fsMeshBuffer, m_FSVertexCount, 2, aPRIMITIVE_TRIANGLE_LIST);
	}

	eMASH_STATUS MashVideoIntermediate::DrawFullScreenQuadTexCoords(const mash::MashRectangle2 &texCoords)
	{
		MashVertexPosTex::sMashVertexPosTex fullScreenQuadVerts[6] = {
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, 1.0f, 1.0f), mash::MashVector2(texCoords.right, texCoords.top)),//tr
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, -1.0f, 1.0f), mash::MashVector2(texCoords.left, texCoords.bottom)),//bl
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, 1.0f, 1.0f), mash::MashVector2(texCoords.left, texCoords.top)),//tl
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, 1.0f, 1.0f), mash::MashVector2(texCoords.right, texCoords.top)),//tr
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, -1.0f, 1.0f), mash::MashVector2(texCoords.right, texCoords.bottom)),//br
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, -1.0f, 1.0f), mash::MashVector2(texCoords.left, texCoords.bottom))};//bl

		int8 *data = 0;
		if (m_dynamicFsMeshBuffer->GetVertexBuffer()->Lock(aLOCK_WRITE_DISCARD, (void**)(&data)) == aMASH_FAILED)
			return aMASH_FAILED;

		memcpy(data, fullScreenQuadVerts, sizeof(fullScreenQuadVerts));

		if (m_dynamicFsMeshBuffer->GetVertexBuffer()->Unlock() == aMASH_FAILED)
			return aMASH_FAILED;

		return DrawVertexList(m_dynamicFsMeshBuffer, m_FSVertexCount, 2, aPRIMITIVE_TRIANGLE_LIST);
	}

	eMASH_STATUS MashVideoIntermediate::DrawTextureClip(mash::MashTexture *pTexture,
				const mash::MashRectangle2 &screenPos,
				const mash::MashRectangle2 &clippingArea,
                bool isTransparent)
	{
		if (!pTexture)
			return aMASH_FAILED;

		uint32 texWidth, texHeight;
		pTexture->GetSize(texWidth, texHeight);

		mash::MashRectangle2 texCoords(0.0f, 0.0f, texWidth, texHeight);
		if (clippingArea.left > screenPos.left)
		{
			f32 lerp = (clippingArea.left - screenPos.left) / (screenPos.right - screenPos.left);
			texCoords.left = math::Lerp(0.0f, texWidth, lerp);
		}
		if (clippingArea.top > screenPos.top)
		{
			f32 lerp = (clippingArea.top - screenPos.top) / (screenPos.bottom - screenPos.top);
			texCoords.top = math::Lerp(0.0f, texHeight, lerp);
		}
		if (clippingArea.right < screenPos.right)
		{
			f32 lerp = (screenPos.right - clippingArea.right) / (screenPos.right - screenPos.left);
			texCoords.right = math::Lerp(texWidth, 0.0f, lerp);
		}
		if (clippingArea.bottom < screenPos.bottom)
		{
			f32 lerp = (screenPos.bottom - clippingArea.bottom) / (screenPos.bottom - screenPos.top);
			texCoords.bottom = math::Lerp(texHeight, 0.0f, lerp);
		}

		f32 oneOverTexWidth = 1.0f / (f32)texWidth;
		f32 oneOverTexHeight = 1.0f / (f32)texHeight;
		texCoords.top *= oneOverTexHeight;
		texCoords.left *= oneOverTexWidth;
		texCoords.bottom *= oneOverTexHeight;
		texCoords.right *= oneOverTexWidth;

		MashVertexPosTex::sMashVertexPosTex fullScreenQuadVerts[6] = {
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, 1.0f, 1.0f), mash::MashVector2(texCoords.right, texCoords.top)),//tr
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, -1.0f, 1.0f), mash::MashVector2(texCoords.left, texCoords.bottom)),//bl
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, 1.0f, 1.0f), mash::MashVector2(texCoords.left, texCoords.top)),//tl
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, 1.0f, 1.0f), mash::MashVector2(texCoords.right, texCoords.top)),//tr
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(1.0f, -1.0f, 1.0f), mash::MashVector2(texCoords.right, texCoords.bottom)),//br
			MashVertexPosTex::sMashVertexPosTex(mash::MashVector3(-1.0f, -1.0f, 1.0f), mash::MashVector2(texCoords.left, texCoords.bottom))};//bl

		int8 *data = 0;
		if (m_dynamicFsMeshBuffer->GetVertexBuffer()->Lock(aLOCK_WRITE_DISCARD, (void**)(&data)) == aMASH_FAILED)
			return aMASH_FAILED;

		memcpy(data, fullScreenQuadVerts, sizeof(fullScreenQuadVerts));

		if (m_dynamicFsMeshBuffer->GetVertexBuffer()->Unlock() == aMASH_FAILED)
			return aMASH_FAILED;

		sMashViewPort oldViewport = GetViewport();
		sMashViewPort newViewport;

		newViewport.width = screenPos.right - screenPos.left;
		newViewport.height = screenPos.bottom - screenPos.top;
		newViewport.x = screenPos.left;
		newViewport.y = screenPos.top;
		newViewport.minZ = 0.0f;
		newViewport.maxZ = 1.0f;
        
        MashMaterial *activeMaterial = (isTransparent)?m_drawTextureTransMaterial:m_drawTextureMaterial;

        MashTechniqueInstance *techniqueInstance = activeMaterial->GetActiveTechnique();
        
		if (!techniqueInstance)
			return aMASH_FAILED;
        
		techniqueInstance->SetTexture(0, pTexture);

		if (SetViewport(newViewport) == aMASH_FAILED)
			return aMASH_FAILED;

		if (pTexture->GetMipmapCount() > 1)
			activeMaterial->GetActiveTechnique()->SetTextureState(0, m_drawTextureHasMipmapSampler);
		else
			activeMaterial->GetActiveTechnique()->SetTextureState(0, m_drawTextureSampler);

		if (activeMaterial->OnSet() == aMASH_OK)
		{
			if (DrawVertexList(m_dynamicFsMeshBuffer, m_FSVertexCount, 2, aPRIMITIVE_TRIANGLE_LIST) == aMASH_FAILED)
				return aMASH_FAILED;
		}

		if (SetViewport(oldViewport) == aMASH_FAILED)
			return aMASH_FAILED;

		return aMASH_OK;
	}

	MashGeometryBatch* MashVideoIntermediate::CreateGeometryBatch(MashMaterial *pMaterial, ePRIMITIVE_TYPE type, MashGeometryBatch::eBATCH_TYPE eBatchType)
	{
		CMashGeometryBatch *newBatch = MASH_NEW_COMMON CMashGeometryBatch(this);
		if (newBatch->Initialise(pMaterial, type, eBatchType) == aMASH_FAILED)
		{
			MASH_DELETE newBatch;
			newBatch = 0;

			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to initialise geometry batch",
					"MashVideoIntermediate::CreateGeometryBatch");
		}

		return newBatch;
	}

	void MashVideoIntermediate::_AddCompileDependency(MashMaterialDependentResourceBase *dependency, MashMaterialDependentResourceBase *resource)
	{
		if (!dependency || !resource)
			return;

		if (dependency->IsValid())
			resource->OnDependencyCompiled(this, dependency);
		else
		{	
			/*
				Resources are only added when first created, therefore there is no need to
				check if it has already been added.

				Resources are grabbed incase things are dropped before this function is reached.
			*/
			std::map<MashMaterialDependentResourceBase*, MashList<MashMaterialDependentResourceBase*> >::iterator dependencyIter = m_onDependencyCompileResources.find(dependency);
			if (dependencyIter == m_onDependencyCompileResources.end())
			{
				dependencyIter = m_onDependencyCompileResources.insert(std::make_pair(dependency, MashList<MashMaterialDependentResourceBase*>())).first;
				dependency->GrabResource();
			}

			dependencyIter->second.PushBack(resource);
			resource->GrabResource();
		}
	}

	void MashVideoIntermediate::_OnDependencyCompiled(MashMaterialDependentResourceBase *dependency)
	{
		std::map<MashMaterialDependentResourceBase*, MashList<MashMaterialDependentResourceBase*> >::iterator dependencyIter = m_onDependencyCompileResources.find(dependency);
		if (dependencyIter != m_onDependencyCompileResources.end())
		{
			MashList<MashMaterialDependentResourceBase*>::Iterator resIter = dependencyIter->second.Begin();
			MashList<MashMaterialDependentResourceBase*>::Iterator resEndIter = dependencyIter->second.End();
			for(; resIter != resEndIter; ++resIter)
			{
				(*resIter)->OnDependencyCompiled(this, dependencyIter->first);
				(*resIter)->DropResource();
			}
			
			dependencyIter->first->DropResource();

			m_onDependencyCompileResources.erase(dependencyIter);
		}
	}

	eMASH_STATUS MashVideoIntermediate::DrawMesh(const MashMesh *mesh, uint32 instanceCount)
	{
		if (instanceCount > 0)
		{
			if (mesh->GetIndexCount() > 0)
			{
				return DrawIndexedInstancedList(mesh->GetMeshBuffer(),
							mesh->GetVertexCount(),
							mesh->GetIndexCount(),
							mesh->GetPrimitiveCount(),
							mesh->GetPrimitiveType(), instanceCount);
			}
			else
			{
				return DrawVertexInstancedList(mesh->GetMeshBuffer(),
							mesh->GetVertexCount(),
							mesh->GetPrimitiveCount(),
							mesh->GetPrimitiveType(), instanceCount);
			}
		}
		else
		{
			if (mesh->GetIndexCount() > 0)
			{
				return DrawIndexedList(mesh->GetMeshBuffer(),
							mesh->GetVertexCount(),
							mesh->GetIndexCount(),
							mesh->GetPrimitiveCount(),
							mesh->GetPrimitiveType());
			}
			else
			{
				return DrawVertexList(mesh->GetMeshBuffer(),
							mesh->GetVertexCount(),
							mesh->GetPrimitiveCount(),
							mesh->GetPrimitiveType());
			}
		}

		return aMASH_FAILED;
	}

	eMASH_STATUS MashVideoIntermediate::DrawTexture(mash::MashTexture *pTexture,
				const mash::MashRectangle2 &screenPos,
                bool isTransparent)
	{
		sMashViewPort oldViewport = GetViewport();
		sMashViewPort newViewport;

		newViewport.width = screenPos.right - screenPos.left;
		newViewport.height = screenPos.bottom - screenPos.top;
		newViewport.x = screenPos.left;
		newViewport.y = screenPos.top;
		newViewport.minZ = 0.0f;
		newViewport.maxZ = 1.0f;

        MashMaterial *activeMaterial = (isTransparent)?m_drawTextureTransMaterial:m_drawTextureMaterial;
        MashTechniqueInstance *techniqueInstance = activeMaterial->GetActiveTechnique();
        
		if (!techniqueInstance)
			return aMASH_FAILED;
        
		techniqueInstance->SetTexture(0, pTexture);

		if (SetViewport(newViewport) == aMASH_FAILED)
			return aMASH_FAILED;

		if (pTexture->GetMipmapCount() > 1)
			activeMaterial->GetActiveTechnique()->SetTextureState(0, m_drawTextureHasMipmapSampler);
		else
			activeMaterial->GetActiveTechnique()->SetTextureState(0, m_drawTextureSampler);

		if (activeMaterial->OnSet() == aMASH_OK)
		{
			if (DrawFullScreenQuad() == aMASH_FAILED)
				return aMASH_FAILED;
		}

		if (SetViewport(oldViewport) == aMASH_FAILED)
			return aMASH_FAILED;

		return aMASH_OK;
	}
}