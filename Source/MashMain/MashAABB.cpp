//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashAABB.h"
#include "MashMatrix4.h"
#include "MashPlane.h"
#include "MashMathHelper.h"
#include "MashTransformState.h"

namespace mash
{
	MashAABB::MashAABB()
	{
		min = MashVector3(mash::math::MaxFloat(),mash::math::MaxFloat(),mash::math::MaxFloat());
		max = MashVector3(mash::math::MinFloat(),mash::math::MinFloat(),mash::math::MinFloat());
	}

	MashAABB::MashAABB(const MashVector3 &vMin,
			const MashVector3 &vMax)
	{
		SetLimits(vMin, vMax);
		
	}

	MashAABB::MashAABB(const MashAABB &aabb)
	{
		min = aabb.min;
		max = aabb.max;
	}

	mash::MashVector3 MashAABB::GetCenter()const
	{
		return mash::MashVector3((min+max)*0.5f);
	}

	void MashAABB::GetCenter(mash::MashVector3 &out)const
	{
		out = (min+max)*0.5f;
	}

	void MashAABB::Add(const MashVector3 &vPoint)
	{
		if (vPoint.x < min.x)
			min.x = vPoint.x;
		if (vPoint.x > max.x)
			max.x = vPoint.x;
		if (vPoint.y < min.y)
			min.y = vPoint.y;
		if (vPoint.y > max.y)
			max.y = vPoint.y;
		if (vPoint.z < min.z)
			min.z = vPoint.z;
		if (vPoint.z > max.z)
			max.z = vPoint.z;
	}

	void MashAABB::ClosestPoint(const MashVector3 &vPoint, MashVector3 &out)const
	{
		/*
			If the point coordinate value is outside the box then
			clamp it to the box, else keep it as is.
		*/
		for(int32 i = 0; i < 3; ++i)
		{
			f32 v = vPoint.v[i];
			if (v < min.v[i])
				v = min.v[i];
			if (v > max.v[i])
				v = max.v[i];
			out.v[i] = v;
		}
	}

	//returns the planes that make up the sides
	//of the aabb.
	void MashAABB::GetPlanes(MashArray<MashPlane> &planes)const
	{
		MashVector3 vNormal;
	
		//right
		vNormal.x = 1.0f; vNormal.y = 0.0f; vNormal.z = 0.0f;
		MashPlane p1(vNormal, max);
		planes.PushBack(p1);
		//left
		vNormal.x = -1.0f; vNormal.y = 0.0f; vNormal.z = 0.0f;
		MashPlane p2(vNormal, min);
		planes.PushBack(p2);
	
		//front
		vNormal.x = 0.0f; vNormal.y = 0.0f; vNormal.z = -1.0f;
		MashPlane p3(vNormal, min);
		planes.PushBack(p3);
	
		//back
		vNormal.x = 0.0f; vNormal.y = 0.0f; vNormal.z = 1.0f;
		MashPlane p4(vNormal, max);
		planes.PushBack(p4);
	
		//top
		vNormal.x = 0.0f; vNormal.y = 1.0f; vNormal.z = 0.0f;
		MashPlane p5(vNormal, max);
		planes.PushBack(p5);
	
		//bottom
		vNormal.x = 0.0f; vNormal.y = -1.0f; vNormal.z = 0.0f;
		MashPlane p6(vNormal, min);
		planes.PushBack(p6);
	}

	void MashAABB::AddPoint(f32 x, f32 y, f32 z)
	{
		if (min.x > x)
			min.x = x;
		if (min.y > y)
			min.y = y;
		if (min.z > z)
			min.z = z;

		if (max.x < x)
			max.x = x;
		if (max.y < y)
			max.y = y;
		if (max.z < z)
			max.z = z;
	}

	//merges this aabb with another
	void MashAABB::Merge(const MashAABB &aabb)
	{
		AddPoint(aabb.min.x, aabb.min.y, aabb.min.z);
		AddPoint(aabb.max.x, aabb.max.y, aabb.max.z);
	}

	void MashAABB::Zero()
	{
		min.Zero();
		max.Zero();
	}

