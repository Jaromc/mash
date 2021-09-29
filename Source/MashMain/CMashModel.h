//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_MODEL_H_
#define _C_MASH_MODEL_H_

#include "MashList.h"
#include "MashModel.h"
#include "MashAABB.h"
#include "MashMatrix4.h"

namespace mash
{
	class MashSceneManager;
	class CMashModel : public MashModel
	{
	private:
		static int32 m_iModelCounter;
		int32 m_iModelID;
	protected:
		/*
			Its important the meshes are kept in linear order
			as some systems such as file loading/saving assume
			this is the case.
		*/
		MashArray<MashArray<MashMesh*> > m_meshes;
		mash::MashAABB m_boundingBox;

		mash::MashSceneManager *m_pSceneManager;
		MashTriangleCollider *m_triangleCollider;
	public:
		CMashModel(mash::MashSceneManager *pSceneManager);
		virtual ~CMashModel();

		/*
			Name must be unique. If no name is given then
			one will be generated for you.
		*/
		MashModel* Clone()const;

		eMASH_STATUS Append(MashMesh **pMeshLODArray, uint32 meshLODCount = 1);

		MashMesh* GetMesh(uint32 index, uint32 lod = 0)const;
		uint32 GetMeshCount(uint32 lod)const;
		uint32 GetLodCount()const;

		const mash::MashAABB& GetBoundingBox()const;
		void SetBoundingBox(const mash::MashAABB &aabb);

		/*
			This only calculates a bounding volume from the meshes
			current values. It does not recalculate the bounding volumes
			for each mesh.
		*/
		eMASH_STATUS CalculateBoundingBox();

		void GetTransformedBoundingBox(const mash::MashMatrix4 &mTransformation, mash::MashAABB &out)const;
		int32 GetID()const;

		void DropAllTriangleBuffers();
		MashTriangleCollider* GetTriangleCollider()const;
		void SetTriangleCollider(MashTriangleCollider *collider);
	};

	inline MashTriangleCollider* CMashModel::GetTriangleCollider()const
	{
		return m_triangleCollider;
	}

	inline uint32 CMashModel::GetLodCount()const
	{
		return m_meshes.Size();
	}

	inline int32 CMashModel::GetID()const
	{
		return m_iModelID;
	}

	inline const mash::MashAABB& CMashModel::GetBoundingBox()const
	{
		return m_boundingBox;
	}

	inline void CMashModel::SetBoundingBox(const mash::MashAABB &aabb)
	{
		m_boundingBox = aabb;
	}
}

#endif