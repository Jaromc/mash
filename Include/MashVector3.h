//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_VECTOR3_H_
#define _MASH_VECTOR3_H_

#include "MashDataTypes.h"

namespace mash
{
	/*!
        3D vector class
    */
	class _MASH_EXPORT MashVector3
	{
	public:
		union
		{
			struct
			{
				f32 x, y, z;
			};
			f32 v[3];
		};
		
	public:
		MashVector3():x(0.f), y(0.f),z(0.f){}
		MashVector3 (f32 nx, f32 ny, f32 nz);

        //! Copy operator.
		MashVector3& operator=(const MashVector3 &v);
        
        //! Equals to.
		bool operator== (const MashVector3 &v)const;
        
        //! Not equal to.
		bool operator!= (const MashVector3 &v)const;
        
        //! Vector subtraction.
		MashVector3 operator-(const MashVector3 &v)const;
        
        //! Negates all elements.
		MashVector3 operator-()const;
        
        //! Vector addition.
		MashVector3 operator+(const MashVector3 &v)const;
        
        //! Vector division.
		MashVector3 operator/(const MashVector3 &v)const;
        
        //! Adds a f32 to all elements.
		MashVector3 operator+(f32 a)const;
        
        //! Multiplies all element by a f32.
		MashVector3 operator*(f32 a)const;
        
        //! Divides all element by a f32.
		MashVector3 operator/(f32 a)const;
        
        //! Vector adition.
		MashVector3& operator+=(const MashVector3 &v);
        
        //! Vector subtraction.
		MashVector3& operator-=(const MashVector3 &v);
        
        //! Vector multiplication.
		MashVector3& operator*=(const MashVector3 &v);
        
        //! Vector divsion.
		MashVector3& operator/=(const MashVector3 &v);
        
        //! Adds a f32 to all elements.
		MashVector3& operator+=(f32 a);
        
        //! Multiplies all elements by a f32.
		MashVector3& operator*=(f32 a);
        
        //! Divides all elements by a f32.
		MashVector3& operator/=(f32 a);
        
        //! Returns a const element.
		f32 operator[](uint32 i)const;

        //! Vector multiplication.
		MashVector3 operator*(const MashVector3 &other)const;

		//! Vector dot product.
		f32 Dot(const MashVector3 &v)const;

		//! Normalize this vector.
		MashVector3& Normalize();

		//! Normalizes the vector then returns the length.
		f32 NormalizeAndGetLength();

		//! Return the length of this vector.
		f32 Length()const;

		//! Returns the length of this vector squared.
		f32 LengthSq()const;

		//! Return the cross product of this vector with other.
		MashVector3 Cross(const MashVector3 &other)const;

		//! Sets all the elements to zero.
		MashVector3& Zero();

		//! Gets the distance from this vector to another.
		/*!
			\param other Vector to use for calculation.
			\return Distance from this vector to the param.
		*/
		f32 GetDistanceTo(const MashVector3 &other)const;

		//! Gets the distance squared from this vector to another.
		/*!
			\param other Vector to use for calculation.
			\return Distance from this vector to the param.
		*/
		f32 GetDistanceToSQ(const MashVector3 &other)const;

		//! Returns true if all the elements are set to zero.
		bool IsZero()const;

		//! Performs linear interpolation from this vector to the target.
		/*! 
			\param target target to perform lerp.
			\param alpha amount in which to lerp.
		*/
		MashVector3 Lerp(const MashVector3 &target, f32 alpha)const;
	};

	MashVector3 operator*(f32 lhs, const MashVector3 &rhs);
}

#endif