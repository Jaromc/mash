//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashVector3.h"
#include <math.h>

namespace mash
{
	MashVector3 operator*(f32 lhs, const MashVector3 &rhs)
	{
		return rhs * lhs;
	}

	MashVector3::MashVector3(f32 nx, f32 ny, f32 nz):x(nx),y(ny),z(nz){}

	bool MashVector3::IsZero()const
	{
		return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f));
	}

	f32 MashVector3::GetDistanceTo(const MashVector3 &other)const
	{
		return MashVector3(x-other.x, y-other.y, z-other.z).Length();
	}

	f32 MashVector3::GetDistanceToSQ(const MashVector3 &other)const
	{
		return MashVector3(x-other.x, y-other.y, z-other.z).LengthSq();
	}

	MashVector3 MashVector3::operator*(const MashVector3 &other)const
	{
		return MashVector3(x*other.x, y*other.y, z*other.z);
	}
    
	bool MashVector3::operator!=(const MashVector3 &v)const
	{
		return x!=v.x || y!=v.y || z!=v.z;
	}

	bool MashVector3::operator==(const MashVector3 &v)const
	{
		return x==v.x && y==v.y && z==v.z;
	}

	MashVector3 MashVector3::operator*(f32 a)const
	{
		return MashVector3(x*a,y*a,z*a);
	}

	MashVector3& MashVector3::operator*=(f32 a)
	{
		x*=a, y*=a, z*=a;
		return *this;
	}

	MashVector3& MashVector3::operator+=(const MashVector3 &v)
	{
		x+=v.x, y+=v.y, z+=v.z;
		return *this;
	}

	MashVector3 MashVector3::operator+(const MashVector3 &v)const
	{
		return MashVector3(x+v.x,y+v.y,z+v.z);
	}

	MashVector3 MashVector3::operator-(const MashVector3 &v)const
	{
		return MashVector3(x-v.x,y-v.y,z-v.z);
	}

	MashVector3 MashVector3::operator-()const
	{
		return MashVector3(-x,-y,-z);
	}

	MashVector3& MashVector3::operator-=(const MashVector3 &v)
	{
		x-=v.x, y-=v.y, z-=v.z;
		return *this;
	}

	MashVector3 MashVector3::operator/(f32 a)const
	{
		f32 oneOverA = 1.f/a;
		return MashVector3(x*oneOverA,y*oneOverA,z*oneOverA);
	}

	MashVector3& MashVector3::operator/=(f32 a)
	{
		f32 oneOverA = 1.f/a;
		x*=oneOverA, y*=oneOverA, z*=oneOverA;
		return *this;
	}

	MashVector3 MashVector3::operator+(f32 a)const
	{
		return MashVector3(x+a,y+a,z+a);
	}

	MashVector3 MashVector3::operator/(const MashVector3 &v)const
	{
		return MashVector3(x/v.x, y/v.y, z/v.z);
	}

	MashVector3& MashVector3::operator=(const MashVector3 &v)
	{
		x=v.x, y=v.y, z=v.z;
		return *this;
	}

	MashVector3& MashVector3::operator*=(const MashVector3 &v)
	{
		x*=v.x, y*=v.y, z*=v.z;
		return *this;
	}

	MashVector3& MashVector3::operator/=(const MashVector3 &v)
	{
		x/=v.x, y/=v.y, z/=v.z;
		return *this;
	}

	MashVector3& MashVector3::operator+=(f32 a)
	{
		x+=a, y+=a, z+=a;
		return *this;
	}

	f32 MashVector3::operator[](uint32 i)const
	{
		return v[i];
	}

	f32 MashVector3::Dot(const MashVector3 &v)const
	{
		return x*v.x + y*v.y + z*v.z;
	}

	MashVector3 MashVector3::Cross(const MashVector3 &v)const
	{
		return MashVector3(y*v.z-z*v.y,
						-x*v.z+z*v.x,
						x*v.y-y*v.x);
	}

	MashVector3& MashVector3::Normalize()
	{
		f32 magSqr = (x*x) + (y*y) + (z*z);
		if (magSqr > 0.f)
		{
			f32 oneOverMag = 1.f/sqrt(magSqr);
			x *= oneOverMag;
			y *= oneOverMag;
			z *= oneOverMag;
		}

		return *this;
	}

	f32 MashVector3::NormalizeAndGetLength()
	{
		f32 magSqr = (x*x) + (y*y) + (z*z);
		if (magSqr == 0.0f)
			return 0.0f;

		f32 fLength = sqrt(magSqr);
		f32 oneOverMag = 1.f/fLength;
		x *= oneOverMag;
		y *= oneOverMag;
		z *= oneOverMag;

		return fLength;
	}

	f32 MashVector3::Length()const
	{
		return sqrt(x*x + y*y + z*z);
	}

	f32 MashVector3::LengthSq()const
	{
		return x*x + y*y + z*z;
	}

	MashVector3& MashVector3::Zero()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		return *this;
	}

	MashVector3 MashVector3::Lerp(const MashVector3 &vTarget, f32 fAlpha)const
	{
		return MashVector3(this->x + (fAlpha * (vTarget.x - this->x)),
			this->y + (fAlpha * (vTarget.y - this->y)),
			this->z + (fAlpha * (vTarget.z - this->z)));
	}
}