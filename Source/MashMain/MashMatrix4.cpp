//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMatrix4.h"
#include "MashVector2.h"
#include "MashVector3.h"
#include "MashVector4.h"
#include "MashQuaternion.h"
#include "MashMathHelper.h"
namespace mash
{
	MashMatrix4 MashMatrix4::operator*(const MashMatrix4 &other)const
	{
		MashMatrix4 newMatrix(false);

		//calculate the 3x3 rotation portion
		newMatrix.m11 = m11*other.m11 + m12*other.m21 + m13*other.m31 + m14*other.m41;
		newMatrix.m12 = m11*other.m12 + m12*other.m22 + m13*other.m32 + m14*other.m42;
		newMatrix.m13 = m11*other.m13 + m12*other.m23 + m13*other.m33 + m14*other.m43;
		newMatrix.m14 = m11*other.m14 + m12*other.m24 + m13*other.m34 + m14*other.m44;

		newMatrix.m21 = m21*other.m11 + m22*other.m21 + m23*other.m31 + m24*other.m41;
		newMatrix.m22 = m21*other.m12 + m22*other.m22 + m23*other.m32 + m24*other.m42;
		newMatrix.m23 = m21*other.m13 + m22*other.m23 + m23*other.m33 + m24*other.m43;
		newMatrix.m24 = m21*other.m14 + m22*other.m24 + m23*other.m34 + m24*other.m44;

		newMatrix.m31 = m31*other.m11 + m32*other.m21 + m33*other.m31 + m34*other.m41;
		newMatrix.m32 = m31*other.m12 + m32*other.m22 + m33*other.m32 + m34*other.m42;
		newMatrix.m33 = m31*other.m13 + m32*other.m23 + m33*other.m33 + m34*other.m43;
		newMatrix.m34 = m31*other.m14 + m32*other.m24 + m33*other.m34 + m34*other.m44;

		//calculate the translation portion
		newMatrix.m41 = m41*other.m11 + m42*other.m21 + m43*other.m31 + m44*other.m41;
		newMatrix.m42 = m41*other.m12 + m42*other.m22 + m43*other.m32 + m44*other.m42;
		newMatrix.m43 = m41*other.m13 + m42*other.m23 + m43*other.m33 + m44*other.m43;
		newMatrix.m44 = m41*other.m14 + m42*other.m24 + m43*other.m34 + m44*other.m44;

		return newMatrix;
	}

	MashVector4 MashMatrix4::TransformVector(const MashVector4 &vector)const
	{
		MashVector4 out;
		out.x = (m[0][0]*vector.v[0])+(m[1][0]*vector.v[1])+(m[2][0]*vector.v[2])+(m[3][0]*vector.v[3]);
		out.y = (m[0][1]*vector.v[0])+(m[1][1]*vector.v[1])+(m[2][1]*vector.v[2])+(m[3][1]*vector.v[3]);
		out.z = (m[0][2]*vector.v[0])+(m[1][2]*vector.v[1])+(m[2][2]*vector.v[2])+(m[3][2]*vector.v[3]);
		out.w = (m[0][3]*vector.v[0])+(m[1][3]*vector.v[1])+(m[2][3]*vector.v[2])+(m[3][3]*vector.v[3]);
		return out;
	}

	void MashMatrix4::TransformVector(const MashVector3 &vector, MashVector3 &out)const
	{
		MashVector3 vec(vector);
		out.x = vec.x*m11 + vec.y*m21 + vec.z*m31 + m41;
		out.y = vec.x*m12 + vec.y*m22 + vec.z*m32 + m42;
		out.z = vec.x*m13 + vec.y*m23 + vec.z*m33 + m43;
	}

	MashVector2 MashMatrix4::TransformVector(const MashVector2 &vector)const
	{
		return MashVector2(vector.x*m11 + vector.y*m21 + m41,
							vector.x*m12 + vector.y*m22 + m42);
	}

	MashVector3 MashMatrix4::TransformVector(const MashVector3 &vector)const
	{
		return MashVector3(vector.x*m11 + vector.y*m21 + vector.z*m31 + m41,
							vector.x*m12 + vector.y*m22 + vector.z*m32 + m42,
							vector.x*m13 + vector.y*m23 + vector.z*m33 + m43);
	}

	MashVector3 MashMatrix4::TransformVectorTranspose(const MashVector3 &vector)const
	{
		return MashVector3(vector.x*m11 + vector.y*m12 + vector.z*m13 - m41,
							vector.x*m21 + vector.y*m22 + vector.z*m23 - m42,
							vector.x*m31 + vector.y*m32 + vector.z*m33 - m43);
	}

