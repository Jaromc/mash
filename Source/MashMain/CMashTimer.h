//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_TIMER_H_
#define _C_MASH_TIMER_H_

#include "MashTimer.h"

#include "MashDataTypes.h"
#ifdef MASH_WINDOWS
    #ifndef __MINGW32__
    #include <windows.h>
    #else
    #include <windef.h>
    #include <winnt.h>
    #include <winbase.h>
    #endif
#include <Wbemidl.h>
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
#include <sys/time.h>
#endif

namespace mash
{

	class CMashTimer : public MashTimer
    {
	private:
#ifdef MASH_WINDOWS
        LARGE_INTEGER m_iCurrentTime;
        LARGE_INTEGER m_iStartTime;
        LARGE_INTEGER m_cntsPerSec;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
		struct timeval m_startTime;
#endif
        uint32 m_frameCount;
		uint32 m_updateCount;
        f32 m_fixedTimeStep;
		f32 m_interpolatorTime;
	public:
		CMashTimer(f32 fixedTimeStep);
		~CMashTimer();

		uint64 GetTimeSinceProgramStart();
		f32 GetFixedTimeInSeconds();
        uint32 GetFrameCount()const;
		uint32 GetUpdateCount()const;
        void _IncrementFrameCount();
		void _IncrementUpdateCount();

		void _SetFrameInterpolatorTime(f32 t);
		f32 GetFrameInterpolatorTime()const;
	};
}
#endif
