//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashLine3.h"

namespace mash
{
	MashLine3::MashLine3():start(0.0f,0.0f,0.0f),end(0.0f,0.0f,0.0f)
	{
	}

	MashLine3::MashLine3(const MashVector3 &vStart, const MashVector3 &vEnd):start(vStart), end(vEnd)
	{
		
	}

	MashVector3 MashLine3::GetMiddle()const
	{
		return (start + end) * 0.5f;
	}

	MashVector3 MashLine3::GetVector()const
	{
		return end - start;
	}
}