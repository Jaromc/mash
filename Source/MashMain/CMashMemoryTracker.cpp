//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashMemoryTracker.h"
#include "MashMemoryManager.h"
#include "MashDevice.h"
#include <cstdio>
#include "MashString.h"
#include <sstream>
#include "MashLog.h"

namespace mash
{
	CMashMemoryTracker *CMashMemoryTracker::m_pInstance = 0;

	CMashMemoryTracker::CMashMemoryTracker():m_trackingEnabled(false)
	{

	}

	CMashMemoryTracker::~CMashMemoryTracker()
	{

	}

	void CMashMemoryTracker::DestroyInstance()
	{
		if (m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = 0;
		}
	}

	CMashMemoryTracker* CMashMemoryTracker::Instance()
	{
		if (!m_pInstance)
		{
			m_pInstance = new CMashMemoryTracker();
		}

		return m_pInstance;
	}

	void CMashMemoryTracker::LogAllocation(void *p, int32 iSize, const int8 *sFile, int32 iLine, const int8 *sFunc)
	{
#ifdef MASH_MEMORY_TRACKING_ENABLED
		if (m_trackingEnabled)
		{
			sAllocation newAlloc;
			newAlloc.iPointer = p;
			newAlloc.iSize = iSize;
			newAlloc.sFile = sFile;
			newAlloc.iLineNumber = iLine;
			newAlloc.sFunc = sFunc;

			m_allocations[p] = newAlloc;
		}
#endif
	}

	void CMashMemoryTracker::LogDeallocation(void *p)
	{
#ifdef MASH_MEMORY_TRACKING_ENABLED
		if (m_trackingEnabled)
		{
			std::map<void*, sAllocation>::iterator iter = m_allocations.find(p);
			if (iter != m_allocations.end())
			{
				m_allocations.erase(iter);
			}
		}
#endif
	}

	void CMashMemoryTracker::OutputMemoryLog()
	{
#ifdef MASH_MEMORY_TRACKING_ENABLED

		if (m_allocations.empty())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
				"No memory leaks detected.", 
				"MashMemoryTracker::OutputMemoryLog");

			return;
		}
		std::stringstream errorString;

		errorString << "Memory leaks detected!." << std::endl;

		std::map<void*, sAllocation>::iterator iter = m_allocations.begin();
		std::map<void*, sAllocation>::iterator end = m_allocations.end();
		for(; iter != end; ++iter)
		{
			errorString << "File : " << iter->second.sFile << "\tLine : " << iter->second.iLineNumber << "\tFunction : " << iter->second.sFunc << "\tSize : " << iter->second.iSize << " bytes" << std::endl;
		}

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				errorString.str().c_str(), 
				"MashMemoryTracker::OutputMemoryLog");

		m_allocations.clear();
#endif
	}
}