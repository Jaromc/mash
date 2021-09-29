//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_RENDER_BATCH_H_
#define _C_MASH_GUI_RENDER_BATCH_H_

#include "MashReferenceCounter.h"
#include "MashMeshBuffer.h"
#include "MashMaterial.h"
#include "MashVector2.h"
#include "MashGUISkin.h"
#include "MashAutoEffectParameter.h"
#include "MashGUITypes.h"

namespace mash
{
	class MashVideo;
	class MashGUIComponent;
	class MashGUIManager;

	class CMashGUIRenderBatch : public MashReferenceCounter
	{
	private:
		MashVideo *m_pRenderer;
		MashVertex *m_pVertexDeclaration;
		MashMeshBuffer *m_meshBuffer;
		uint8 *m_pVertexPtr;
		int32 m_iCurrentReservedVertex;
		int32 m_iCurrentVertexCount;
		int32 m_iMaxVertexCount;
		MashMaterial *m_pMaterial;

		int32 m_alphaBlendState;
		int32 m_solidBlendState;

		MashGUIManager *m_GUIManager;

		MashParamGUIAlphaMaskThreshold *m_pAlphaMaskThreshholdEffectHandler;
		MashParamGUIBaseColour *m_pBaseColourEffectHandler;

		mash::MashTexture *m_baseTexture;
		bool m_isTransparent;
		f32 m_alphaMaskThreshold;
		sGUIOverrideTransparency m_transparencyOverride;

		eMASH_STATUS UnlockBuffers();
		eMASH_STATUS LockBuffers();
		eMASH_STATUS ResizeBuffers();

	public:
		CMashGUIRenderBatch(MashGUIManager *pGUIManager,
			MashVideo *pRenderer, 
			int32 iMaxVertexCount = 100000);
		~CMashGUIRenderBatch();

		eMASH_STATUS Initialise();

		eMASH_STATUS Draw(const mash::MashVertexGUI::sMashVertexGUI *pVertices,
			uint32 iVertexCount,
			const MashGUISkin *skin,
			const sGUIOverrideTransparency &transparencyOverride);

		eMASH_STATUS Flush();
	};
}

#endif