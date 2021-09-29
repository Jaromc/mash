//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashVector4.h"
#include "MashVector3.h"
#include <limits>
#include <math.h>

namespace mash
{
	
	MashVector4::MashVector4():x(0.f),y(0.f),z(0.f),w(0.f){}
	MashVector4::MashVector4(f32 nx, f32 ny, f32 nz, f32 nw):x(nx),y(ny),z(nz),w(nw){}
	MashVector4::MashVector4(f32 nx, f32 ny, f32 nz):x(nx),y(ny),z(nz),w(1.0f){}
	MashVector4::MashVector4(const MashVector3 &v):x(v.x),y(v.y),z(v.z),w(1.0f){}
	MashVector4::MashVector4(const MashVector4 &v):x(v.x),y(v.y),z(v.z),w(v.w){}

	bool MashVector4::operator==(const MashVector4 &other)const
	{
		return (x==other.x && y==other.y && z==other.z && w==other.w);
	}

	MashVector4& MashVector4::operator=(const MashVector3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = 1.0f;
		return *this;
	}

	MashVector4& MashVector4::operator=(const MashVector4 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
		return *this;
	}

	f32 MashVector4::operator[](int32 i)const
	{
		return v[i];
	}

	MashVector4 MashVector4::operator*(const MashVector4 &other)const
	{
		 MashVector4 result( v[0] * other.v[0],
			v[1] * other.v[1],
			v[2] * other.v[2],
			v[3] * other.v[3]);

		return result;
	}

	MashVector4& MashVector4::Zero()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
		return *this;
	}

	MashVector4& MashVector4::Normalize()
	{
		f32 magSqr = (x*x) + (y*y) + (z*z) + (w*w);
		if (magSqr > 0.0f)
		{
			f32 oneOverMag = 1.f/sqrt(magSqr);
			x *= oneOverMag;
			y *= oneOverMag;
			z *= oneOverMag;
			w *= oneOverMag;
		}

		return *this;
	}
}