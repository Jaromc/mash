//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TRIANGLE_COLLIDER_H_
#define _MASH_TRIANGLE_COLLIDER_H_

#include "MashReferenceCounter.h"
#include "MashTypes.h"
#include "MashArray.h"

namespace mash
{
	class MashGenericArray;
	class MashTriangleBuffer;
	class MashTransformState;
	class MashMatrix4;
	class MashRay;
	class MashAABB;
	class MashTriangle;

    /*!
        Triangle colliders are used for collision detection and are created from
        MashSceneManager::CreateTriangleCollider().
     
        Colliders may contain many triangle buffers to describe a models bounds.
    */
	class MashTriangleCollider : public MashReferenceCounter
	{
	public:
		MashTriangleCollider():MashReferenceCounter(){}
		virtual ~MashTriangleCollider(){}

		//! Gets intersecting triangles within an AABB.
		/*!
			This method can be used as a broad phase collision method. The returned triangles
			can then be examined closer for collision details.

			\param bounds Bounds in world space.
			\param transform The world space trasform of the object that owns this collider.
			\param out Intersecting triangles will be added here. Note, this function will not initially clear the array.
			\return True if any triangles were added to the array. False otherwise.
		*/
		virtual bool GetIntersectingTriangles(const MashAABB &bounds, 
			const MashTransformState &transform, 
			MashArray<sIntersectingTriangleResult> &out)const = 0;

        //! Collision test against a ray.
        /*!
            \param ray Ray to test in world space.
            \param transform The world space trasform of the object that owns this collider.
            \return True if the ray intersects any triangles. False otherwise.
        */
		virtual bool CheckCollision(const MashRay &ray, 
			const MashTransformState &transform)const = 0;

        //! Gets the closest triangle to a ray.
        /*!
            \param ray Ray to test in world space.
            \param transform The world space trasform of the object that owns this collider.
            \param out Closest triangle that intersects the ray.
            \return True if the ray intersects any triangles. False otherwise.
        */
		virtual bool GetClosestTriangle(const MashRay &ray,
			const MashTransformState &transform,
			sTriPickResult &out)const = 0;

        //! Gets the triangles that intersect a ray.
        /*!
            \param ray Ray to test in world space.
            \param transform The world space trasform of the object that owns this collider.
            \param out A list of triangles that intersects the ray.
            \return True if the ray intersects any triangles. False otherwise.
        */
		virtual bool GetIntersectingTriangles(const MashRay &ray,
			const MashTransformState &transform,
			MashArray<sTriPickResult> &out)const = 0;

        //! Gets the triangle buffers in this collection. This list must not be modified.
		virtual const MashArray<MashTriangleBuffer*>& GetTriangleBufferCollection()const = 0;
        
        //! Gets a buffer from this collection.
        /*!
            \param index Index from 0 - (GetTriangleBufferCount() - 1).
            \return Triangle buffer.
        */
		virtual const MashTriangleBuffer* GetTriangleBuffer(uint32 index)const = 0;
        
        //! Gets the triangle buffer count.
		virtual uint32 GetTriangleBufferCount()const = 0;
        
        //! Gets the collider type.
		virtual eTRIANGLE_COLLIDER_TYPE GetColliderType()const = 0;

		//! Writes the internal spacial structure to an array for file saving.
		/*!
			If this collider contains a tree or spacial information then its data
			is written to the array. This array can then be written to file to
			make application loading faster.

			The data can be loaded using Deserialize().

			It is valid for a collider to write no data if the triangles are not sorted
			in any way.

			\param out Array to write data to.
		*/
		virtual void Serialize(MashGenericArray &out)const = 0;

		//! Loads spacial data into the collider.
		/*!
			Loads an array created from Serialize(). This can be used to reduce calculations
			made at runtime therefore making an applications load time faster.
			
			\param dataArray Data saved from Serialize().
			\param bytesRead This counter will be advanced by the number of read.
		*/
		virtual void Deserialize(const uint8 *dataArray, uint32 &bytesRead) = 0;
	};
}

#endif