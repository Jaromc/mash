//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_PRIMITIVE_BATCH_H_
#define _C_MASH_PRIMITIVE_BATCH_H_

#include "MashGeometryBatch.h"
#include "MashTypes.h"
#include "MashGenericArray.h"
#include "MashMeshBuffer.h"

namespace mash
{
	class MashMaterial;
	class MashVideo;
	class MashMesh;

	class CMashGeometryBatch : public MashGeometryBatch
	{
	private:
		MashVideo *m_pRenderer;
		uint32 m_iVertexCount;
		ePRIMITIVE_TYPE m_iPrimitiveType;
		MashMaterial *m_pMaterial;
		MashMeshBuffer *m_meshBuffer;

		//expressed in bytes
		uint32 m_iCurrentBufferSizeInBytes;

		MashGenericArray m_cachedPoints;
		bool m_commitNeeded;
		bool m_bInitialised;
		eBATCH_TYPE m_eBatchType;

		int32 GetPrimitiveCount()const;
	public:
		CMashGeometryBatch(MashVideo *pRenderer);
		~CMashGeometryBatch();

		eMASH_STATUS Initialise(MashMaterial *pMaterial, mash::ePRIMITIVE_TYPE type, eBATCH_TYPE eBatchType = aDYNAMIC);

		eMASH_STATUS AddPoints(const uint8 *pPoints, uint32 iCount);

		eMASH_STATUS Flush();
	};
}

#endif