	MashMatrix4::MashMatrix4(bool setIdentity)
	{
        if (setIdentity)
            Identity();
	}
    
    MashMatrix4::MashMatrix4(const MashQuaternion &orientation, const MashVector3 &translation, const MashVector3 & scale)
    {
        SetRotation(orientation);
        
        m11 *= scale.x;
        m21 *= scale.x;
        m31 *= scale.x;
        
        m12 *= scale.y;
        m22 *= scale.y;
        m32 *= scale.y;
        
        m13 *= scale.z;
        m23 *= scale.z;
        m33 *= scale.z;
        
		m41 = translation.x;
        m42 = translation.y;
        m43 = translation.z;
        m44 = 1.0f;
        
        m14 = 0.0f;
        m24 = 0.0f;
        m34 = 0.0f;
    }

	MashMatrix4& MashMatrix4::operator*=(MashMatrix4 &matrix)
	{
		(*this) = (*this)*matrix;
		return (*this);
	}

	MashMatrix4& MashMatrix4::Identity()
	{
		m11 = 1.f; m12 = 0.f; m13 = 0.f; m14 = 0.f;
		m21 = 0.f; m22 = 1.f; m23 = 0.f; m24 = 0.f;
		m31 = 0.f; m32 = 0.f; m33 = 1.f; m34 = 0.f;
		m41 = 0.f; m42 = 0.f, m43 = 0.f; m44 = 1.f;

		return *this;
	}

	MashMatrix4::MashMatrix4(const MashQuaternion &orientation)
	{
		m11 = 1.0f - 2.0f*orientation.y*orientation.y - 2.0f*orientation.z*orientation.z;
		m12 = 2.0f*orientation.x*orientation.y + 2.0f*orientation.z*orientation.w;
		m13 = 2.0f*orientation.x*orientation.z - 2.0f*orientation.y*orientation.w;
		m14 = 0.0f;

		m21 = 2.0f*orientation.x*orientation.y - 2.0f*orientation.z*orientation.w;
		m22 = 1.0f - 2.0f*orientation.x*orientation.x - 2.0f*orientation.z*orientation.z;
		m23 = 2.0f*orientation.z*orientation.y + 2.0f*orientation.x*orientation.w;
		m24 = 0.0f;

		m31 = 2.0f*orientation.x*orientation.z + 2.0f*orientation.y*orientation.w;
		m32 = 2.0f*orientation.z*orientation.y - 2.0f*orientation.x*orientation.w;
		m33 = 1.0f - 2.0f*orientation.x*orientation.x - 2.0f*orientation.y*orientation.y;
		m34 = 0.0f;

		m41 = 0.f;
		m42 = 0.f;
		m43 = 0.f;
		m44 = 1.f;
	}

	MashMatrix4& MashMatrix4::Transpose()
	{
		MashMatrix4 temp(*this);
		m11 = temp.m11;
		m12 = temp.m21;
		m13 = temp.m31;
		m14 = temp.m41;
		m21 = temp.m12;
		m22 = temp.m22;
		m23 = temp.m32;
		m24 = temp.m42;
		m31 = temp.m13;
		m32 = temp.m23;
		m33 = temp.m33;
		m34 = temp.m43;
		m41 = temp.m14;
		m42 = temp.m24;
		m43 = temp.m34;
		m44 = temp.m44;

		return (*this);
	}

	bool MashMatrix4::FastEquals(const mash::MashMatrix4 &other)const
	{
		return ((m11 == other.m11) &&
			(m12 == other.m12) &&
			(m13 == other.m13) &&
			(m14 == other.m14) &&
			(m21 == other.m21) &&
			(m22 == other.m22) &&
			(m23 == other.m23) &&
			(m24 == other.m24) &&
			(m31 == other.m31) &&
			(m32 == other.m32) &&
			(m33 == other.m33) &&
			(m34 == other.m34) &&
			(m41 == other.m41) &&
			(m42 == other.m42) &&
			(m43 == other.m43) &&
			(m44 == other.m44));
	}

