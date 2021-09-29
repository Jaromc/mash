//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_DECAL_INTERMEDIATE_H_
#define _C_MASH_DECAL_INTERMEDIATE_H_

#include "MashDecal.h"
#include "MashMeshBuffer.h"
#include "MashArray.h"
namespace mash
{
	class MashSceneManager;
	class MashTriangleBuffer;
	class MashVideo;	

	class CMashDecalIntermediate : public MashDecal
	{
	protected:
		struct sClipPoint
		{
			mash::MashVector3 position;
			mash::MashVector3 normal;

			sClipPoint(){}
			sClipPoint(const mash::MashVector3 &p, const mash::MashVector3 &n):position(p), normal(n){}
		};

		struct sVertexData
		{
			uint8 positionElementLocation;
			uint8 normalElementLocation;
			uint8 textureElementLocation;
			uint8 blendWeightElementLocation;
			uint8 blendIndexElementLocation;

			uint8 positionElementSize;
			uint8 normalElementSize;
			uint8 texcoordElementSize;
			uint8 blendWeightElementSize;
			uint8 blendIndexElementSize;
            
            sVertexData():positionElementLocation(mash::math::MaxUInt8()),
            normalElementLocation(mash::math::MaxUInt8()),
            textureElementLocation(mash::math::MaxUInt8()),
            blendWeightElementLocation(mash::math::MaxUInt8()),
            blendIndexElementLocation(mash::math::MaxUInt8()),
            positionElementSize(mash::math::MaxUInt8()),
            normalElementSize(mash::math::MaxUInt8()),
            texcoordElementSize(mash::math::MaxUInt8()),
            blendWeightElementSize(mash::math::MaxUInt8()),
            blendIndexElementSize(mash::math::MaxUInt8()){}
		};

		mash::MashVideo *m_pRenderer;
		MashMaterial *m_pMaterial;
		uint32 m_iVertexCount;
		ePRIMITIVE_TYPE m_ePrimitiveType;
		uint32 m_iPrimitiveCount;
		mash::MashAABB m_aabb;

		MashMeshBuffer *m_meshBuffer;
		bool m_bIsVisible;
		MashSkin *m_skin;

		uint32 m_iReservedSizeInBytes;
		void *m_pVertices;

		f32 m_mergeTolerance;

		void CreateNonSkinnedDecalVertices(const MashTriangleBuffer *triangleBuffer,
			const MashArray<uint32> &decalTriangles,
			const MashArray<mash::MashPlane> &clippingPlanes,
				const mash::MashVector3 &vHitPoint,
				const mash::MashVector3 &vCollisionNormal,
				const mash::MashVector2 &vTextureDim,
				const mash::MashVector3 &vTangent,
				const mash::MashVector3 &vBiTangent,
				const mash::MashMatrix4 *pTransformation,
				const sVertexData &vertexData,
				uint32 &verticesAdded);

		void CreateSkinnedDecalVertices(const MashTriangleBuffer *triangleBuffer,
				const MashArray<uint32> &decalTriangles,
				const mash::MashVector3 &vHitPoint,
				const mash::MashVector3 &vCollisionNormal,
				const mash::MashVector2 &vTextureDim,
				const mash::MashVector3 &vTangent,
				const mash::MashVector3 &vBiTangent,
				const mash::MashMatrix4 *pTransformation,
				const sVertexData &vertexData,
				uint32 &verticesAdded);

		bool Clip(const MashArray<mash::MashPlane> &clippingPlanes,
			MashArray<sClipPoint> &pointsToClip,
			MashArray<sClipPoint> &clippedPoints,
			MashArray<sClipPoint> &triangleListOut);

		void GetVertexData(sVertexData &out);

		void OnPassCullImpl(f32 interpolateAmount){}
		void InstanceDecalMembers(CMashDecalIntermediate *from);
		virtual void OnAddNewDecal(){}
	public:
		CMashDecalIntermediate(MashSceneNode *parent,
			MashSceneManager *pSceneManager,
			mash::MashVideo *pRenderer,
			const MashStringc &sName,
			MashMaterial *pMaterial,
			MashSkin *skin);

		virtual ~CMashDecalIntermediate();

		virtual void Draw();
		virtual uint32 GetNodeType()const;

		bool ContainsRenderables()const;

		eMASH_STATUS _AppendVertices(const MashTriangleCollider *pTriangleCollection,
				const sTriPickResult &collisionResult,
				const mash::MashVector2 &vTextureDim,
				f32 fRotation,
				const mash::MashMatrix4 *pTransformation,
				const sVertexData &vertexData,
				uint32 &verticesAdded);

		MashMeshBuffer* GetMeshBuffer()const{return m_meshBuffer;}
		MashMaterial* GetMaterial()const{return m_pMaterial;}
		const mash::MashAABB& GetLocalBoundingBox()const{return m_aabb;}
		const mash::MashAABB& GetWorldBoundingBox()const;
		const mash::MashAABB& GetTotalWorldBoundingBox()const;
		
		void ResizeVertexBuffer(uint32 iNewSizeInBytes);
		void SetMaximumDecalLimit(uint32 limit);

		bool AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr);

		void SetMergeTolerance(f32 angle);

		MashSkin* GetSkin()const;
	};

	inline void CMashDecalIntermediate::SetMergeTolerance(f32 angle)
	{
		m_mergeTolerance = angle;
	}

	inline bool CMashDecalIntermediate::ContainsRenderables()const
	{
		return true;
	}

	inline MashSkin* CMashDecalIntermediate::GetSkin()const
	{
		return m_skin;
	}

	inline uint32 CMashDecalIntermediate::GetNodeType()const
	{
		return aNODETYPE_DECAL;
	}
}

#endif