//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashRectangle2.h"
#include "MashMathHelper.h"
#include "MashVector2.h"

namespace mash
{
	
	MashRectangle2::MashRectangle2():left(0.0f),right(0.0f),top(0.0f),bottom(0.0f)
	{

	}

	MashRectangle2::MashRectangle2(f32 _left, f32 _top, f32 _right, f32 _bottom):
	left(_left),right(_right),top(_top),bottom(_bottom)
	{

	}

	MashRectangle2::MashRectangle2(const MashRectangle2 &copy):
	left(copy.left),right(copy.right),top(copy.top),bottom(copy.bottom)
	{

	}

	void MashRectangle2::MergeGUI(const MashRectangle2 &rect)
	{
		mash::MashVector2 p;
		p.x = rect.left;
		p.y = rect.top;
		AddGUI(p);

		p.x = rect.right;
		p.y = rect.bottom;
		AddGUI(p);
	}

	void MashRectangle2::AddGUI(const mash::MashVector2 &vPoint)
	{
		if (vPoint.x < left)
			left = vPoint.x;
		if (vPoint.x > right)
			right = vPoint.x;
		if (vPoint.y > bottom)
			bottom = vPoint.y;
		if (vPoint.y < top)
			top = vPoint.y;
	}

	void MashRectangle2::MaxBounds()
	{
		left = mash::math::MinFloat();
		right = mash::math::MaxFloat();
		top = mash::math::MaxFloat();
		bottom = mash::math::MinFloat();
	}

	void MashRectangle2::MaxInvBounds()
	{
		left = mash::math::MaxFloat();
		right = mash::math::MinFloat();
		top = mash::math::MinFloat();
		bottom = mash::math::MaxFloat();
	}

	bool MashRectangle2::IsZero()const
	{
		f32 ep = 0.0001f;
		return (math::FloatEqualTo(left, 0.0f, ep) && 
			math::FloatEqualTo(right, 0.0f, ep) && 
			math::FloatEqualTo(top, 0.0f, ep) && 
			math::FloatEqualTo(bottom, 0.0f, ep));
	}

	void MashRectangle2::Zero()
	{
		left = 0.0f;
		right = 0.0f;
		top = 0.0f;
		bottom = 0.0f;
	}

	void MashRectangle2::GetPointsAsSquare(mash::MashVector2 *pPoints)const
	{
		pPoints[0] = mash::MashVector2(left, bottom);
		pPoints[1] = mash::MashVector2(left, top);
		pPoints[2] = mash::MashVector2(right, top);
		pPoints[3] = mash::MashVector2(right, bottom);
	}

	void MashRectangle2::GetPointsAsTris(mash::MashVector2 *pPoints)const
	{
		pPoints[0] = mash::MashVector2(left, bottom);
		pPoints[1] = mash::MashVector2(left, top);
		pPoints[2] = mash::MashVector2(right, top);

		pPoints[3] = mash::MashVector2(left, bottom);
		pPoints[4] = mash::MashVector2(right, top);
		pPoints[5] = mash::MashVector2(right, bottom);
	}

	eCULL MashRectangle2::ClassifyGUI(const MashRectangle2 &other)const
	{
		if (other.IsZero())
			return aCULL_CULLED;
		if ((left >= other.right) || (right <= other.left))
			return aCULL_CULLED;
		if ((top >= other.bottom) || (bottom <= other.top))
			return aCULL_CULLED;

		int32 iCount = 0;
		if (left < other.left)
			++iCount;
		if (right > other.right)
			++iCount;
		if (top < other.top)
			++iCount;
		if (bottom > other.bottom)
			++iCount;
		
		if (iCount != 0)
			return aCULL_CLIPPED;

		return aCULL_VISIBLE;
	}

	eCULL MashRectangle2::ClipGUI(const MashRectangle2 &other)
	{
		int32 iResult = ClassifyGUI(other);
		if (iResult == aCULL_VISIBLE)
			return aCULL_VISIBLE;
		if (iResult == aCULL_CULLED)
		{
			Zero();
			return aCULL_CULLED;
		}

		if (left < other.left)
			left = other.left;
		if (right > other.right)
			right = other.right;
		if (top < other.top)
			top = other.top;
		if (bottom > other.bottom)
			bottom = other.bottom;

		return aCULL_CLIPPED;
	}

	bool MashRectangle2::IntersectsGUI(const mash::MashVector2 &vPoint)const
	{
		if (vPoint.x >= right)
			return false;
		if (vPoint.y >= bottom)
			return false;
		if (vPoint.x <= left)
			return false;
		if (vPoint.y <= top)
			return false;

		return true;
	}

	bool MashRectangle2::operator==(const MashRectangle2 &other)const
	{
		return ((left == other.left) &&
			(right == other.right) &&
			(top == other.top) &&
			(bottom == other.bottom));
	}
}