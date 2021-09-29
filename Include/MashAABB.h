//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_AABB_H_
#define _MASH_AABB_H_

#include "MashCompileSettings.h"
#include "MashVector3.h"
#include "MashArray.h"

namespace mash
{
	class MashPlane;
	class MashMatrix4;
	class MashTransformState;

	//! Defines an axis aligned 3D box.
	class _MASH_EXPORT MashAABB
	{
	public:
		MashVector3 min;
		MashVector3 max;

		//! Constructor.
		MashAABB();

		//! Constructor.
		/*!
			\param _min Min value.
			\param _max Max value.
		*/
		MashAABB(const MashVector3 &_min,
			const MashVector3 &_max);

		//! Copy Constructor.
		/*!
			\param AABB Aabb to copy.
		*/
		MashAABB(const MashAABB &AABB);

		//! Get the planes that make up this AABB.
		/*!
			\param outArray Empty container where all the planes will be added.
		*/
		void GetPlanes(MashArray<MashPlane> &outArray)const;

		//! Gets all corners that make up this AABB.
		/*!
			\param outArray Array of 8 MashVector3's
		*/
		void GetVerticies(MashVector3 *outArray)const;

		//! Sets the limits
		/*!
			\param _min New min value.
			\param _max New max value.
		*/
		void SetLimits(const MashVector3 &_min,
						const MashVector3 &_max);

		//! Merges this AABB with another.
		/*!
			This AABB will automatically grow if necessary.
         
			\param AABB Other box to merge with.
		*/
		void Merge(const MashAABB &AABB);

		//! Intersection test with another AABB.
		/*!
			\param other Other AABB to perform test against.
			\return True if this AAB intersects with the other.
		*/
		bool Intersects(const MashAABB &other)const;

		//! Intersection test with a point.
		/*!
			\param point Point to test.
			\return True if the point is inside or touching this AABB.
		*/
		bool Intersects(const MashVector3 &point)const;

		//! Scales this AABB.
		/*!
			\param scale Scale to apply to this AABB.
		*/
		void Scale(const MashVector3 &scale);

		//! Repairs this AABB.
		/*!
			Checks the min and max values are valid and repairs
			if necessary.
		*/
		void Repair();

		//! Transforms this AABB.
		/*!
			\param matrix Transformation matrix.
			\return This AABB.
		*/
		MashAABB& Transform(const MashMatrix4 &matrix);

		//! Transforms this AABB.
		/*!
			\param state Transformation state.
			\return This AABB.
		*/
		MashAABB& Transform(const MashTransformState &state);

		//! Transforms this AABB by the inverse of the state.
		/*!
			\param state Transformation state.
			\return This AABB.
		*/
		MashAABB& TransformInverse(const MashTransformState &state);

		//! Transforms this AABB and returns the result.
		/*!
			\param matrix Transformation matrix.
			\param transformedOut Transformation result.
		*/
		void Transform(const MashMatrix4 &matrix, MashAABB &transformedOut)const;

		//! Zeros out all elements,
		void Zero();

		//! Returns the center of this bounds.
		/*!
			\param centerOut Center position.
		*/
		void GetCenter(MashVector3 &centerOut)const;

		//! Returns the center of this bounds.
		/*!
			\return Center position.
		*/
		MashVector3 GetCenter()const;

		//! Returns the closest point on this bounds to the given point.
		/*!
			\param point The position returned will be from this point to the aabb.
			\paran closestPointOut Closest point result on the aabb.
		*/
		void ClosestPoint(const MashVector3 &point, MashVector3 &closestPointOut)const;

		//! Adds or merges a point to the aabb.
		/*!
			\param x X coord.
			\param y Y coord.
			\param z Z coord.
		*/
		void AddPoint(f32 x, f32 y, f32 z);

		//! Adds or merges a point to the aabb.
		/*!
            The AABB will grow out if necessary.
         
			\param point New point to add to this box.
		*/
		void Add(const MashVector3 &point);
	};
}

#endif