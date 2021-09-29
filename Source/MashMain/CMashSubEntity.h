//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SUB_ENTITY_H_
#define _C_MASH_SUB_ENTITY_H_

#include "MashMesh.h"
#include "MashSubEntity.h"

namespace mash
{
	class MashVideo;
	class MashSceneNode;
	class MashSceneManager;

	class MashEntity;

	class CMashSubEntity : public MashSubEntity
	{
	protected:
		mash::MashVideo *m_pRenderer;
		mash::MashSceneManager *m_pSceneManager;
		mash::MashMesh *m_mesh;
		MashMaterial *m_pMaterial;
		bool m_bActive;

		uint32 m_iDistanceFromCamera;

		MashEntity *m_pOwner;
	public:
		CMashSubEntity(mash::MashVideo *pMashRenderer, mash::MashSceneManager *pSceneManager, mash::MashMesh *mesh, MashEntity *pOwner);
		virtual ~CMashSubEntity();

		eMASH_STATUS LateUpdate(bool parentTransformChanged, uint32 iDistanceFromCamera);
		void SetIsActive(bool bActive);
		bool GetIsActive()const;
		mash::MashMesh* GetMesh();
		eMASH_STATUS SetMaterial(MashMaterial *pMaterial);
		MashMaterial* GetMaterial()const;
		void Draw();
		const mash::MashAABB& GetTotalWorldBoundingBox()const;
		const mash::MashAABB& GetWorldBoundingBox()const;

		const mash::MashAABB& GetLocalBoundingBox()const;
		MashMeshBuffer* GetMeshBuffer()const;
	};

	inline const mash::MashAABB& CMashSubEntity::GetLocalBoundingBox()const
	{
		if (m_mesh)
			return m_mesh->GetBoundingBox();

		return *((mash::MashAABB*)0);
	}

	inline void CMashSubEntity::SetIsActive(bool bActive)
	{
		m_bActive = bActive;
	}

	inline bool CMashSubEntity::GetIsActive()const
	{
		return m_bActive;
	}

	inline mash::MashMesh* CMashSubEntity::GetMesh()
	{
		return m_mesh;
	}

	inline MashMaterial* CMashSubEntity::GetMaterial()const
	{
		return m_pMaterial;
	}
}

#endif