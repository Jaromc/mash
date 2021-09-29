//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MashMatrix4_H_
#define _MASH_MashMatrix4_H_

#include "MashCompileSettings.h"
#include "MashEnum.h"

namespace mash
{
	class MashVector2;
	class MashVector3;
	class MashVector4;
	class MashQuaternion;

	/*!
        Defines a transformation matrix stored as row vectors.
        Elements m41, m42, m43 contain the translation portion.
    */
	class _MASH_EXPORT MashMatrix4
	{
	public:
		union
		{
			struct
			{
				f32 m11, m12, m13, m14;
				f32 m21, m22, m23, m24;
				f32 m31, m32, m33, m34;
				f32 m41, m42, m43, m44;
			};
			f32 m[4][4];
			f32 v[16];
		};

	public:
        
        //! Constructor.
        /*!
            \param setIdentity Setting this to true will have the same affect as calling Identity() after this.
        */
		MashMatrix4(bool setIdentity = true);

        //! Creates a matrix from a quaternion.
        /*!
            \param orientation Quaternion to build this matrix from.
        */
		MashMatrix4(const MashQuaternion &orientation);
        
        //! Creates a transform matrix from 3 components
        /*!
            \param orientation Rotation.
            \param translation Translation.
            \param scale Scale.
        */
        MashMatrix4(const MashQuaternion &orientation, const MashVector3 &translation, const MashVector3 & scale);
        
        //! Multiplies two matrices.
		MashMatrix4 operator*(const MashMatrix4 &other)const;
        
        //! Multiplies two matrices.
		MashMatrix4& operator*=(MashMatrix4 &matrix);

		//! Set this matrix to the identity matrix.
		MashMatrix4& Identity();
        
		//! Inverts this matrix.
		MashMatrix4& Invert();
        
        //! Transposes this matrix.
		MashMatrix4& Transpose();

        //! Determines if this matrix is equal to another within an epsilon value.
        /*!
            For a quick method see FastEquals.
         
            \param other Matrix to test against this.
            \param ep Epsilon value.
            \return True if all elements are equal, false otherwise.
        */
		bool Equals(const mash::MashMatrix4 &other, f32 ep = 0.00001f)const;
        
        //! Determines if this matrix is equal to another.
        /*!
            For a method using an epsilon value see Equals().
         
            \param other Matrix to test against this.
            \return True if all elements are equal, false otherwise.
         */
		bool FastEquals(const mash::MashMatrix4 &other)const;

        //! Creates a camera lookat matrix.
        /*!
            \param position Camera position.
            \param target Camera lookat position.
            \param up Up vector.
            \return This matrix.
        */
		MashMatrix4& CreateCameraLookAt(const MashVector3 &position, const MashVector3 &target, const MashVector3 &up);
        
        //! Creates a camera perspective matrix.
        /*!
            \param FOV Feild of view.
            \param aspectRatio Aspect ratio.
            \param zNear Near z value.
            \param zFar Far z value.
            \return This matrix.
        */
		MashMatrix4& CreatePerspectiveFOV(f32 FOV, f32 aspectRatio, f32 zNear, f32 zFar);
        
        //! Creates a perspective matrix
        /*!
            \param width Screen width.
            \param height Screen height.
            \param zNear Near z value.
            \param zFar Far z value.
            \return This matrix.
        */
		MashMatrix4& CreatePerspective(f32 width, f32 height, f32 zNear, f32 zFar);
        
        //! Creates an orthographic matrix.
        /*!
            \param width Screen width.
            \param height Screen height.
            \param zNear Near z value.
            \param zFar Far z value.
            \return This matrix.
         */
		MashMatrix4& CreateOrthographic(f32 width, f32 height, f32 zNear, f32 zFar);
        
        //! Creates an off center orthographic matrix.
        /*!
            \param minX Screen min X value.
            \param minY Screen min Y value.
            \param maxX Screen max X value.
            \param maxY Screen max Y value.
            \param zn Near z value.
            \param zf Far z value.
            \return This matrix.
         */
        MashMatrix4& CreateOrthographicOffCenter(f32 minX, f32 minY, f32 maxX, f32 maxY, f32 zn, f32 zf);

        //! Sets only the camera forward portion on the matrix.
        /*!
            Elements m13, m23, m33.
            
            \param forward Forward vector to set.
        */
		void SetForward(const mash::MashVector3 &forward);
        
