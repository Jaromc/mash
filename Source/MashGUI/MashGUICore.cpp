//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashGUICore.h"

namespace mash
{
	void MashGUIUnit::Zero()
	{
		scale = 0.0f;
		offset = 0.0f;
	}

	MashGUIUnit MashGUIUnit::operator+(const MashGUIUnit &other)const
	{
		return MashGUIUnit(scale + other.scale,
			offset + other.offset);
	}

	void MashGUIUnit::operator+=(const MashGUIUnit &other)
	{
		scale += other.scale;
		offset += other.offset;
	}

	MashGUIRect::MashGUIRect():left(0.0f,0.0f),
		top(0.0f, 0.0f),
		right(1.0f, 0.0f),
		bottom(1.0f, 0.0f)
	{
		
	}

	MashGUIRect::MashGUIRect(const MashGUIUnit &L,
			const MashGUIUnit &T,
			const MashGUIUnit &R,
			const MashGUIUnit &B):left(L),
		top(T),
		right(R),
		bottom(B)
	{
		
	}

	MashGUIRect::MashGUIRect(const MashGUIRect &copy):left(copy.left),
		top(copy.top),
		right(copy.right),
		bottom(copy.bottom)
	{
		
	}

	void MashGUIRect::GetAbsoluteValue(const mash::MashRectangle2 &parent,
			mash::MashRectangle2 &out)const
	{
		out.Zero();
		f32 fWidth = parent.right - parent.left;
		f32 fHeight = parent.bottom - parent.top;

		out.left = left.offset + (left.scale * fWidth) + parent.left;
		out.top = top.offset + (top.scale * fHeight) + parent.top;
		out.right = right.offset + (right.scale * fWidth) + parent.left;
		out.bottom = bottom.offset + (bottom.scale * fHeight) + parent.top;

		//TODO : Clamp out to parent
	}

	void MashGUIRect::GetLocalValue(const mash::MashRectangle2 &parent,
			mash::MashRectangle2 &out)const
	{
		out.Zero();
		f32 fWidth = parent.right - parent.left;
		f32 fHeight = parent.bottom - parent.top;

		out.left = left.offset + (left.scale * fWidth);
		out.top = top.offset + (top.scale * fHeight);
		out.right = right.offset + (right.scale * fWidth);
		out.bottom = bottom.offset + (bottom.scale * fHeight);
	}

	void MashGUIRect::GetOffsetValue(mash::MashRectangle2 &out)const
	{
		out.Zero();

		out.left = left.offset;
		out.top = top.offset;
		out.right = right.offset;
		out.bottom = bottom.offset;
	}

	void MashGUIRect::Zero()
	{
		left.Zero();
		top.Zero();
		right.Zero();
		bottom.Zero();
	}

	MashGUIRect MashGUIRect::operator+(const MashGUIRect &other)const
	{
		return MashGUIRect(left + other.left,
			right + other.right,
			top + other.top,
			bottom + other.bottom);
	}

	void MashGUIRect::operator+=(const MashGUIRect &other)
	{
		left += other.left;
		right += other.right;
		top += other.top;
		bottom += other.bottom;
	}
}