//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashMathHelper.h"
#include "MashVector3.h"

namespace mash
{
    namespace math
    {
	mash::MashVector3 ComputeFaceNormal(const mash::MashVector3 &a,
		const mash::MashVector3 &b,
		const mash::MashVector3 &c)
	{
		MashVector3 ab = b - a;
		MashVector3 ac = c - a;

		return ab.Cross(ac).Normalize();
	}
    }
}