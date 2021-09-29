//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_VECTOR2_H_
#define _MASH_VECTOR2_H_

#include "MashDataTypes.h"

namespace mash
{
	/*!
        2D vector class
    */
	class _MASH_EXPORT MashVector2
	{
	public:
		union
		{
			struct
			{
				f32 x, y;
			};
			f32 v[2];
		};
	public:
		MashVector2();
		MashVector2(f32 nx, f32 ny);

        //! Vector adition.
		MashVector2 operator+(const MashVector2 &v)const;
        
        //! Vector subtraction.
		MashVector2 operator-(const MashVector2 &other)const;
        
        //! Equals to.
		bool operator==(const MashVector2 &other)const;
        
        //! Mutiplies each element by a f32.
		MashVector2 operator*(f32 f)const;

        //! Returns the length of this vector.
		f32 Length()const;
        
        //! Sets this vector to zero.
		MashVector2& Zero();
	};
}

#endif