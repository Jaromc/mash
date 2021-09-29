//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_DECAL_H_
#define _MASH_DECAL_H_

#include "MashRenderable.h"
#include "MashSceneNode.h"

namespace mash
{
	class MashSkin;
	class MashTriangleCollider;
    
    /*!
        Decal nodes are objects that batch render many decals. They can be attached to
        other scene nodes so the decals transform with a node.
    */
	class MashDecal : public MashSceneNode, public MashRenderable
	{
	public:
		MashDecal(MashSceneNode *parent,
			mash::MashSceneManager *manager,
			const MashStringc &name):MashSceneNode(parent, manager, name),
			MashRenderable(){}

		virtual ~MashDecal(){}

		//! Returns the skin used by this decal.
		/*!
			This is only needed for decals applied to animated objects for skinning.
			\param Skin used by this decal.
		*/
		virtual MashSkin* GetSkin()const = 0;

        //! Appends a new decal to a list.
		/*!
            Creates a new decal based on the collision data passed in.
            The transform pointer is only really needed for static meshes
            so the decal can be transformed into world space.
         
            \param triangleCollection Tri collection of the mesh to which this decal will be applied.
            \param collisionResult Information on where the decal will be applied.
            \param textureDim The width and height of the new decal.
            \param rotation Rotates the new decal in radians.
            \param transform Can be NULL. World transform for static meshes.
        */
		virtual eMASH_STATUS AppendVertices(const MashTriangleCollider *triangleCollection,
			const sTriPickResult &collisionResult,
			const MashVector2 &textureDim,
			f32 rotation,
			const MashMatrix4 *transformation = 0) = 0;

		//! Returns the current decal count.
		/*!
			\return Current decal count.
		*/
		virtual uint32 GetDecalCount()const = 0;

		//! Merge tolerance in radians.
		/*!
			Triangles with a facing direction greater than this angle,
			relative to the colliding triangle, will not be added to create the
			final decal.
			This stops incorrect lighting and bad texture mapping.
			45 degress is a good starting point.

			\param angle Angle in radians.
		*/
		virtual void SetMergeTolerance(f32 angle) = 0;
	};
}

#endif