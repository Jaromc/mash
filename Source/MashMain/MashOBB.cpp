//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashOBB.h"
#include "MashQuaternion.h"
#include "MashMatrix4.h"
#include <cstring>

namespace mash
{
	MashOBB::MashOBB():center(0.0f, 0.0f, 0.0f),
		halfWidth(0.5f, 0.5f, 0.5f)
	{
		localAxis[0] = MashVector3(1,0,0);//right
		localAxis[1] = MashVector3(0,1,0);//up
		localAxis[2] = MashVector3(0,0,1);//forward
	}

	MashOBB::MashOBB(const MashOBB &OBB):center(OBB.center),
		halfWidth(OBB.halfWidth)
	{
		memcpy(localAxis, OBB.localAxis, sizeof(MashVector3) * 3);
	}

	MashOBB::MashOBB(const MashVector3 &vCenter,
		const MashVector3 &x,
		const MashVector3 &y,
		const MashVector3 &z,
		const MashVector3 &vHalfWidth):center(vCenter),
		halfWidth(vHalfWidth)
	{
		localAxis[0] = x;
		localAxis[1] = y;
		localAxis[2] = z;
	}
	
	MashOBB::~MashOBB()
	{
		
	}

	MashOBB& MashOBB::Transform(const MashQuaternion &q)
	{
		localAxis[0] = q.TransformVector(localAxis[0]);
		localAxis[1] = q.TransformVector(localAxis[1]);
		localAxis[2] = q.TransformVector(localAxis[2]);

		return *this;
	}

	MashOBB& MashOBB::Transform(const MashMatrix4 &m)
	{
		m.TransformRotation(localAxis[0], localAxis[0]);
		m.TransformRotation(localAxis[1], localAxis[1]);
		m.TransformRotation(localAxis[2], localAxis[2]);

		return *this;
	}

	void MashOBB::ClosestPoint(const MashVector3 &vPoint, MashVector3 &out)const
	{
		MashVector3 vDist = vPoint - center;

		out = center;

		for(int32 i = 0; i < 3; ++i)
		{
			//projects vDist onto the axis
			f32 fDist = vDist.Dot(localAxis[i]);

			//if dist is greater than box extents than clamp
			if (fDist > halfWidth.v[i])
				fDist = halfWidth.v[i];
			if (fDist < -halfWidth.v[i])
				fDist = -halfWidth.v[i];

			//move distance along the current axis to
			//transform it into world coordinates
			out += localAxis[i] * fDist;
		}
	}
}