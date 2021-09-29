//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_REFERENCE_COUNTER_H_
#define _MASH_REFERENCE_COUNTER_H_

#include "MashMemoryObject.h"

namespace mash
{
    /*!
        Simple reference counting. Most objects within the engine are derived from this.
    */
	class MashReferenceCounter : public MashMemoryObject
	{
	private:
        mutable int32 m_referenceCounter;
	public:
		MashReferenceCounter():MashMemoryObject(),m_referenceCounter(1)
		{
		}

		virtual ~MashReferenceCounter()
		{
		}

        //! Increments the reference counter by one.
		void Grab()const
        {
            ++m_referenceCounter;
        }

        //! Decrements the reference counter by one.
        /*!
            \return True if the counter has reached zero and has been marked for deletion. False otherwise.
        */
		bool Drop()
		{
			MASH_ASSERT(m_referenceCounter != 0);

			--m_referenceCounter;

			if (m_referenceCounter == 0)
			{
				OnDelete();
				return true;
			}

			return false;
		}

        //! Customize what happens when the reference counter reaches zero.
		/*!
			Objects don't necessarily need to be deleted straight away, deletion
			could be delayed until later.
		*/
		virtual void OnDelete()
		{
			MASH_DELETE this;
		}

        //! Returns the reference count.
		int32 GetReferenceCount()const
		{
			return m_referenceCounter;
		}
	};
}

#endif