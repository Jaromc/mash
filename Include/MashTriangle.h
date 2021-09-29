//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TRIANGLE_H_
#define _MASH_TRIANGLE_H_

#include "MashCompileSettings.h"
#include "MashVector3.h"

namespace mash
{
	class MashRay;

	/*!
        Defines a 3D triangle with some useful methods.
    */
	class _MASH_EXPORT MashTriangle
	{
	public:
		MashVector3 pointA,pointB,pointC;

	public:
		//! Constructor
		MashTriangle();

		//! Copy Constructor
		MashTriangle(const MashTriangle &copy);

		//! Constructor
		/*!
			\param a Point A
			\param b Point B
			\param c Point C
		*/
		MashTriangle(const MashVector3 &a,
			const MashVector3 &b,
			const MashVector3 &c);

		//! Intersection test against a ray.
		/*!
			\param ray Ray to do test with.
			\param collisionPoint Point of intersection.
			\return True if an intersection has occured.
		*/
		bool Intersects(const MashRay &ray, MashVector3 &collisionPoint)const;

		//! Finds the closest point on this triangle to a given point.
		/*!
			\param point Point to test.
			\param closestPoint This is set to the result of finding the closest point.
		*/
		void ClosestPoint(const MashVector3 &point, MashVector3 &closestPoint)const;

		//! Index operator that allows modifying points.
		/*! 
			\param i - 0 PointA.
					- 1 PointB.
					- 2 PointC.
			\return Points a, b, or c.
		*/
		MashVector3& operator[](int32 i);
        
        //! Index operator.
		/*! 
            \param i - 0 PointA.
            - 1 PointB.
            - 2 PointC.
            \return Points a, b, or c.
         */
		const MashVector3& operator[](int32 i)const;

        //! Generates a normal for this triangles face.
		MashVector3 GenerateNormal()const;
	};
}

#endif