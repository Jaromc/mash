//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_RESOURCE_H_
#define _MASH_RESOURCE_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashTypes.h"

namespace mash
{
    /*!
        Objects that derive from this are video resource.
    */
	class MashResource : public MashReferenceCounter
	{
	public:
		MashResource(){}
		virtual ~MashResource(){}

        //! Gets the resource type.
		virtual eRESOURCE_TYPE GetType()const = 0;

        //! Gets the usage type.
		/*
			This function is usful for determining if a resources buffer
			can be locked for writing.
		*/
		virtual eUSAGE GetUsageType()const = 0;
	};
}

#endif