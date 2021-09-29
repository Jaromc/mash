//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MashQuaternion_H_
#define _MASH_MashQuaternion_H_

#include "MashCompileSettings.h"
#include "MashDataTypes.h"

namespace mash
{
	class MashMatrix4;
	class MashVector3;

	/*!
        Defines a quaternion used for rotations.
    */
	class _MASH_EXPORT MashQuaternion
	{
	public:
		union
		{
			struct
			{
				f32 x, y, z, w;
			};

			f32 v[4];
		};
		
	public:
		MashQuaternion();
		MashQuaternion(f32 nw, f32 nx, f32 ny, f32 nz);
        
        _MASH_EXPORT friend MashQuaternion operator*(f32 a, const MashQuaternion &b);
        
		//! Cross product.
		MashQuaternion& operator*=(const MashQuaternion &q);
        
        //! Addition.
		MashQuaternion& operator+=(const MashQuaternion &q);
        
        //! Subtraction.
		MashQuaternion& operator-=(const MashQuaternion &q);
        
        //! Multiplies all elements by this value.
		MashQuaternion& operator*=(f32 value);
        
        //! Divides all elements by this value.
		MashQuaternion& operator/=(f32 value);
        
        //! Subtraction.
		MashQuaternion operator-(const MashQuaternion &other)const;
        
        //! Cross product.
		MashQuaternion operator*(const MashQuaternion &other)const;
        
        //! Addition.
		MashQuaternion operator+(const MashQuaternion &other)const;
		
        //! Multiplies all elements by this value.
		MashQuaternion operator*(f32 f)const;
        
        //! Equals to.
		bool operator== (const MashQuaternion &q)const;
        
        //! Not equals to.
		bool operator!= (const MashQuaternion &q)const;

		//! Conjugate of this MashQuaternion.
		MashQuaternion operator~()const;

		//! Sets this quaternion to an identity quaternion.
		void Identity();
        
		//! Rotate about the X axis by theta.
		/*!
			\param theta Rotation in radians.
		*/
		void SetRotationX(f32 theta);

		//! Rotate about the Y axis by theta.
		/*!
			\param theta Rotation in radians.
		*/
		void SetRotationY(f32 theta);
		
		//! Rotate about the Z axis by theta.
		/*!
			\param theta Rotation in radians.
		*/
		void SetRotationZ(f32 theta);

		//! Set the rotation axis and angle.
		/*!
			\param axis Axis of rotation.
			\param theta Rotation in radians.
		*/
		void SetRotationAxis(const MashVector3 &axis, f32 theta);

		//! Adds the vector to this quaterion. Influenced by scale.
		/*!
			\param vector Vector to add.
			\param scale Number to scale the vector by.
		*/
		void AddScaledVector(const MashVector3 &vector, f32 scale);

		//! Normalize this quaternion.
		void Normalize();

		//! Gets the magnitude.
		/*!
            This length of x,y,z,w elements combined.
         
            \return magnitude of this quaternion.
        */
		f32 Magnitude();

		//! Gets the angle of rotation.
		/*!
            \return Angle of rotation.
        */
		f32 GetRotationAngle()const;

		//! Gets the axis of rotation.
		/*!
            \return Axis of rotation.
        */
		MashVector3 GetRotationAxis()const;

		//! Returns a vector representing this quaternion as Euler angles.
		/*!
            \param outVal Euler representation in pitch, yaw, roll.
        */
		void ToEulerAngles(MashVector3 &outVal)const;

		//! Returns a matrix representing this MashQuaternion as a rotation matrix.
		/*!
            \param outVal Matrix representation.
        */
		void ToMatrix(MashMatrix4 &outVal)const;

		//! MashQuaternion dot product. 
		/*!
            \param other Other MashQuaternion to perform a dit product with.
            \return Dot product of this MashQuaternion with another.
        */
		f32 DotProduct(const MashQuaternion &other)const;

		//! Spherical interpolation.
		/*!
			Sets this quaternion to the result of a slerp operation
			from a to b as t varies from 0 to 1.
         
			\param a Start.
			\param b End.
			\param t Slerp amount.
            \return This result.
		*/
		MashQuaternion& Slerp(const MashQuaternion &a, const MashQuaternion &b, f32 t);
        
        //! Linear interpolation.
        /*!
            Sets this quaternion to the result of linear interpolation between a and b.
         
            \param a Start.
            \param b End.
            \param t Slerp amount.
            \return This result.
        */
        MashQuaternion& Lerp(const MashQuaternion &a, const MashQuaternion &b, f32 t);

		//! Equivalent to MashQuaternion inverse for unit MashQuaternions.
		/*!
            Perfoms the operation on this MashQuaternion.
         
            \return This MashQuaternion.
        */
		MashQuaternion& Conjugate();

		//! Raises to the power of 'exponent'.
		/*!
			Used to extract a fraction of an angular displacement.
			Sets this MashQuaternion to the result.
         
			\param q MashQuaternion to raise.
			\param exponent Amount to raise.
		*/
		void Exponentiation(const MashQuaternion &q, f32 exponent);
        
        //! Transforms a vector by this rotation.
        /*!
            \param vector to Transform.
            \return Transformed vector.
         */
        MashVector3 TransformVector(const MashVector3 &vector)const;

        //! Sets the rotation from euler angles in degrees.
        /*!
            \param x Pitch.
            \param y Yaw.
            \param z Roll.
        */
		void SetEuler(f32 x, f32 y, f32 z);
        
        //! Sets the rotation from euler angles in degrees.
        /*!
            \param angles Angles in pitch yaw and roll. 
        */
		void SetEuler(const MashVector3 &angles);

        //! Creates a rotation from one direction to another.
        /*!
            \param from Start direction.
            \param to End direction.
        */
		void RotateTo(const MashVector3 &from, const MashVector3 &to);
	};
}
#endif