	bool MashMatrix4::Equals(const mash::MashMatrix4 &other, f32 ep)const
	{
		if (math::FloatEqualTo(m11, other.m11, ep) &&
			math::FloatEqualTo(m12, other.m12, ep) &&
			math::FloatEqualTo(m13, other.m13, ep) &&
			math::FloatEqualTo(m14, other.m14, ep) &&
			math::FloatEqualTo(m21, other.m21, ep) &&
			math::FloatEqualTo(m22, other.m22, ep) &&
			math::FloatEqualTo(m23, other.m23, ep) &&
			math::FloatEqualTo(m24, other.m24, ep) &&
			math::FloatEqualTo(m31, other.m31, ep) &&
			math::FloatEqualTo(m32, other.m32, ep) &&
			math::FloatEqualTo(m33, other.m33, ep) &&
			math::FloatEqualTo(m34, other.m34, ep) &&
			math::FloatEqualTo(m41, other.m41, ep) &&
			math::FloatEqualTo(m42, other.m42, ep) &&
			math::FloatEqualTo(m43, other.m43, ep) &&
			math::FloatEqualTo(m44, other.m44, ep))
		{
			return true;
		}

		return false;
	}

	MashMatrix4& MashMatrix4::Invert()
	{
	    f32 fDeterminant = (m11 * m22 - m12 * m21) * (m33 * m44 - m34 * m43) -
							(m11 * m23 - m13 * m21) * (m32 * m44 - m34 * m42) +
							(m11 * m24 - m14 * m21) * (m32 * m43 - m33 * m42) +
							(m12 * m23 - m13 * m22) * (m31 * m44 - m34 * m41) -
							(m12 * m24 - m14 * m22) * (m31 * m43 - m33 * m41) +
							(m13 * m24 - m14 * m23) * (m31 * m42 - m32 * m41);

		//make sure the determinant is not zero
		if (fabs(fDeterminant) <= 0.00000001f)
			return *this;

		fDeterminant = 1.0f / fDeterminant;

		MashMatrix4 newMatrix(false);

		newMatrix.m11 = fDeterminant * (m22 * (m33 * m44 - m34 * m43) +
				m23 * (m34 * m42 - m32 * m44) +
				m24 * (m32 * m43 - m33 * m42));
		newMatrix.m12 = fDeterminant * (m32 * (m13 * m44 - m14 * m43) +
				m33 * (m14 * m42 - m12 * m44) +
				m34 * (m12 * m43 - m13 * m42));
		newMatrix.m13 = fDeterminant * (m42 * (m13 * m24 - m14 * m23) +
				m43 * (m14 * m22 - m12 * m24) +
				m44 * (m12 * m23 - m13 * m22));
		newMatrix.m14 = fDeterminant * (m12 * (m24 * m33 - m23 * m34) +
				m13 * (m22 * m34 - m24 * m32) +
				m14 * (m23 * m32 - m22 * m33));
		newMatrix.m21 = fDeterminant * (m23 * (m31 * m44 - m34 * m41) +
				m24 * (m33 * m41 - m31 * m43) +
				m21 * (m34 * m43 - m33 * m44));

		//////////////////////
		newMatrix.m22 = fDeterminant * (m33 * (m11 * m44 - m14 * m41) +
				m34 * (m13 * m41 - m11 * m43) +
				m31 * (m14 * m43 - m13 * m44));
		newMatrix.m23 = fDeterminant * (m43 * (m11 * m24 - m14 * m21) +
				m44 * (m13 * m21 - m11 * m23) +
				m41 * (m14 * m23 - m13 * m24));
		newMatrix.m24 = fDeterminant * (m13 * (m24 * m31 - m21 * m34) +
				m14 * (m21 * m33 - m23 * m31) +
				m11 * (m23 * m34 - m24 * m33));
		newMatrix.m31 = fDeterminant * (m24 * (m31 * m42 - m32 * m41) +
				m21 * (m32 * m44 - m34 * m42) +
				m22 * (m34 * m41 - m31 * m44));
		newMatrix.m32 = fDeterminant * (m34 * (m11 * m42 - m12 * m41) +
				m31 * (m12 * m44 - m14 * m42) +
				m32 * (m14 * m41 - m11 * m44));
		newMatrix.m33 = fDeterminant * (m44 * (m11 * m22 - m12 * m21) +
				m41 * (m12 * m24 - m14 * m22) +
				m42 * (m14 * m21 - m11 * m24));
		newMatrix.m34 = fDeterminant * (m14 * (m22 * m31 - m21 * m32) +
				m11 * (m24 * m32 - m22 * m34) +
				m12 * (m21 * m34 - m24 * m31));
		newMatrix.m41 = fDeterminant * (m21 * (m33 * m42 - m32 * m43) +
				m22 * (m31 * m43 - m33 * m41) +
				m23 * (m32 * m41 - m31 * m42));
		newMatrix.m42 = fDeterminant * (m31 * (m13 * m42 - m12 * m43) +
				m32 * (m11 * m43 - m13 * m41) +
				m33 * (m12 * m41 - m11 * m42));
		newMatrix.m43 = fDeterminant * (m41 * (m13 * m22 - m12 * m23) +
				m42 * (m11 * m23 - m13 * m21) +
				m43 * (m12 * m21 - m11 * m22));
		newMatrix.m44 = fDeterminant * (m11 * (m22 * m33 - m23 * m32) +
				m12 * (m23 * m31 - m21 * m33) +
				m13 * (m21 * m32 - m22 * m31));

		*this = newMatrix;
		return *this;
	}

