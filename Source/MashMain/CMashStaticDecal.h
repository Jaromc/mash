//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_STATIC_DECAL_SCENE_NODE_H_
#define _C_MASH_STATIC_DECAL_SCENE_NODE_H_

#include "CMashDecalIntermediate.h"

namespace mash
{
	class CMashStaticDecal : public CMashDecalIntermediate
	{
	public:
		CMashStaticDecal(MashSceneNode *parent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			const MashStringc &sName,
			MashMaterial *pMaterial,
			MashSkin *skin);

		virtual ~CMashStaticDecal();

		mash::MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		eMASH_STATUS AppendVertices(const MashTriangleCollider *pTriangleCollection,
				const sTriPickResult &collisionResult,
				const mash::MashVector2 &vTextureDim,
				f32 fRotation,
				const mash::MashMatrix4 *pTransformation = 0);

		uint32 GetDecalCount()const;

		void Draw();
	};
}

#endif