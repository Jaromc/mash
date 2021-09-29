//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_VIDEO_INTERMEDIATE_H_
#define _MASH_VIDEO_INTERMEDIATE_H_

#include "MashVideo.h"
#include "MashArray.h"
#include "MashString.h"
#include "MashList.h"
#include <map>

namespace mash
{
	class MashFileManager;
	/*
		Serves as an intermediate class between the core engine
        and API implimentations.
     
        Access these methods through MashVideo.
	*/
	class MashVideoIntermediate : public MashVideo
	{
	protected:
		MashVideoIntermediate();
		virtual ~MashVideoIntermediate();
		MashFileManager *m_pFileManager;

		MashMeshBuffer *m_fsMeshBuffer;//fullscreen quad
		MashMeshBuffer *m_dynamicFsMeshBuffer;//dynamic fullscreen quad

		MashMaterial *m_drawTextureMaterial;
        MashMaterial *m_drawTextureTransMaterial;
		std::map<MashStringc, MashTexture*> m_textures;
		//held for onResize() calls
		MashList<MashRenderSurface*> m_renderSurfaces;
		MashRenderSurface *m_currentRenderSurface;
		sMashColour4 m_FillColour;

		mash::MashTextureState *m_drawTextureSampler;
		mash::MashTextureState *m_drawTextureHasMipmapSampler;
        mash::MashTextureState *m_drawTextureSamplerTrans;
		mash::MashTextureState *m_drawTextureHasMipmapSamplerTrans;
		
		uint32 m_currentDrawCount;
		uint32 m_currentFrameTechniqueChangeCount;
		
		uint16 m_FSVertexCount;
		MashRenderInfo *m_renderInfo;
		MashMaterialManager *m_skinManager;
		MashArray<MashVertex*> m_vertexTypes;
		sMashViewPort m_viewPort;
		sMashViewPort m_defaultRenderTargetViewport;

		uint32 m_textureIDCounter;
		uint32 m_defaultBlendState;
		uint32 m_defaultRasterizerState;

		int32 m_currentRasterizerState;
		int32 m_currentBlendState;

		bool m_lockRasterizerState;
		bool m_lockBlendState;

		mash::MashSceneManager *m_sceneManager;

		void GenerateUniqueTextureName(MashStringc &out);
		eMASH_STATUS ValidateTextureName(const MashStringc &in, MashStringc &out);

		eMASH_STATUS _InitialiseCommon(MashFileManager *fileManager);
		eMASH_STATUS OnPreResolutionChangeIntermediate();
		eMASH_STATUS OnPostResolutionChangeIntermediate(uint32 width, uint32 height);

		std::map<MashMaterialDependentResourceBase*, MashList<MashMaterialDependentResourceBase*> > m_onDependencyCompileResources;
	private:
		MashTexture* FindTexture(const MashStringc &name);
		virtual MashTexture* LoadTextureFromFile(const MashStringc &fileName) = 0;
	public:

		virtual eMASH_STATUS EndRender();
		virtual eMASH_STATUS BeginRender();		

		void LockRasterizerState(bool enable);
		void LockBlendState(bool enable);

		void _AddCompileDependency(MashMaterialDependentResourceBase *dependency, MashMaterialDependentResourceBase *resource);
		void _OnDependencyCompiled(MashMaterialDependentResourceBase *dependency);

		eMASH_STATUS SetRenderTarget(MashRenderSurface *renderTarget, int32 surface);
		virtual void SetFillColour(const sMashColour4 &fillColour);
		const sMashColour4& GetFillColour()const;

		mash::MashRenderInfo* GetRenderInfo()const;
		const sMashViewPort& GetViewport()const;
		mash::MashMaterialManager* GetMaterialManager()const;

		MashGeometryBatch* CreateGeometryBatch(MashMaterial *material, ePRIMITIVE_TYPE type, MashGeometryBatch::eBATCH_TYPE batchType);

		MashTexture* GetTexture(const MashStringc &fileName);
		bool RemoveTextureFromCache(mash::MashTexture *texture);

		void RemoveAllTexturesFromCache();

		MashRenderSurface* GetRenderSurface()const;


		eMASH_STATUS DrawFullScreenQuad();
		eMASH_STATUS DrawFullScreenQuadTexCoords(const mash::MashRectangle2 &texCoords);

		eMASH_STATUS DrawTextureClip(mash::MashTexture *texture,
				const mash::MashRectangle2 &screenPos,
				const mash::MashRectangle2 &clippingArea,
                bool isTransparent = false);

		eMASH_STATUS DrawTexture(mash::MashTexture *texture,
				const mash::MashRectangle2 &screenPos,
                bool isTransparent = false);

		eMASH_STATUS DrawMesh(const MashMesh *mesh, uint32 instanceCount);

		uint32 GetCurrentFrameDrawCount()const;
		uint32 GetCurrentFrameTechniqueChangeCount()const;
		void _IncrementCurrentFrameTechniqueChanges();

		void _OnViewportChange();
		void _SetSceneManager(mash::MashSceneManager *pSceneManager);

		/*
			called internally. Only removes the render surface from an internal list, this does not delete it.
		*/
		void _RemoveRenderSurface(MashRenderSurface *surface);
	};

	inline const sMashColour4& MashVideoIntermediate::GetFillColour()const
	{
		return m_FillColour;
	}

	inline void MashVideoIntermediate::LockBlendState(bool enable)
	{
		m_lockBlendState = enable;
	}

	inline void MashVideoIntermediate::LockRasterizerState(bool enable)
	{
		m_lockRasterizerState = enable;
	}

	inline void MashVideoIntermediate::_IncrementCurrentFrameTechniqueChanges()
	{
		++m_currentFrameTechniqueChangeCount;
	}

	inline uint32 MashVideoIntermediate::GetCurrentFrameTechniqueChangeCount()const
	{
		return m_currentFrameTechniqueChangeCount;
	}

	inline uint32 MashVideoIntermediate::GetCurrentFrameDrawCount()const
	{
		return m_currentDrawCount;
	}

	inline mash::MashMaterialManager* MashVideoIntermediate::GetMaterialManager()const
	{
		return m_skinManager;
	}

	inline MashRenderSurface* MashVideoIntermediate::GetRenderSurface()const
	{
		return m_currentRenderSurface;
	}

	inline const sMashViewPort& MashVideoIntermediate::GetViewport()const
	{
		return m_viewPort;
	}
}

#endif