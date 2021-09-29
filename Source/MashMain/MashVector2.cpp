//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashVector2.h"
#include <math.h>

namespace mash
{
	MashVector2::MashVector2():x(0.f),y(0.f){}
	MashVector2::MashVector2(f32 nx, f32 ny):x(nx),y(ny){}

	MashVector2 MashVector2::operator-(const MashVector2 &other)const
	{
		return MashVector2(x-other.x,y-other.y);
	}

	MashVector2 MashVector2::operator+(const MashVector2 &other)const
	{
		return MashVector2(x+other.x,y+other.y);
	}

	bool MashVector2::operator==(const MashVector2 &other)const
	{
		return ((x == other.x) && (y == other.y));
	}

	MashVector2 MashVector2::operator*(f32 f)const
	{
		return MashVector2(x*f,y*f);
	}

	f32 MashVector2::Length()const
	{
		return sqrt(x*x + y*y);
	}

	MashVector2& MashVector2::Zero()
	{
		x = 0.0f;
		y = 0.0f;
		return *this;
	}
}