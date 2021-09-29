//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashTriangle.h"
#include "MashRay.h"
#include "MashMathHelper.h"

namespace mash
{
	MashTriangle::MashTriangle():pointA(0.0f,0.0f,0.0f),pointB(0.0f,0.0f,0.0f),pointC(0.0f,0.0f,0.0f)
	{
	
	}

	MashTriangle::MashTriangle(const MashTriangle &copy):pointA(copy.pointA),pointB(copy.pointB),pointC(copy.pointC)
	{
		
	}

	MashTriangle::MashTriangle(const MashVector3 &a,
		const MashVector3 &b,
		const MashVector3 &c):pointA(a),pointB(b),pointC(c)
	{
	
	}

	MashVector3& MashTriangle::operator[](int32 i)
	{
		if (i <= 0)
			return pointA;
		else if (i == 1)
			return pointB;
		else
			return pointC;
	}

	const MashVector3& MashTriangle::operator[](int32 i)const
	{
		if (i <= 0)
			return pointA;
		else if (i == 1)
			return pointB;
		else
			return pointC;
	}

	mash::MashVector3 MashTriangle::GenerateNormal()const
	{
		return math::ComputeFaceNormal(pointA, pointB, pointC);
	}

	bool MashTriangle::Intersects(const mash::MashRay &ray, mash::MashVector3 &collisionPoint)const
	{
		MashVector3 ab = pointB - pointA;
		MashVector3 ac = pointC - pointA;
		MashVector3 qp = -ray.dir;

		//triangle normal
		MashVector3 n = ab.Cross(ac);

		f32 d = qp.Dot(n);
		if (d <= 0.0f)
			return false;

		MashVector3 ap = ray.origin - pointA;
		f32 t = ap.Dot(n);
		if (t < 0.0f)
			return false;

		MashVector3 e = qp.Cross(ap);
		f32 v = ac.Dot(e);
		if ((v < 0.0f) || (v > d))
			return false;
		f32 w = -(ab.Dot(e));
		if ((w < 0.0f) || ((v+w) > d))
			return false;

		f32 ood = 1.0f / d;
		collisionPoint = ray.origin - (qp * (t*ood));
		return true;
	}

	void MashTriangle::ClosestPoint(const MashVector3 &point, MashVector3 &closestPoint)const
	{
		MashVector3 ab = pointB - pointA;
		MashVector3 ac = pointC - pointA;
		MashVector3 ap = point - pointA;
		f32 d1 = ab.Dot(ap);
		f32 d2 = ac.Dot(ap);

		if ((d1 <= 0.0f) && (d2 <= 0.0f))
		{
			closestPoint = pointA;
			return;
		}

		MashVector3 bp = point - pointB;
		f32 d3 = ab.Dot(bp);
		f32 d4 = ac.Dot(bp);

		if ((d3 >= 0.0f) && (d4 <= d3))
		{
			closestPoint = pointB;
			return;
		}

		f32 vc = d1*d4 - d3*d2;
		if ((vc <= 0.0f) && (d1 >= 0.0f) && (d3 <= 0.0f))
		{
			f32 v = d1/(d1-d3);
			closestPoint = pointA+(ab*v);
			return;
		}

		MashVector3 cp = point-pointC;
		f32 d5 = ab.Dot(cp);
		f32 d6 = ac.Dot(cp);

		if ((d6 >= 0.0f) && (d5 <= d6))
		{
			closestPoint = pointC;
			return;
		}

		f32 vb = d5*d2 - d1*d6;

		if ((vb <= 0.0f) && (d2 >= 0.0f) && (d6 <= 0.0f))
		{
			f32 w = d2/(d2-d6);
			closestPoint = pointA+(ac*w);
			return;
		}

		f32 va = d3*d6 - d5*d4;
		if ((va <= 0.0f) && ((d4 - d3) >= 0.0f) && ((d5-d6) >= 0.0f))
		{
			f32 w = (d4-d3)/((d4-d3)+(d5-d6));
			closestPoint = pointB+((pointC-pointB)*w);
			return;
		}

		f32 denom = 1.0f/(va+vb+vc);
        f32 fv = vb*denom;
        f32 fw = vc*denom;
        closestPoint = pointA+ab * fv+ac * fw;
        
        return;
	}
}