//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_VECTOR4_H_
#define _MASH_VECTOR4_H_

#include "MashDataTypes.h"

namespace mash
{
	class MashVector3;

    /*!
        4D vector.
    */
	class _MASH_EXPORT MashVector4
	{
	public:
		union
		{
			struct
			{
				f32 x, y, z,w;
			};
			f32 v[4];
		};
	public:
		MashVector4();
		MashVector4(f32 nx, f32 ny, f32 nz, f32 nw);
		MashVector4(f32 nx, f32 ny, f32 nz);
		MashVector4(const MashVector3 &v);
		MashVector4(const MashVector4 &v);
        
        //! Copy operator.
		MashVector4& operator=(const MashVector3 &v);

		//! Copy operator.
		MashVector4& operator=(const MashVector4 &v);

        //! Equals to.
		bool operator== (const MashVector4 &v)const;
        
        //! Returns a const element.
		f32 operator[](int32 i)const;
		
        //! Vector multiplication.
		MashVector4 operator*(const MashVector4 &other)const;

        //! Sets all elements to zero.
		MashVector4& Zero();
        
        //! Normalizes this vector.
		MashVector4& Normalize();
	};
}

#endif