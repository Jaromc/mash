//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef MASH_GUI_CORE_H_
#define MASH_GUI_CORE_H_

#include "MashCompileSettings.h"
#include "MashTypes.h"
#include "MashRectangle2.h"

namespace mash
{
	struct _MASH_EXPORT MashGUIUnit
	{
	public:
		f32 scale;
		f32 offset;
	public:
		MashGUIUnit():scale(0.0f), offset(0.0f){}
		MashGUIUnit(f32 s, f32 o):scale(s),offset(o){}
		MashGUIUnit(const MashGUIUnit &copy):scale(copy.scale),offset(copy.offset){}

		//! Zeros out all values.
		void Zero();

		//! Operators.
		MashGUIUnit operator+(const MashGUIUnit &other)const;
		void operator+=(const MashGUIUnit &other);
	};

	class _MASH_EXPORT MashGUIRect
	{
	public:
		MashGUIUnit left, top, right, bottom;
	public:
		MashGUIRect();
		MashGUIRect(const MashGUIUnit &L,
			const MashGUIUnit &T,
			const MashGUIUnit &R,
			const MashGUIUnit &B);
		MashGUIRect(const MashGUIRect &copy);

		//! This rect expressed in abs coordinates using the parent as reference.
		/*!
			This resolves the scale attributes + offsets using the parent rect.
			\param parent Reference rect.
			\param out This rect in absolute coordinates.
		*/
		void GetAbsoluteValue(const mash::MashRectangle2 &parent,
			mash::MashRectangle2 &out)const;

		//! This rect expressed in local coordinates using the parent as reference.
		/*!
			This resolves the scale attributes + offsets using the parent rect.
			\param parent Reference rect.
			\param out This rect in local coordinates.
		*/
		void GetLocalValue(const mash::MashRectangle2 &parent,
			mash::MashRectangle2 &out)const;

		//! Returns the offset values.
		/*!
			\param out Offset values.
		*/
		void GetOffsetValue(mash::MashRectangle2 &out)const;

		//! Zeros out all values of this rect.
		void Zero();

		//! Operators.
		MashGUIRect operator+(const MashGUIRect &other)const;
		void operator+=(const MashGUIRect &other);
	};
}

#endif