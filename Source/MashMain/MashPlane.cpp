//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashPlane.h"
#include "MashLine3.h"

namespace mash
{
	MashPlane::MashPlane():thickness(0.1f),
		dist(0.0f)
	{

	}

	MashPlane::MashPlane(const MashVector3 &a,
			const MashVector3 &b,
			const MashVector3 &c,
			f32 fThickness)
	{
		MashVector3 e1 = b-a;
		MashVector3 e2 = c-a;

		normal = e1.Cross(e2).Normalize();
		dist = -normal.Dot(a);

		thickness = fThickness;
	}

	MashPlane::MashPlane(const MashVector3 &vNormal,
			const MashVector3 &vPoint,
			f32 fThickness,
			bool callNormalize)
	{
		normal = vNormal;

		if (callNormalize)
			normal.Normalize();

		dist = -normal.Dot(vPoint);

		thickness = fThickness;
	}

	//calculates the distance from the point to the plane
	f32 MashPlane::Distance(const MashVector3 &point)const
	{
		return (point.Dot(normal) + dist);
	}

	bool MashPlane::Intersect(const MashLine3 &line, f32 &t)const
	{
		return Intersect(line.start, line.end, t);
	}

	bool MashPlane::Intersect(const mash::MashVector3 &start, const mash::MashVector3 &end, f32 &t)const
	{
		MashVector3 ab = end-start;
		f32 t2 = normal.Dot(ab);
		if (t2 == 0.0f)
			return false;

		t = -(dist + (normal.Dot(start))) / t2;

		if ((t > 0.0f) && (t < 1.0f))
			return true;

		return false;
	}

	eCLASSIFY MashPlane::Classify(const MashVector3 &point)const
	{
		f32 distance = normal.Dot(point) + dist;

		if (distance > thickness)
			return aCLASS_FRONT;
		if (distance < -thickness)
			return aCLASS_BEHIND;
		return aCLASS_STRADDLE;
	}

	eCLASSIFY MashPlane::Classify(const MashVector3 *pPointList, uint32 iPointCount)const
	{
		int32 numFront = 0, numBehind = 0;
		for(int32 i = 0; i < iPointCount; i++)
		{
			switch(Classify(pPointList[i]))
			{
			case aCLASS_FRONT:
				{
					numFront++;
					break;
				}
			case aCLASS_BEHIND:
				{
					numBehind++;
					break;
				}
			default:
				{
				break;
				}
			}
		}

			if (numFront == iPointCount)
				return aCLASS_FRONT;
			if (numBehind == iPointCount)
				return aCLASS_BEHIND;

			return aCLASS_STRADDLE;
	}
}