	//returns true if the aabb intersects with this one
	bool MashAABB::Intersects(const MashAABB &aabb)const
	{
		if (min.x > aabb.max.x || aabb.min.x > max.x)
			return false;
		if (min.y > aabb.max.y || aabb.min.y > max.y)
			return false;
		if (min.z > aabb.max.z || aabb.min.z > max.z)
			return false;

		return true;
	}

	//returns true if the point is inside the aabb
	bool MashAABB::Intersects(const MashVector3 &vPoint)const
	{
		if (vPoint.x >= max.x)
			return false;
		if (vPoint.y >= max.y)
			return false;
		if (vPoint.z >= max.z)
			return false;
		if (vPoint.x <= min.x)
			return false;
		if (vPoint.y <= min.y)
			return false;
		if (vPoint.z <= min.z)
			return false;

		return true;
	}

	//returns the verticies of this aabb
	void MashAABB::GetVerticies(MashVector3 *out)const
	{
			//       8    7
			//       x -- x
			//   5 / | 6/ |
			//   x   | x  |
			//   |   x |- x 
			//   | / 4 | /3
			//   x --- x
			//   1     2

		out[0].x = min.x;
		out[0].y = min.y;
		out[0].z = min.z;

		out[1].x = max.x;
		out[1].y = min.y;
		out[1].z = min.z;

		out[2].x = max.x;
		out[2].y = max.y;
		out[2].z = min.z;

		out[3].x = min.x;
		out[3].y = max.y;
		out[3].z = min.z;

		out[4].x = min.x;
		out[4].y = min.y;
		out[4].z = max.z;

		out[5].x = max.x;
		out[5].y = min.y;
		out[5].z = max.z;

		out[6].x = max.x;
		out[6].y = max.y;
		out[6].z = max.z;

		out[7].x = min.x;
		out[7].y = max.y;
		out[7].z = max.z;
	}

	void MashAABB::Repair()
	{
		f32 t;

		if (min.x > max.x)
			{ t=min.x; min.x = max.x; max.x=t; }
		if (min.y > max.y)
			{ t=min.y; min.y = max.y; max.y=t; }
		if (min.z > max.z)
			{ t=min.z; min.z = max.z; max.z=t; }
	}

	void MashAABB::Scale(const MashVector3 &vScale)
	{
		mash::MashMatrix4 scale;
		scale.CreateScale(vScale);

		//center = scale * center;
		max = scale.TransformVector(max);
		min = scale.TransformVector(min);

		Repair();
	}

	MashAABB& MashAABB::Transform(const MashTransformState &state)
	{
		return Transform(state.ToMatrix());
	}

	MashAABB& MashAABB::TransformInverse(const MashTransformState &state)
	{
		return Transform(state.ToMatrix().Invert());
	}

	MashAABB& MashAABB::Transform(const MashMatrix4 &matrix)
	{
		const f32 Amin[3] = {min.x, min.y, min.z};
		const f32 Amax[3] = {max.x, max.y, max.z};

		f32 Bmin[3];
		f32 Bmax[3];

		Bmin[0] = Bmax[0] = matrix.m41;
		Bmin[1] = Bmax[1] = matrix.m42;
		Bmin[2] = Bmax[2] = matrix.m43;

		for (int32 i = 0; i < 3; ++i)
		{
			for (int32 j = 0; j < 3; ++j)
			{
				const f32 a = matrix.m[j][i] * Amin[j];
				const f32 b = matrix.m[j][i] * Amax[j];

				if (a < b)
				{
					Bmin[i] += a;
					Bmax[i] += b;
				}
				else
				{
					Bmin[i] += b;
					Bmax[i] += a;
				}
			}
		}

		min.x = Bmin[0];
		min.y = Bmin[1];
		min.z = Bmin[2];

		max.x = Bmax[0];
		max.y = Bmax[1];
		max.z = Bmax[2];

		return *this;
	}

	void MashAABB::Transform(const MashMatrix4 &matrix, MashAABB &out)const
	{
		out = *this;
		out.Transform(matrix);
	}

	//sets the limits
	void MashAABB::SetLimits(const MashVector3 &vMin,
					const MashVector3 &vMax)
	{
		min = vMin;
		max = vMax;
	}
}