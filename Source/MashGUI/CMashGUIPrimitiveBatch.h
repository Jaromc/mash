//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_PRIMITIVE_BATCH_H_
#define _C_MASH_GUI_PRIMITIVE_BATCH_H_

#include "MashReferenceCounter.h"
#include "MashMeshBuffer.h"
#include "MashMaterial.h"
#include "MashVector2.h"
#include "MashRectangle2.h"
#include "MashAutoEffectParameter.h"

namespace mash
{
	class MashVideo;
	class MashGUIManager;

	class CMashGUIPrimitiveBatch : public MashReferenceCounter
	{
	private:
		MashVideo *m_pRenderer;
		MashMeshBuffer *m_meshBuffer;
		MashVertex *m_pVertexDeclaration;
		uint8 *m_pVertexPtr;
		int32 m_iCurrentReservedVertex;
		int32 m_iCurrentVertexCount;
		int32 m_iMaxVertexCount;
		MashMaterial *m_pMaterial;

		mash::sMashColour4 m_currentColour;

		eMASH_STATUS UnlockBuffers();
		eMASH_STATUS LockBuffers();
		eMASH_STATUS ResizeBuffers();

		MashGUIManager *m_GUIManager;

	public:
		CMashGUIPrimitiveBatch(MashGUIManager *pGUIManager,
			MashVideo *pRenderer, 
			int32 iMaxVertexCount = 100000);
		~CMashGUIPrimitiveBatch();

		eMASH_STATUS Initialise();

		eMASH_STATUS Draw(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour);

		eMASH_STATUS Draw(const mash::MashVertexColour::sMashVertexColour *vertices, uint32 vertexCount);

		eMASH_STATUS Flush();
	};
}

#endif