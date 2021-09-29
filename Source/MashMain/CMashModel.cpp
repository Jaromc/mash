//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashModel.h"
#include "MashLog.h"
#include "MashTriangleCollider.h"
#include "MashMesh.h"
#include "MashSceneManager.h"
#include <assert.h>
namespace mash
{
	int32 CMashModel::m_iModelCounter = 0;

	CMashModel::CMashModel(mash::MashSceneManager *pSceneManager):m_iModelID(m_iModelCounter++),
		m_pSceneManager(pSceneManager), m_triangleCollider(0)
	{
		m_boundingBox = mash::MashAABB();
	}

	MashModel* CMashModel::Clone()const
	{
		CMashModel *pNewModel = (CMashModel*)m_pSceneManager->CreateModel();

		if (!pNewModel)
			return 0;

		MashArray<MashMesh*> meshBuffer;
		MashArray<MashArray<MashMesh*> >::ConstIterator iter = m_meshes.Begin();
		MashArray<MashArray<MashMesh*> >::ConstIterator end = m_meshes.End();
		for(; iter != end; ++iter)
		{
			const uint32 lodCount = iter->Size();
			for(uint32 i = 0; i < lodCount; ++i)
			{
				meshBuffer.PushBack((*iter)[i]->Clone());
			}

			pNewModel->Append(&meshBuffer[0]);

			meshBuffer.Clear();
		}
        
        return pNewModel;
	}

	CMashModel::~CMashModel()
	{
		MashArray<MashArray<MashMesh*> >::Iterator iter = m_meshes.Begin();
		MashArray<MashArray<MashMesh*> >::Iterator end = m_meshes.End();
		for(; iter != end; ++iter)
		{
			const uint32 lodCount = iter->Size();
			for(uint32 i = 0; i < lodCount; ++i)
			{
				(*iter)[i]->Drop();
			}
		}

		m_meshes.Clear();

		if (m_triangleCollider)
		{
			m_triangleCollider->Drop();
			m_triangleCollider = 0;
		}
	}

	uint32 CMashModel::GetMeshCount(uint32 lod)const
	{
#ifdef MASH_DEBUG
		if (lod >= m_meshes.Size())
		{
			MASH_LOG_BOUNDS_ERROR(lod, 0, m_meshes.Size(), "lod", "CMashModel::GetMeshCount")
			return 0;
		}
#endif

		return m_meshes[lod].Size();
	}

	MashMesh* CMashModel::GetMesh(uint32 index, uint32 lod)const
	{
#ifdef MASH_DEBUG
		if (lod >= m_meshes.Size())
		{
			MASH_LOG_BOUNDS_ERROR(lod, 0, m_meshes.Size(), "lod", "CMashModel::GetMesh")
			return 0;
		}
#endif

#ifdef MASH_DEBUG
		if (index >= m_meshes[lod].Size())
		{
			MASH_LOG_BOUNDS_ERROR(lod, 0, m_meshes[lod].Size(), "index", "CMashModel::GetMesh")
			return 0;
		}
#endif

		return m_meshes[lod][index];
	}

	eMASH_STATUS CMashModel::Append(MashMesh **pMeshLODArray, uint32 meshCount)
	{
		if (!pMeshLODArray)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							"Mesh lod array is null",
							"CMashModel::Append");

			return aMASH_FAILED;
		}

		MashArray<MashMesh*> meshLODList;
		for(uint32 i = 0; i < meshCount; ++i)
		{
			if (pMeshLODArray[i])
			{
				m_boundingBox.Merge(pMeshLODArray[i]->GetBoundingBox());
				meshLODList.PushBack(pMeshLODArray[i]);
				pMeshLODArray[i]->Grab();
			}
		}
		
		m_meshes.PushBack(meshLODList);

		return aMASH_OK;
	}

	void CMashModel::SetTriangleCollider(MashTriangleCollider *collider)
	{
		if (collider)
			collider->Grab();

		if (m_triangleCollider)
			m_triangleCollider->Drop();

		m_triangleCollider = collider;
	}

	void CMashModel::DropAllTriangleBuffers()
	{
		MashArray<MashArray<MashMesh*> >::Iterator iter = m_meshes.Begin();
		MashArray<MashArray<MashMesh*> >::Iterator end = m_meshes.End();
		for(; iter != end; ++iter)
		{
			const uint32 lodCount = iter->Size();
			for(uint32 i = 0; i < lodCount; ++i)
				(*iter)[i]->SetTriangleBuffer(0);
		}

	}

	eMASH_STATUS CMashModel::CalculateBoundingBox()
	{
		eMASH_STATUS status = aMASH_OK;
		m_boundingBox = mash::MashAABB();

		MashArray<MashArray<MashMesh*> >::Iterator iter = m_meshes.Begin();
		MashArray<MashArray<MashMesh*> >::Iterator end = m_meshes.End();
		for(; iter != end; ++iter)
		{
			const uint32 lodCount = iter->Size();
			for(uint32 i = 0; i < lodCount; ++i)
				m_boundingBox.Merge((*iter)[i]->GetBoundingBox());
		}

		return status;
	}

	void CMashModel::GetTransformedBoundingBox(const mash::MashMatrix4 &mTransformation, mash::MashAABB &out)const
	{			
		m_boundingBox.Transform(mTransformation, out);
	}
}