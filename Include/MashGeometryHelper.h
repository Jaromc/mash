//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GEOMETRY_HELPER_H_
#define _MASH_GEOMETRY_HELPER_H_

#include "MashDataTypes.h"

namespace mash
{
	class MashAABB;
	class MashSphere;
	class MashPlane;
	class MashRay;
	class MashOBB;
	class MashTriangle;
	class MashVector3;

	/*!
		Methods for basic collision detection.
        These methods return true if intersecting OR touching.
	*/
	namespace collision
	{
		bool AABB_AABB(const MashAABB &a, const MashAABB &b); 
		bool AABB_Sphere(const MashAABB &a, const MashSphere &b);
		bool AABB_OBB(const MashAABB &a, const MashOBB &b);
		bool AABB_Plane(const MashAABB &a, const MashPlane &b); 
		bool AABB_Triangle(const MashAABB &aabb, const MashVector3 &pointA, const MashVector3 &pointB, const MashVector3 &pointC); 

		bool Ray_AABB(const MashAABB &a, const MashRay &b);
		bool Sphere_Sphere(const MashSphere &a, const MashSphere &b); 
		bool Sphere_OBB(const MashSphere &a, const MashOBB &b);
		bool Sphere_Plane(const MashSphere &a, const MashPlane &b); 

		bool OBB_OBB(const MashOBB &a, const MashOBB &b);
		bool OBB_Plane(const MashOBB &a, const MashPlane &b);
        
        bool Ray_Triangle(const MashVector3 &pointA, const MashVector3 &pointB, const MashVector3 &pointC, const MashRay &ray);
		bool Ray_Triangle(const MashTriangle &a, const MashRay &b);

        /*!
            \param t Distance from the ray position to the aabb.
        */
		bool Ray_AABB(const MashAABB &a, const MashRay &b, f32 &t);
        
        /*!
            \param t Distance from the ray position to the sphere.
        */
		bool Ray_Sphere(const MashSphere &a, const MashRay &b, f32 &t);
        
        /*!
            \param t Distance from the ray position to the obb.
        */
		bool Ray_OBB(const MashOBB &a, const MashRay &b, f32 &t);
        
		/*!
            Returns the barycentric coord of the intersection point on the triangle
            in u, v, and w. T is the distance from the ray position to the triangle. 
        */
		bool Ray_Triangle(const MashTriangle &a, const MashRay &b, 
			f32 &u, f32 &v, f32 &w, f32 &t);
        
        /*!
            Returns the barycentric coord of the intersection point on the triangle
            in u, v, and w. T is the distance from the ray position to the triangle. 
        */
		bool Ray_Triangle(const MashVector3 &pointA, const MashVector3 &pointB, const MashVector3 &pointC, const MashRay &ray, 
			f32 &u, f32 &v, f32 &w, f32 &t);
        
        /*!
            \param t Distance from the ray position to the obb.
        */
		bool Ray_Plane(const MashPlane &a, const MashRay &b, f32 &t);

		//! Conveniance function. Calls Ray_AABB
		f32 GetDistanceToAABB(const mash::MashVector3 &vPosition, const MashAABB &aabb);
		
	}
}

#endif