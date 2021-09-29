//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TIMER_H_
#define _MASH_TIMER_H_

#include "MashReferenceCounter.h"
#include "MashDataTypes.h"

namespace mash
{
    /*!
        Timer class.
    */
	class MashTimer : public MashReferenceCounter
	{
	public:
		MashTimer():MashReferenceCounter(){}
		virtual ~MashTimer(){}

        //! Gets the time since this application started in milliseconds.
		virtual uint64 GetTimeSinceProgramStart() = 0;

        //! Gets the fixed update time in seconds.
        /*!
            If the target frame rate is 60fps then this value is equal to 1 / 60.
        */
		virtual f32 GetFixedTimeInSeconds() = 0;
        
        //! Gets the render frame count.
        /*!
			This is equal to the number of times MashGameLoop::Render() is called.
            This is used by some components to only calculate things once per frame.
        */
        virtual uint32 GetFrameCount()const = 0;

		//! Gets the update count.
		/*!
			This is equal to the number of times MashGameLoop::Update() is called.
			This is used by some components to only calculate things once per frame.
		*/
        virtual uint32 GetUpdateCount()const = 0;

		//! Gets the render interpolator time.
		/*!
			Animated objects use this value to interpolate
			between updates.

			\param t Interpolation value between 0.0 and 1.0.
		*/
		virtual f32 GetFrameInterpolatorTime()const = 0;

        //! Increments the frame counter by one.
        virtual void _IncrementFrameCount() = 0;

		//! Increments the update counter by one.
		virtual void _IncrementUpdateCount() = 0;

		//! Sets the render interpolator time.
		/*!
			Animated objects use this value to interpolate
			between updates.

			\param t Interpolation value between 0.0 and 1.0.
		*/
		virtual void _SetFrameInterpolatorTime(f32 t) = 0;
	};
}

#endif