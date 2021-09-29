//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashSphere.h"

namespace mash
{
	MashSphere::MashSphere():radius(0.0f), center(0.0f, 0.0f, 0.0f)
	{
		
	}

	MashSphere::MashSphere(const MashSphere &copy):radius(copy.radius), center(copy.center)
	{
		
	}

	MashSphere::MashSphere(f32 r, const mash::MashVector3 &c):radius(r), center(c)
	{
		
	}
}