        //! Sets only the camera up portion of the matrix.
        /*!
            Elements m12, m22, m32.
         
            \param up Up vector.
        */
		void SetUp(const mash::MashVector3 &up);
        
        //! Sets only the camera right portion of the matrix.
        /*!
            Elements m11, m21, m31.
         
            \param right Right vector.
        */
		void SetRight(const mash::MashVector3 &right);

        //! Gets the camera forward portion of the matrix.
        /*!
            \return Elements m13, m23, m33.
        */
		const mash::MashVector3 GetForward()const;
        
        //! Gets the camera up portion of the matrix.
        /*!
            \return Elements m12, m22, m32.
        */
		const mash::MashVector3 GetUp()const;
        
        //! Gets the camera right portion of the matrix.
        /*!
            \return Elements m11, m21, m31.
        */
		const mash::MashVector3 GetRight()const;

		//! Converts the rotation portion of this matrix into a quaternion.
        /*!
            \param out This matrix as a quaternion.
        */
		void ToQuaternion(MashQuaternion &out)const;
        
        //! Converts the rotation portion of this matrix into a quaternion.
        /*!
            \return This matrix as a quaternion.
        */
		MashQuaternion ToQuaternion()const;

		//! Sets this to the identity then sets the scale portion.
		void CreateScale(const MashVector3 &scale);
        
		//! Sets this to the identity then sets the translation portion.
		void CreateTranslation(const MashVector3 &translation);


		//! Transforms a vector by rotation only.
		/*!
			\param vector Vector to transform.
			\param out Vector that has been transformed.
		*/
		void TransformRotation(const MashVector3 &vector, MashVector3 &out)const;
        
        //! Transforms a vector by rotation only.
        /*!
            \param vector Vector to transform.
            \return Transformed vector.
        */
		MashVector3 TransformRotation(const MashVector3 &vector)const;
        
        //! Transforms a vector by rotation only.
        /*!
            \param vector Vector to transform.
            \return Transformed vector.
         */
		MashVector2 TransformRotation(const MashVector2 &vector)const;
		
		//! Transforms a vector by rotation only using the transpose of this matrix.
		/*!
			\param vector Vector to transform.
			\param out Vector that has been transformed.
		*/
		void TransformTransposeRotation(const MashVector3 &vector, MashVector3 &out)const;

		//! Sets the translation portion of the matrix only.
		/*!
			\param translation Translation vector.
		*/
		void SetTranslation(const MashVector3 &translation);

		//! Sets the scale portion of the matrix only.
		/*!
			\param scale Scale vector.
		*/
		void SetScale(const MashVector3 &scale);

		//! Sets the roation portion only from a quaternion.
		/*!
			\param rotation Quaternion to get rotation from.
		*/
		void SetRotation(const MashQuaternion &rotation);

		//! Sets the rotation portion only from axis and angle.
		/*!
			\param axis axis to create rotation around.
			\param angle angle in which to rotate.
		*/
		void SetRotationFromAxisAngle(const MashVector3 &axis, f32 angle);

		//! Gets the translation portion of the matrix.
		MashVector3 GetTranslation()const;

        //! Gets the scale portion of this matrix.
		MashVector3 GetScale()const;

		//! Transforms a vector.
		/*!
			\param vector Vector to transform.
			\return Transformed vector.
		*/
		MashVector3 TransformVector(const MashVector3 &vector)const;
        
        //! Transforms a vector.
		/*!
            \param vector Vector to transform.
            \return Transformed vector.
         */
		MashVector2 TransformVector(const MashVector2 &vector)const;
        
        //! Transforms a vector.
		/*!
            \param vector Vector to transform.
            \return Transformed vector.
         */
        MashVector4 TransformVector(const MashVector4 &vector)const;
        
        //! Transforms a vector.
		/*!
            \param vector Vector to transform.
            \param out Transformed vector.
         */
		void TransformVector(const MashVector3 &vector, MashVector3 &out)const;

		//! Transforms a vector by the transpose of this matrix.
		/*!
			\param vector Vector to transform.
			\return Transformed vector.
		*/
		MashVector3 TransformVectorTranspose(const MashVector3 &vector)const;

        //! Decomposes this matrix into scale, rotation, and translation.
        /*!
            \param scale Scale portion out.
            \param rotation Rotation portion out.
            \param translation Translation portion out.
            \return Ok on success, failed if any errors.
        */
		eMASH_STATUS Decompose(MashVector3 &scale, MashQuaternion &rotation, MashVector3 &translation)const;
	};
}

#endif