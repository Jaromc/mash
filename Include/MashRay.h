//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_RAY_H_
#define _MASH_RAY_H_

#include "MashCompileSettings.h"
#include "MashVector3.h"

namespace mash
{
	class MashMatrix4;
	class MashTransformState;

	/*!
        Defines a 3D ray with some useful methods.
    */
	class _MASH_EXPORT MashRay
	{
	private:

	public:
		MashVector3 origin;
		MashVector3 dir;

		//! Constructor.
		MashRay();

		//! Constructor.
		/*!
			\param _origin Ray origin.
			\param _dir Ray direction.
		*/
		MashRay(const MashVector3 &_origin,
			const MashVector3 &_dir);

		//! Copy constructor.
		MashRay(const MashRay &ray);

		//! Sets the ray to the parameters.
		/*!
			\param _origin Ray origin.
			\param _dir Ray direction.
		*/
		void Set(const MashVector3 &_origin,
			const MashVector3 &_dir);

		//! Transforms the ray into the coordinate space of the matrix.
		/*!
			\param matrix Transformation matrix.
		*/
		void Transform(const MashMatrix4 &matrix);
        
        //! Transforms the ray by the transform state.
        /*!
            \param state Transform state.
        */
		void Transform(const MashTransformState &state);
        
        //! Transforms the ray by the inverse of the transform state.
        /*!
            \param state Transform state.
        */
		void TransformInverse(const MashTransformState &state);

		//! Intersection test against a triangle.
		/*!
			\param a, b, c Triangle points
			\param cull True if using backface culling.
			\param t Distance from the ray to point of intersection.
			\return True if intersection occured.
		*/
		bool Intersects(const MashVector3 &a,
			const MashVector3 &b,
			const MashVector3 &c,
			bool cull = false,
			f32 *t = 0)const;
		
		//! Intersection test against a plane.
		/*!
			\param planeNormal Planes normal.
            \param planeD Plane distance from the origin.
			\param cull True if using backface culling.
			\param t Distance from the ray to point of intersection.
			\return True if intersection occured.
		*/
		bool Intersects(const MashVector3 &planeNormal,
			f32 planeD,
			bool cull = false,
			f32 *t = 0)const;

		//! Intersection test against an AABB.
		/*!
			\param min Min of the aabb.
            \param max Max of the aabb.
			\return True if an intersection has occured.
		*/
		bool Intersects(const MashVector3 &min,
			const MashVector3 &max)const;
	};
}

#endif