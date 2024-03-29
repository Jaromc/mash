#include "TimeHelpers.h"

#ifndef __MINGW32__
#include <windows.h>
#else
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#endif

namespace UnitTest {

Timer::Timer()
    : m_startTime(0)
	, m_threadHandle(::GetCurrentThread())
{
#if defined(_MSC_VER) && (_MSC_VER == 1200) // VC6 doesn't have DWORD_PTR?
	typedef unsigned long DWORD_PTR;
#endif

    DWORD_PTR systemMask;
    ::GetProcessAffinityMask(GetCurrentProcess(), &m_processAffinityMask, &systemMask);
    
    ::SetThreadAffinityMask(m_threadHandle, 1);
	::QueryPerformanceFrequency(reinterpret_cast< LARGE_INTEGER* >(&m_frequency));
    ::SetThreadAffinityMask(m_threadHandle, m_processAffinityMask);
}

void Timer::Start()
{
    m_startTime = GetTime();
}

int Timer::GetTimeInMs() const
{
    __int64 const elapsedTime = GetTime() - m_startTime;
	double const seconds = double(elapsedTime) / double(m_frequency);
	return int(seconds * 1000.0f);
}

__int64 Timer::GetTime() const
{
    LARGE_INTEGER curTime;
    ::SetThreadAffinityMask(m_threadHandle, 1);
	::QueryPerformanceCounter(&curTime);
    ::SetThreadAffinityMask(m_threadHandle, m_processAffinityMask);
    return curTime.QuadPart;
}



void TimeHelpers::SleepMs(int const ms)
{
	::Sleep(ms);
}

}
