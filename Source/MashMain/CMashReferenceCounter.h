//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_REFERENCE_COUNTER_H_
#define _CMASH_REFERENCE_COUNTER_H_

#include "MashMemoryObject.h"
#include "MashMemory.h"

namespace mash
{
	class CMashReferenceCounter : public MashMemoryObject
	{
	public:
		CMashReferenceCounter():MashMemoryObject(),m_iReferenceCounter(1)
		{
		}

		virtual ~CMashReferenceCounter()
		{
		}

		virtual void Grab()const{++m_iReferenceCounter;}

		virtual bool Drop()const
		{
			--m_iReferenceCounter;

			if (m_iReferenceCounter == 0)
			{
				MASH_DELETE this;
				return true;
			}

			return false;
		}

		int32 GetReferenceCount()const
		{
			return m_iReferenceCounter;
		}

	private:
		mutable int32 m_iReferenceCounter;
	};
}

#endif