//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_ENTITY_EX_H_
#define _C_MASH_ENTITY_EX_H_

#include "MashEntity.h"
#include "CMashSubEntity.h"

namespace mash
{
	class MashVideo;

	class CMashEntityEx : public MashEntity
	{
	private:
		
		mash::MashVideo *m_pMashRenderer;
		bool m_bTransformChanged;

		MashArray<MashArray<CMashSubEntity*> > m_subEntityLodList;
		MashArray<uint32> m_lodDistances;
		MashModel *m_model;
		uint32 m_currentLod;

		MashSkin *m_skin;

		void OnPassCullImpl(f32 interpolateAmount);
        void OnNodeTransformChange();
	public:
		CMashEntityEx(MashSceneNode *pParent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pMashRenderer,
			const MashStringc &sUserName,
			bool bUseOctree = false); 

		CMashEntityEx(MashSceneNode *pParent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pMashRenderer,
			MashModel *pModel,
			const MashStringc &sUserName,
			bool bUseOctree = false);

		~CMashEntityEx();

		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		bool ContainsRenderables()const;
		void SetMaterialToAllSubEntities(MashMaterial *material);
		void SetSubEntityMaterial(MashMaterial *material, uint32 meshIndex, uint32 lod = 0);

		MashSubEntity* GetSubEntity(uint32 meshIndex, uint32 lod = 0);

		uint32 GetLodCount()const;
		uint32 GetSubEntityCount(uint32 lod)const;
		eMASH_STATUS SetLodDistance(uint32 lod, uint32 distance);
		const MashArray<uint32>& GetLodDistances()const;

		MashSkin* GetSkin()const;
		void SetSkin(MashSkin *skin);
		void SetModel(mash::MashModel *model);
		bool AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr);

		const mash::MashAABB& GetLocalBoundingBox()const;
		MashModel* GetModel()const;

		uint32 GetNodeType()const;
		const mash::MashTriangleCollider* GetTriangleCollider()const;
	};

	inline const MashArray<uint32>& CMashEntityEx::GetLodDistances()const
	{
		return m_lodDistances;
	}

	inline MashSkin* CMashEntityEx::GetSkin()const
	{
		return m_skin;
	}

	inline uint32 CMashEntityEx::GetLodCount()const
	{
		return m_subEntityLodList.Size();
	}

	inline bool CMashEntityEx::ContainsRenderables()const
	{
		return true;
	}

	inline MashModel* CMashEntityEx::GetModel()const
	{
		return m_model;
	}

	inline uint32 CMashEntityEx::GetNodeType()const
	{
		return aNODETYPE_ENTITY;
	}
}

#endif