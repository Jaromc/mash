//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTimer.h"

namespace mash
{
	CMashTimer::CMashTimer(f32 fixedTimeStep): m_frameCount(0),
    m_fixedTimeStep(fixedTimeStep),
	m_updateCount(0),
	m_interpolatorTime(0.0f)
	{
#ifdef MASH_WINDOWS
        QueryPerformanceFrequency(&m_cntsPerSec);
        
        QueryPerformanceCounter(&m_iStartTime);
        
		m_iCurrentTime = m_iStartTime;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
		gettimeofday(&m_startTime, NULL);
#endif
	}
    
	CMashTimer::~CMashTimer()
	{
        
	}
    
	uint64 CMashTimer::GetTimeSinceProgramStart()
	{
#ifdef MASH_WINDOWS
        QueryPerformanceCounter(&m_iCurrentTime);
		return (m_iCurrentTime.QuadPart - m_iStartTime.QuadPart) * 1000 / m_cntsPerSec.QuadPart;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
		struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        return ((currentTime.tv_sec - m_startTime.tv_sec) * 1000) + ((currentTime.tv_usec - m_startTime.tv_usec) / 1000);
#endif
	}

	f32 CMashTimer::GetFixedTimeInSeconds()
	{
		return m_fixedTimeStep;
	}
    
    uint32 CMashTimer::GetFrameCount()const
    {
        return m_frameCount;
    }

	uint32 CMashTimer::GetUpdateCount()const
    {
        return m_updateCount;
    }
    
    void CMashTimer::_IncrementFrameCount()
    {
        ++m_frameCount;
    }

	void CMashTimer::_IncrementUpdateCount()
    {
        ++m_updateCount;
    }

	void CMashTimer::_SetFrameInterpolatorTime(f32 t)
	{
		m_interpolatorTime = t;
	}

	f32 CMashTimer::GetFrameInterpolatorTime()const
	{
		return m_interpolatorTime;
	}
}
