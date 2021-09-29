//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_DYNAMIC_DECAL_SCENE_NODE_H_
#define _C_MASH_DYNAMIC_DECAL_SCENE_NODE_H_

#include "CMashDecalIntermediate.h"

namespace mash
{
	class CMashDynamicDecal : public CMashDecalIntermediate
	{
	private:
		/*
			These variables are used when a decal limit is set.
		*/
		bool m_decalLimitSet;
		bool m_decalLimitReached;
		uint32 m_newestDecalIndex;
		uint32 m_decalLimit;
		uint32 m_decalCount;
		//each element is a decal.
		MashArray<uint32> m_decalVertexCountList;

		sVertexData m_vertexData;

		void OnAddNewDecal();
	public:
		CMashDynamicDecal(MashSceneNode *parent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			const MashStringc &sName,
			MashMaterial *pMaterial,
			MashSkin *skin,
			uint32 decalLimit);

		virtual ~CMashDynamicDecal();

		mash::MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		eMASH_STATUS AppendVertices(const MashTriangleCollider *pTriangleCollection,
				const sTriPickResult &collisionResult,
				const mash::MashVector2 &vTextureDim,
				f32 fRotation,
				const mash::MashMatrix4 *pTransformation = 0);

		uint32 GetDecalCount()const;
	};

	inline uint32 CMashDynamicDecal::GetDecalCount()const
	{
		return m_decalCount;
	}
}

#endif