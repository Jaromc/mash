//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_FONT_BATCH_H_
#define _C_MASH_GUI_FONT_BATCH_H_

#include "MashReferenceCounter.h"
#include "MashMeshBuffer.h"
#include "MashMaterial.h"
#include "MashVector2.h"
#include "MashAutoEffectParameter.h"

namespace mash
{
	class MashVideo;

	class MashGUIManager;

	class CMashGUIFontBatch : public MashReferenceCounter
	{
	private:
		MashVideo *m_pRenderer;
		MashVertex *m_pVertexDeclaration;
		MashMeshBuffer *m_meshBuffer;
		uint8 *m_pVertexPtr;
		int32 m_iCurrentReservedVertex;
		int32 m_iCurrentVertexCount;
		MashMaterial *m_pMaterial;

		mash::MashTexture *m_pCurrentTexture;
		mash::sMashColour4 m_currentColour;

		MashParamGUIFontColour *m_pFontColourEffectHandler;

		eMASH_STATUS UnlockBuffers();
		eMASH_STATUS LockBuffers();
		eMASH_STATUS ResizeBuffers();

		MashGUIManager *m_GUIManager;

	public:
		CMashGUIFontBatch(MashGUIManager *pGUIManager,
			MashVideo *pRenderer);
		~CMashGUIFontBatch();

		eMASH_STATUS Initialise();

		eMASH_STATUS Draw(const mash::MashVertexPosTex::sMashVertexPosTex *pVertices, 
			int32 iVertexCount, 
			mash::MashTexture *pTexture,
			const mash::sMashColour &fontColour);

		eMASH_STATUS Flush();
	};
}

#endif