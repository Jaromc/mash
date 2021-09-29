//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashQuaternion.h"
#include "MashVector3.h"
#include "MashMatrix4.h"
#include "MashMathHelper.h"

namespace mash
{
	MashQuaternion MashQuaternion::operator+(const MashQuaternion &other)const
	{
		return MashQuaternion(w+other.w,x+other.x,y+other.y,z+other.z);
	}

	MashVector3 MashQuaternion::TransformVector(const MashVector3 &vector)const
	{
        //nVidia
		MashVector3 uv, uuv;
		MashVector3 qvec(x, y, z);
		uv = qvec.Cross(vector);
		uuv = qvec.Cross(uv);
		uv *= (2.0f * w);
		uuv *= 2.0f;

		return vector + uv + uuv;
	}

	bool MashQuaternion::operator== (const MashQuaternion &q)const
	{
		return x==q.x && y==q.y && z==q.z && w==q.w;
	}

	bool MashQuaternion::operator!= (const MashQuaternion &q)const
	{
		return x!=q.x || y!=q.y || z!=q.z || w!=q.w;
	}

	MashQuaternion MashQuaternion::operator*(f32 f)const
	{
		return MashQuaternion(w*f,x*f,y*f,z*f);
	}

	MashQuaternion MashQuaternion::operator-(const MashQuaternion &other)const
	{
		return MashQuaternion(w - other.w, x - other.x, y - other.y, z - other.z);
	}

	MashQuaternion::MashQuaternion():w(1.f),x(0.f),y(0.f),z(0.f){}
	MashQuaternion::MashQuaternion(f32 nw, f32 nx, f32 ny, f32 nz):w(nw),x(nx),y(ny),z(nz){}

	MashQuaternion MashQuaternion::operator*(const MashQuaternion &other)const
	{
		return MashQuaternion(w*other.w - x*other.x - y*other.y - z*other.z,
						w*other.x + x*other.w + y*other.z - z*other.y,
						w*other.y + y*other.w + z*other.x - x*other.z,
						w*other.z + z*other.w + x*other.y - y*other.x);
	}

	void MashQuaternion::Identity()
	{
		w = 1.f;
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}

	void MashQuaternion::SetRotationX(f32 theta)
	{
		f32 thetaOver2 = theta *0.5f;

		w = cosf(thetaOver2);
		x = sinf(thetaOver2);
	}

	void MashQuaternion::SetRotationY(f32 theta)
	{
		f32 thetaOver2 = theta *0.5f;

		w = cosf(thetaOver2);
		y = sinf(thetaOver2);
	}

	void MashQuaternion::SetRotationZ(f32 theta)
	{
		f32 thetaOver2 = theta *0.5f;

		w = cosf(thetaOver2);
		//x = 0.f;
		//y = 0.f;
		z = sinf(thetaOver2);
	}

	void MashQuaternion::SetRotationAxis(const MashVector3 &axis, f32 theta)
	{
		f32 thetaOver2 = theta * 0.5f;
		f32 sinThetaOver2 = sinf(thetaOver2);

		w = cosf(thetaOver2);
		x = axis.x * sinThetaOver2;
		y = axis.y * sinThetaOver2;
		z = axis.z * sinThetaOver2;
	}

	void MashQuaternion::SetEuler(f32 _x, f32 _y, f32 _z)
	{
		f32 angle = _x * 0.5f;
		f32 sr = sin(angle);
		f32 cr = cos(angle);

		angle = _y * 0.5f;
		f32 sp = sin(angle);
		f32 cp = cos(angle);

		angle = _z * 0.5f;
		f32 sy = sin(angle);
		f32 cy = cos(angle);

		f32 cpcy = cp * cy;
		f32 spcy = sp * cy;
		f32 cpsy = cp * sy;
		f32 spsy = sp * sy;

		x = (sr * cpcy) - (cr * spsy);
		y = (cr * spcy) + (sr * cpsy);
		z = (cr * cpsy) - (sr * spcy);
		w = (cr * cpcy) + (sr * spsy);

		Normalize();
	}

	void MashQuaternion::SetEuler(const mash::MashVector3 &angles)
	{
		SetEuler(angles.x, angles.y, angles.z);
	}

	void MashQuaternion::AddScaledVector(const MashVector3 &vVector, f32 fScale)
	{
		MashQuaternion q(0.f, vVector.x * fScale, vVector.y * fScale, vVector.z * fScale);
		q *= *this;

		w += q.w * (0.5f);
		x += q.x * (0.5f);
		y += q.y * (0.5f);
		z += q.z * (0.5f);
	}

