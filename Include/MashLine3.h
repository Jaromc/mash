//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LINE_3_H_
#define _MASH_LINE_3_H_

#include "MashCompileSettings.h"
#include "MashVector3.h"

namespace mash
{
	//! Represents a 3D line with some useful methods.
	class _MASH_EXPORT MashLine3
	{
	public:
		MashVector3 start;
		MashVector3 end;
	public:
		MashLine3();
		MashLine3(const MashVector3 &vStart, const MashVector3 &vEnd);

        //! Calculates the middle of this line.
        /*!
            \return Middle of the line.
        */
		MashVector3 GetMiddle()const;
        
        //! Calculates the vector of this line from start to end.
        /*!
            \return Direction. The returned vector is not normailzed.
        */
		MashVector3 GetVector()const;
	};
}

#endif