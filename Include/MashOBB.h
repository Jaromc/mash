//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_OBB_H_
#define _MASH_OBB_H_

#include "MashCompileSettings.h"
#include "MashVector3.h"

namespace mash
{
	class MashQuaternion;
	class MashMatrix4;

    /*!
        Oriented bounding box.
    */
	class _MASH_EXPORT MashOBB
	{
	public:
		MashVector3 center;
		MashVector3 localAxis[3];
		MashVector3 halfWidth;

		MashOBB();

		MashOBB(const MashOBB &OBB);

        //! Constructor.
        /*!
            \param center Center of the obb.
            \param x,y,z Axis vectors.
            \param halfWidth Half width vector.
        */
		MashOBB(const MashVector3 &center,
			const MashVector3 &x,
			const MashVector3 &y,
			const MashVector3 &z,
			const MashVector3 &halfWidth);

		
		~MashOBB();

		//! Transform this obb by a matrix.
		MashOBB& Transform(const MashMatrix4 &m);
        
        //! Transforms this obb by a quaternion.
		MashOBB& Transform(const MashQuaternion &q);

        //! Calculates the closest point on this obb to a given point.
        /*!
            \param point Point to test.
            \param closestPointOut Closest point on this obb to the point.
        */
		void ClosestPoint(const MashVector3 &point, MashVector3 &closestPointOut)const;
	};
}

#endif