	void MashQuaternion::Normalize()
	{
		//calculate the length
		f32 fMag = w*w + x*x + y*y + z*z;

		//if the length is valid then normalize it
		if (fMag > 0.f)
		{
			f32 fOneOverMag = 1.f/sqrtf(fMag);
			w *= fOneOverMag;
			x *= fOneOverMag;
			y *= fOneOverMag;
			z *= fOneOverMag;
		}
		//else bad length so just set it to the identity Quaternion
		else
		{
			Identity();
		}
	}

	f32 MashQuaternion::Magnitude()
	{
		return sqrtf(w*w+x*x+y*y+z*z);
	}

	f32 MashQuaternion::GetRotationAngle()const
	{
		//w = cos(theta / 2) so multiply it back
		return 2*acos(w);
	}

	MashVector3 MashQuaternion::GetRotationAxis()const
	{
		f32 fSinThetaOver2Sq = 1.f - w*w;

		//if bad number then return
		if (fSinThetaOver2Sq <= 0.f)
		{
			return MashVector3(1.f, 0.f,0.f);
		}

		f32 fOneOverSinThetaOver2 = 1.f / sqrt(fSinThetaOver2Sq);

		//return axis 
		return MashVector3(x * fOneOverSinThetaOver2,
							y * fOneOverSinThetaOver2,
							z * fOneOverSinThetaOver2);
	}

	//returns pitch, yaw, roll
	void MashQuaternion::ToEulerAngles(MashVector3 &vReturnEuler)const
	{
		const f32 sqw = w*w;
		const f32 sqx = x*x;
		const f32 sqy = y*y;
		const f32 sqz = z*z;
		const f32 test = 2.0f * (y*w - x*z);

		if (math::FloatEqualTo(test,1.0f, 0.000001f))
		{
			vReturnEuler.z = -2.0f * atan2(x,w);
			vReturnEuler.x = 0.0f;
			vReturnEuler.y = mash::math::Pi() / 2.0f;
		}
		else if (math::FloatEqualTo(test,-1.0f, 0.000001f))
		{
			vReturnEuler.z = 2.0f * atan2(x,w);
			vReturnEuler.x = 0.0f;
			vReturnEuler.y = mash::math::Pi() / -2.0f;
		}
		else
		{
			vReturnEuler.z = (atan2(2.0f * (x*y +z*w),(sqx - sqy - sqz + sqw)));
			vReturnEuler.x = (atan2(2.0f * (y*z +x*w),(-sqx - sqy + sqz + sqw)));
			vReturnEuler.y = asinf( math::Clamp(-1.0f, 1.0f, test) );
		}
	}

	f32 MashQuaternion::DotProduct(const MashQuaternion &quat)const
	{
		return w*quat.w + x*quat.x + y*quat.y + z*quat.z;
	}

	MashQuaternion operator*(f32 a, const MashQuaternion &b)
	{
		return MashQuaternion(b.w * a, b.x * a, b.y * a, b.z * a);
	}

	//interploates from a to b as t varies from 0 to 1. Set this to the result
	MashQuaternion& MashQuaternion::Slerp(const MashQuaternion &a, const MashQuaternion &b, f32 t)
	{
		f32 fAngle = a.DotProduct(b);

		MashQuaternion at(a.w, a.x, a.y, a.z);
		MashQuaternion bt(b.w, b.x, b.y, b.z);

		if (fAngle < 0.0f)
		{
			at *= -1.0f;
			fAngle *= -1.0f;
		}

		f32 fScale;
		f32 fInvScale;

		if ((fAngle + 1.0f) > 0.05f)
		{
			if ((1.0f - fAngle) >= 0.05f)
			{
				f32 fTheta = (f32)acos(fAngle);
				f32 fInvTheta = 1.0f / (f32)sin(fTheta);
				fScale = (f32)sin(fTheta * (1.0f - t)) * fInvTheta;
				fInvScale = (f32)sin(fTheta * t) * fInvTheta;
			}
			else
			{
				fScale = 1.0f - t;
				fInvScale = t;
			}
		}
		else
		{
			bt.x = -at.y;
			bt.y = at.x;
			bt.z = -at.w;
			bt.w = at.z;

			fScale = (f32)sin(mash::math::Pi() * (0.5f - t));
			fInvScale = (f32)sin(mash::math::Pi() * t);
		}

		*this = (at * fScale) + (bt * fInvScale);
        return *this;
	}

	MashQuaternion& MashQuaternion::Lerp(const MashQuaternion &a, const MashQuaternion &b, f32 t)
	{
        const f32 scale = 1.0f - t;
        *this = (a*scale) + (b*t);
        return *this;
	}

