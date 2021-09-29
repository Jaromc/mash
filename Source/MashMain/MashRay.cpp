//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashRay.h"
#include "MashMatrix4.h"
#include "MashTransformState.h"
#include <cmath>

namespace mash
{
	MashRay::MashRay():origin(0.0f,0.0f,0.0f), dir(0.0f,0.0f,1.0f){}

	MashRay::MashRay(const MashVector3 &vOrigin,
			const MashVector3 &vDir)
	{
		origin = vOrigin;
		dir = vDir;

		if (dir.IsZero())
			dir.z = 1.0f;

		dir.Normalize();
	}

	MashRay::MashRay(const MashRay &ray)
	{
		origin = ray.origin;
		dir = ray.dir;
	}

	void MashRay::Transform(const MashMatrix4 &m)
	{
		origin = m.TransformVector(origin);
		dir = m.TransformRotation(dir);

		dir.Normalize();
	}

	void MashRay::Transform(const MashTransformState &state)
	{
		Transform(state.ToMatrix());
	}

	void MashRay::TransformInverse(const MashTransformState &state)
	{
		Transform(state.ToMatrix().Invert());
	}

	//intersection test against a triangle.
	//set bCull to true if using backface culling.
	//if fLength is given a value then it checks that the intersection
	//occured within the length (basically a line segment test)
	bool MashRay::Intersects(const mash::MashVector3 &a,
						const mash::MashVector3 &b,
						const mash::MashVector3 &c,
						bool bCull,
						f32 *t)const
	{
		MashVector3 vP1, vP2, vP3;
		MashVector3 e1 = b-a;
		MashVector3 e2 = c-a;
		f32 fDistance = 0;

		vP1 = dir.Cross(e2);

		//if close to 0 then ray is parallel
		f32 fDet = e1.Dot(vP1);
		if ((bCull) && (fDet < 0.00001f))
			return false;
		else if ((fDet < 0.00001f) && (fDet > -0.00001f))
			return false;

		//distance to plane, < 0 means ray is behind the plane
		vP2 = origin - a;
		f32 u = vP2.Dot(vP1);
		if (u < 0.0f || u > fDet)
			return false;

		vP3 = vP2.Cross(e1);
		f32 v = dir.Dot(vP3);
		if ((v < 0.0f) || (u+v > fDet))
			return false;

		fDistance = e2.Dot(vP3) / fDet;
		if (fDistance < 0.0f)
			return false;

		//returns the length if we want it
		if (t)
		{
			*t = fDistance;
		}

		return true;
	}

	//intersection test with a plane.
	//set bCull to true if using backface culling.
	//if fLength is given a value then it checks that the intersection
	//occured within the length (basically a line segment test)
	bool MashRay::Intersects(//const MashPlane &plane,
						const mash::MashVector3 &vPlaneNormal,
						f32 fPlaneD,
						bool bCull,
						f32 *t)const
	{
		f32 fD = vPlaneNormal.Dot(dir);

		//ray is parallel to the plane
		if (fabs(fD) < 0.00001f)
			return false;

		//plane normal points away from the ray direction
		// if >= then intersection with back face
		if (bCull &&(fD > 0.0f))
			return false;

		f32 f1 = -((vPlaneNormal.Dot(origin)) + fPlaneD);

		f32 fTempT = f1/fD;

		//intersection before ray origin
		if (fTempT < 0.0f)
			return false;

		if (t)
		{
			*t = fTempT;
		}

		return true;
	}

	bool MashRay::Intersects(/*const MashAABB &aabb*/const mash::MashVector3 &vMin,
			const mash::MashVector3 &vMax)const
	{
		const f32 fEpsilon = 0.00001f;
		f32 tMin = 0.0f;
		f32 tMax = 99999999.f;
		MashVector3 p(origin.x,origin.y,origin.z );
		MashVector3 d(dir.x,dir.y,dir.z);

		for (int32 i = 0; i < 3; i++)
		{
			if (fabs(d.v[i]) < fEpsilon)
			{
				if ((p.v[i] < vMin.v[i]) || (p.v[i] > vMax.v[i]))
					return false;
			}
			else
			{
				f32 ood = 1.f/d.v[i];
				f32 t1 = (vMin.v[i] - p.v[i])*ood;
				f32 t2 = (vMax.v[i] - p.v[i])*ood;

				if (t1 > t2)
				{
					f32 temp = t1;
					t1 = t2;
					t2 = temp;
				}

				if (t1 > tMin)
					tMin = t1;
				if (t2 > tMax)
					tMax = t2;

				if (tMin > tMax)
					return false;
			}
		}

		return true;
	}
}