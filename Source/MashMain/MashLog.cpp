//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashLog.h"
#include "MashMemory.h"
#include "MashDevice.h"
#include <cstdarg>
#include "MashEventTypes.h"
#include "MashMathHelper.h"

namespace mash
{
	MashLog *MashLog::m_instance = 0;

    MashLog::MashLog():m_log(0), m_errorLevelFlags(mash::math::MaxUInt32()), m_receiverID(0),
        m_suppressMessages(false)
	{
	}

	MashLog* MashLog::Instance()
	{
		if (!m_instance)
			m_instance = new MashLog();

		return m_instance;
	}

	void MashLog::DestroyInstance()
	{
		if (m_instance)
		{
            delete m_instance;
			m_instance = 0;
		}
	}

	MashLog::~MashLog()
	{
		CloseLog();
	}

	// Closes the current log if it is open.
	void MashLog::CloseLog()
	{
		if (m_log != 0)
		{
			m_receivers.Clear();

			WriteToLog(aERROR_LEVEL_INFORMATION, "Log Closed", "MashLog::CloseLog");

			fflush(m_log);
			fclose(m_log);
			m_log = 0;
		}
	}

	void MashLog::CreateLog()
	{
		if (m_log == 0)
		{
			if (MashDevice::StaticDevice)
				m_log = fopen(MashDevice::StaticDevice->GetDebugFilePath().GetCString(), "w");
		}
	}

	uint32 MashLog::AddReceiver(MashLogEventFunctor callback)
	{
		m_receivers.PushBack(sReceiver(callback, m_receiverID++));
		return (m_receiverID-1);
	}
    
    void MashLog::SuppressMessages(bool val)
    {
        m_suppressMessages = val;
    }

	void MashLog::RemoveReceiver(uint32 id)
	{
		const uint32 receiverCount = m_receivers.Size();
		for(uint32 i = 0; i < receiverCount; ++i)
		{
			if (m_receivers[i].id == id)
			{
				m_receivers.Erase(m_receivers.Begin() + i);
				return;
			}
		}
	}

	void MashLog::WriteBoundsError(int32 value, int32 minVal, int32 maxVal, int8 *valName, int8* functionName)
	{
		WriteToLogEx(aERROR_LEVEL_ERROR, functionName,
			"%s index '%d' is beyond array bounds of '%d'-'%d'.",valName, value, minVal, maxVal);
	}

	void MashLog::WriteToLogEx(eERROR_LEVEL level, const int8 *sFunctionName, const int8 *sMsg, ...)
	{
		va_list args;
		int8 buffer[500];
		va_start(args, sMsg);
		vsnprintf(buffer, sizeof(buffer), sMsg, args);
		va_end(args);

		WriteToLog(level, buffer, sFunctionName);
	}

	// Writes a messge to the current log file.
	void MashLog::WriteToLog(eERROR_LEVEL level, const int8 *sMsg, const int8 *sFunctionName)
	{
		if (m_suppressMessages)
            return;
        
		if (m_errorLevelFlags & (uint32)level)
		{
			if (m_log == 0)
			{
				CreateLog();

				if (m_log == 0)
					return;
			}
            else
            {
            }

			MashStringc message;
			switch(level)
			{
			case aERROR_LEVEL_ERROR:
				message = "**Error** Location : ";
				break;
			case aERROR_LEVEL_INFORMATION:
				message = "**Information** Location : ";
				break;
			case aERROR_LEVEL_WARNING:
				message = "**Warning** Location : ";
				break;
                default:
                    message = "**Message** Location : ";
			};

			if (sFunctionName)
			{
				message += sFunctionName;
			}

			message += ". Message : ";

			if (sMsg)
			{
				message += sMsg;
			}

			message += "\n";


			fprintf(m_log, message.GetCString());

			fflush(m_log);

			if (!m_receivers.Empty())
			{
				sLogEvent e;
				e.msg = sMsg;
				e.level = level;

				const uint32 receiverCount = m_receivers.Size();
				for(uint32 i = 0; i < receiverCount; ++i)
					m_receivers[i].callback.Call(e);
			}
		}
	}
}
