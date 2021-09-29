//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SPHERE_H_
#define _MASH_SPHERE_H_

#include "MashCompileSettings.h"
#include "MashVector3.h"

namespace mash
{
    /*!
        Defines a sphere.
    */
	class _MASH_EXPORT MashSphere
	{
	public:
		f32 radius;
		MashVector3 center;
	public:
		MashSphere();
		MashSphere(const MashSphere &copy);
        
        //! Constructor.
        /*!
            \param r Sphere radius.
            \param c Sphere center.
        */
		MashSphere(f32 r, const MashVector3 &c);
	};
}

#endif