	MashQuaternion MashMatrix4::ToQuaternion()const
	{
		MashQuaternion q;
		ToQuaternion(q);
		return q;
	}

	//converts this rotation matrix to a Quaternion
	void MashMatrix4::ToQuaternion(MashQuaternion &out)const
	{
		f32 w1 = m11+m22+m33;
		f32 x1 = m11-m22-m33;
		f32 y1 = m22-m11-m33;
		f32 z1 = m33-m11-m22;

		int32 iBiggestIndex = 0;
		f32 i = w1;
		if (x1 > i)
		{
			i = x1;
			iBiggestIndex = 1;
		}
		if (y1 > i)
		{
			i = y1;
			iBiggestIndex = 2;
		}
		if (z1 > i)
		{
			i = z1;
			iBiggestIndex = 3;
		}

		f32 fBiggestVal = sqrt(i+1.0f)*0.5f;
		f32 fMult = 0.25f/fBiggestVal;

		switch(iBiggestIndex)
		{
		case 0:
			{
				out.w = fBiggestVal;
				out.x = (m23-m32)*fMult;
				out.y = (m31-m13)*fMult;
				out.z = (m12-m21)*fMult;
				break;
			}
		case 1:
			{
				out.x = fBiggestVal;
				out.w = (m23-m32)*fMult;
				out.y = (m12+m21)*fMult;
				out.z = (m31+m13)*fMult;
				break;
			}
		case 2:
			{
				out.y = fBiggestVal;
				out.w = (m31-m13)*fMult;
				out.x = (m12+m21)*fMult;
				out.z = (m23+m32)*fMult;
				break;
			}
		case 3:
			{
				out.z = fBiggestVal;
				out.w = (m12-m21)*fMult;
				out.x = (m31+m13)*fMult;
				out.y = (m23+m32)*fMult;
				break;
			}
		}

		out.Normalize();
	}

	//transforms the vector by rotation only
	//not the direction
	void MashMatrix4::TransformRotation(const MashVector3 &vector, MashVector3 &out)const
	{
		MashVector3 vec(vector);
		out.x = m11*vec.x + m21*vec.y + m31*vec.z;
		out.y = m12*vec.x + m22*vec.y + m32*vec.z;
		out.z = m13*vec.x + m23*vec.y + m33*vec.z;
	}

	MashVector3 MashMatrix4::TransformRotation(const MashVector3 &vector)const
	{
		return MashVector3(m11*vector.x + m21*vector.y + m31*vector.z,
			m12*vector.x + m22*vector.y + m32*vector.z,
			m13*vector.x + m23*vector.y + m33*vector.z);
	}

	MashVector2 MashMatrix4::TransformRotation(const MashVector2 &vector)const
	{
		return MashVector2(m11*vector.x + m21*vector.y,
			m12*vector.x + m22*vector.y);
	}

	//transforms the vector by inverse rotation only
	//not the direction
	void MashMatrix4::TransformTransposeRotation(const MashVector3 &vector, MashVector3 &out)const
	{
		MashVector3 vec(vector);
		out.x = m11*vec.x + m12*vec.y + m13*vec.z;
		out.y = m21*vec.x + m22*vec.y + m23*vec.z;
		out.z = m31*vec.x + m32*vec.y + m33*vec.z;
	}

	//set the scale of the matrix
	void MashMatrix4::CreateScale(const MashVector3 &scale)
	{
		Identity();

		m11 = scale.x;
		m22 = scale.y;
		m33 = scale.z;
	}

	void MashMatrix4::CreateTranslation(const MashVector3 &translation)
	{
		Identity();

		m41 = translation.x;
		m42 = translation.y;
		m43 = translation.z;
	}

