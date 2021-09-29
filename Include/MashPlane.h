//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PLANE_H_
#define _MASH_PLANE_H_

#include "MashCompileSettings.h"
#include "MashEnum.h"
#include "MashVector3.h"

namespace mash
{
	class MashLine3;

	/*!
        Defines a 3D plane with some useful methods.
    */
	class _MASH_EXPORT MashPlane
	{
	public:
		//! Normal of the plane.
		MashVector3 normal;
		//! Distance from the origin.
		f32 dist;
		//! Plane thickness.
		f32 thickness;
	public:
		//! Constructor.
		MashPlane();

		//! Constructor.
		/*!
			\param callNormalize Set to false if you guarantee _normal is already normalized.
		*/
		MashPlane(const MashVector3 &_normal,
			const MashVector3 &_point,
			f32 _thickness = 0.1f,
			bool callNormalize = true);

		//! Constructor.
		/*!
			Constructs a plane from 3 points.
			\param a Point A.
			\param b Point B.
			\param c Point C.
			\param _thinckness Thickness of the plane. Good for resolving precision errors.
		*/
		MashPlane(const MashVector3 &a,
			const MashVector3 &b,
			const MashVector3 &c,
			f32 _thinckness = 0.1f);

		//! Calculates the distance from the point to the plane,
		/*!
			\param point Point to test.
			\return Distance to this plane.
		*/
		f32 Distance(const MashVector3 &point)const;

		//! Determines if a point is infront, on, or behind this plane.
		/*!
			\param point Point to classify.
			\return - 0 if the point is in front.
					- 1 if the point is behind.
					- 2 if the point is straddling.
		*/
		eCLASSIFY Classify(const MashVector3 &point)const;

		//! Determines if a polygon is infront, behind, coplanar, or straddling with respect to this plane.
		/*!
			\param pointList Polygon points to classify.
            \param pointCount Number of points in the point array.
			\return - 0 if the poly is in front.
					- 1 if the poly is behind.
					- 2 if the poly is straddling.
					- 3 if the poly is coplaner.
		*/
		eCLASSIFY Classify(const MashVector3 *pointList, uint32 pointCount)const;

		//! Intersection test against a line segment.
		/*!
			\param line Line to test.
			\param t Returned distance to intersection from the lines start point.
			\return True if the segment intersects this plane.
		*/
		bool Intersect(const MashLine3 &line, f32 &t)const;

        //! Intersection test against a line segment.
		/*!
            \param start Line start.
            \param end Line end.
            \param t Returned distance to intersection from the lines start point.
            \return True if the segment intersects this plane.
         */
		bool Intersect(const MashVector3 &start, const MashVector3 &end, f32 &t)const;
	};
}

#endif