	//equivalent to MashQuaternion inverse for unit MashQuaternions
	MashQuaternion& MashQuaternion::Conjugate()
	{
		//same rotation
		w = w;
		//opposite axis
		x = -x;
		y = -y;
		z = -z;

		return *this;
	}

	//raises to the power of 'exponent'
	//used to extract a fraction of an angular displacement
	void MashQuaternion::Exponentiation(const MashQuaternion &q, f32 exponent)
	{
		//check for identical MashQuaternion
		//this will stop divide by zero errors
		if (fabs(q.w)>0.999f)
		{
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
			return;
		}

		//extract theta/2 
		f32 alpha = acos(q.w);
		f32 newAlpha = alpha*exponent;
		w = cos(newAlpha);

		f32 mult = sin(newAlpha)/sin(alpha);
		x = q.x * mult;
		y = q.y * mult;
		z = q.z * mult;
	}

	void MashQuaternion::ToMatrix(MashMatrix4 &mReturnMatrix)const
	{
	    mReturnMatrix.m11 = 1.0f - 2.0f*y*y - 2.0f*z*z;
		mReturnMatrix.m12 = 2.0f*x*y + 2.0f*z*w;
		mReturnMatrix.m13 = 2.0f*x*z - 2.0f*y*w;
		mReturnMatrix.m14 = 0.0f;

		mReturnMatrix.m21 = 2.0f*x*y - 2.0f*z*w;
		mReturnMatrix.m22 = 1.0f - 2.0f*x*x - 2.0f*z*z;
		mReturnMatrix.m23 = 2.0f*z*y + 2.0f*x*w;
		mReturnMatrix.m24 = 0.0f;

		mReturnMatrix.m31 = 2.0f*x*z + 2.0f*y*w;
		mReturnMatrix.m32 = 2.0f*z*y - 2.0f*x*w;
		mReturnMatrix.m33 = 1.0f - 2.0f*x*x - 2.0f*y*y;
		mReturnMatrix.m34 = 0.0f;

		mReturnMatrix.m41= 0.f;
		mReturnMatrix.m42= 0.f;
		mReturnMatrix.m43 = 0.f;
		mReturnMatrix.m44 = 1.f;
	}

	void MashQuaternion::RotateTo(const mash::MashVector3 &from, const mash::MashVector3 &to)
	{
		mash::MashVector3 fromCopy = from;
		mash::MashVector3 toCopy = to;
		fromCopy.Normalize();
		toCopy.Normalize();

		mash::MashVector3 c = fromCopy.Cross(toCopy);

		f32 d = fromCopy.Dot(toCopy);
		if (d >= 1.0f)
		{
			Identity();
			return;
		}
        else if (d <= -1.0f)
        {
            MashVector3 axis(1.0f, 0.0f, 0.0f);
            
            axis = axis.Cross(fromCopy);
            if (axis.Length() == 0.0f)
            {
                axis = MashVector3(0.0f,1.f,0.0f);
                axis = axis.Cross(fromCopy);
            }

            w = 0.0f;
            x = axis.x;
            y = axis.y;
            z = axis.z;
            Normalize();
			return;
        }
		f32 s = sqrtf((1.0f + d) * 2.0f);
		f32 inverses = 1.0f / s;

        MashVector3 c1 = fromCopy.Cross(toCopy) * inverses;
		w = s * 0.5f;
        x = c1.x;
        y = c1.y;
        z = c1.z;
        Normalize();
	}

	MashQuaternion& MashQuaternion::operator*=(const MashQuaternion &q)
	{
		*this = *this * q;
		return *this;
	}

	MashQuaternion& MashQuaternion::operator+=(const MashQuaternion &q)
	{
		w += q.w;
		x += q.x;
		y += q.y;
		z += q.z;
		return *this;
	}

	MashQuaternion& MashQuaternion::operator-=(const MashQuaternion &q)
	{
		w -= q.w;
		x -= q.x;
		y -= q.y;
		z -= q.z;
		return *this;
	}

	MashQuaternion& MashQuaternion::operator*=(f32 value)
	{
		w *= value;
		x *= value;
		y *= value;
		z *= value;
		return *this;
	}

	MashQuaternion& MashQuaternion::operator/=(f32 value)
	{
		w /= value;
		x /= value;
		y /= value;
		z /= value;
		return *this;
	}

	MashQuaternion MashQuaternion::operator~()const
	{
		return MashQuaternion(w,-x,-y,-z);
	}
}