	//sets the translation portion of the matrix
	void MashMatrix4::SetTranslation(const MashVector3 &translation)
	{
		m41 = translation.x;
		m42 = translation.y;
		m43 = translation.z;
		m44 = 1.0f;
	}

	void MashMatrix4::SetScale(const MashVector3 &scale)
	{
		m11 = scale.x;
		m22 = scale.y;
		m33 = scale.z;
	}

	void MashMatrix4::SetRotation(const MashQuaternion &rotation)
	{
		m11 = 1.0f - 2.0f*rotation.y*rotation.y - 2.0f*rotation.z*rotation.z;
		m12 = 2.0f*rotation.x*rotation.y + 2.0f*rotation.z*rotation.w;
		m13 = 2.0f*rotation.x*rotation.z - 2.0f*rotation.y*rotation.w;
		m14 = 0.0f;

		m21 = 2.0f*rotation.x*rotation.y - 2.0f*rotation.z*rotation.w;
		m22 = 1.0f - 2.0f*rotation.x*rotation.x - 2.0f*rotation.z*rotation.z;
		m23 = 2.0f*rotation.z*rotation.y + 2.0f*rotation.x*rotation.w;
		m24 = 0.0f;

		m31 = 2.0f*rotation.x*rotation.z + 2.0f*rotation.y*rotation.w;
		m32 = 2.0f*rotation.z*rotation.y - 2.0f*rotation.x*rotation.w;
		m33 = 1.0f - 2.0f*rotation.x*rotation.x - 2.0f*rotation.y*rotation.y;
		m34 = 0.0f;
	}

	void MashMatrix4::SetRotationFromAxisAngle(const MashVector3 &axis, f32 angle)
	{
		f32 cosTheta = cosf(angle);
		f32 sinTheta = sinf(angle);

		f32 sinThetaX = axis.x*sinTheta;
		f32 sinThetaY = axis.y*sinTheta;
		f32 sinThetaZ = axis.z*sinTheta;

		f32 xy = axis.x*axis.y;
		f32 xz = axis.x*axis.z;
		f32 yz = axis.y*axis.z;

		m11 = (axis.x*axis.x)*(1-cosTheta)+cosTheta;
		m12 = xy*(1-cosTheta)+sinThetaZ;
		m13 = xz*(1-cosTheta)-sinThetaY;
		m21 = xy*(1-cosTheta)-sinThetaZ;
		m22 = (axis.y*axis.y)*(1-cosTheta)+cosTheta;
		m23 = yz*(1-cosTheta)+sinThetaX;
		m31 = xz*(1-cosTheta)+sinThetaY;
		m32 = yz*(1-cosTheta)-sinThetaX;
		m33 = (axis.z*axis.z)*(1-cosTheta)+cosTheta;
	}

	MashMatrix4& MashMatrix4::CreateCameraLookAt(const MashVector3 &position, const MashVector3 &target, const MashVector3 &up)
	{
		MashVector3 zaxis =  target - position;
		zaxis.Normalize();

		MashVector3 xaxis = up.Cross(zaxis);
		xaxis.Normalize();

		MashVector3 yaxis = zaxis.Cross(xaxis);
		//yaxis.Normalize();

		m11 = xaxis.x;
		m12 = yaxis.x;
		m13 = zaxis.x;
		m14 = 0;

		m21 = xaxis.y;
		m22 = yaxis.y;
		m23 = zaxis.y;
		m24 = 0;

		m31 = xaxis.z;
		m32 = yaxis.z;
		m33 = zaxis.z;
		m34 = 0;

		m41 = -xaxis.Dot(position);
		m42 = -yaxis.Dot(position);
		m43 = -zaxis.Dot(position);
		m44 = 1;
		return *this;
	}

	MashMatrix4& MashMatrix4::CreatePerspectiveFOV(f32 FOV, f32 aspectRatio, f32 zNear, f32 zFar)
	{
		const f32 h = 1.0f/tan(FOV*0.5f);
		const f32 w = h / aspectRatio;

		m11 = w;
		m12 = 0;
		m13 = 0;
		m14 = 0;

		m21 = 0;
		m22 = h;
		m23 = 0;
		m24 = 0;

		m31 = 0;
		m32 = 0;
		m33 = zFar/(zFar-zNear);
		m34 = 1;

		m41 = 0;
		m42 = 0;
		m43 = (-zNear*zFar)/(zFar-zNear);
		m44 = 0;
		return *this;
	}

	MashMatrix4& MashMatrix4::CreatePerspective(f32 width, f32 height, f32 zNear, f32 zFar)
	{
		m11 = 2*zNear/width;
		m12 = 0;
		m13 = 0;
		m14 = 0;

		m21 = 0;
		m22 = 2*zNear/height;
		m23 = 0;
		m24 = 0;

		m31 = 0;
		m32 = 0;
		m33 = zFar/(zFar-zNear);
		m34 = 1;

		m41 = 0;
		m41 = 0;
		m42 = zNear*zFar/(zNear-zFar);
		m43 = 0;
		return *this;
	}

	MashMatrix4& MashMatrix4::CreateOrthographic(f32 width, f32 height, f32 zNear, f32 zFar)
	{
		m11 = 2/width;
		m12 = 0;
		m13 = 0;
		m14 = 0;

		m21 = 0;
		m22 = 2/height;
		m23 = 0;
		m24 = 0;

		m31 = 0;
		m32 = 0;
		m33 = 1/(zFar-zNear);
		m34 = 0;

		m41 = 0;
		m42 = 0;
		m43 = zNear/(zNear-zFar);
		m44 = 1;
		return *this;
	}
    
    MashMatrix4& MashMatrix4::CreateOrthographicOffCenter(f32 minX, f32 minY, f32 maxX, f32 maxY, f32 zNear, f32 zFar)
    {
        m11 = 2/(maxX - minX);
		m12 = 0;
		m13 = 0;
		m14 = 0;
        
		m21 = 0;
		m22 = 2/(maxY - minY);
		m23 = 0;
		m24 = 0;
        
		m31 = 0;
		m32 = 0;
		m33 = 1/(zFar-zNear);
		m34 = 0;
        
		m41 = (minX + maxX) / (minX - maxX);
		m42 = (maxY + minY) / (minY - maxY);
		m43 = zNear / (zNear - zFar);
		m44 = 1;
		return *this;
    }

	MashVector3 MashMatrix4::GetTranslation()const
	{
		return MashVector3(m41,m42,m43);
	}

	MashVector3 MashMatrix4::GetScale()const
	{
		return MashVector3(m11,m22,m33);
	}

	void MashMatrix4::SetForward(const mash::MashVector3 &vForward)
	{
		m13 = vForward.x;
		m23 = vForward.y;
		m33 = vForward.z;
	}

	void MashMatrix4::SetUp(const mash::MashVector3 &vUp)
	{
		m12 = vUp.x;
		m22 = vUp.y;
		m32 = vUp.z;
	}

	void MashMatrix4::SetRight(const mash::MashVector3 &vRight)
	{
		m11 = vRight.x;
		m21 = vRight.y;
		m31 = vRight.z;
	}

	const mash::MashVector3 MashMatrix4::GetForward()const
	{
		return mash::MashVector3(m13, m23, m33);
	}

	const mash::MashVector3 MashMatrix4::GetUp()const
	{
		return mash::MashVector3(m12, m22, m32);
	}

	const mash::MashVector3 MashMatrix4::GetRight()const
	{
		return mash::MashVector3(m11, m21, m31);
	}

	eMASH_STATUS MashMatrix4::Decompose(MashVector3 &scale, MashQuaternion &rotation, MashVector3 &translation)const
	{
		MashMatrix4 n;

		scale.x = sqrt((m[0][0] * m[0][0]) + (m[0][1] * m[0][1]) + (m[0][2] * m[0][2]));
		scale.y = sqrt((m[1][0] * m[1][0]) + (m[1][1] * m[1][1]) + (m[1][2] * m[1][2]));
		scale.z = sqrt((m[2][0] * m[2][0]) + (m[2][1] * m[2][1]) + (m[2][2] * m[2][2]));
		
		if ((scale.x == 0) || (scale.y == 0) || (scale.z == 0))
			return aMASH_FAILED;

		translation.x = m[3][0];
		translation.y = m[3][1];
		translation.z = m[3][2];

		n.m[0][0] = m[0][0] / scale.x;
		n.m[0][1] = m[0][1] / scale.x;
		n.m[0][2] = m[0][2] / scale.x;
		n.m[1][0] = m[1][0] / scale.y;
		n.m[1][1] = m[1][1] / scale.y;
		n.m[1][2] = m[1][2] / scale.y;
		n.m[2][0] = m[2][0] / scale.z;
		n.m[2][1] = m[2][1] / scale.z;
		n.m[2][2] = m[2][2] / scale.z;

		n.ToQuaternion(rotation);
		return aMASH_OK;
	}
}