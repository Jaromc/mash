//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LOG_H_
#define _MASH_LOG_H_

#include "MashMemoryTypes.h"
#include "MashArray.h"
#include "MashString.h"
#include "MashEventTypes.h"
#include <stdio.h>

namespace mash
{
    /*!
        Static logging class.
     
        An event receiver can be set for custom logging using AddReceiver.
    */
	class _MASH_EXPORT MashLog
	{
	public:
		/*
			Simple allocation class that doesn't rely on memory management so we don't get
			false positives when logging memory leaks.
		*/
		class LogMemoryPool
		{
		public:
			LogMemoryPool(){}
			LogMemoryPool(const LogMemoryPool &c){}
			~LogMemoryPool(){}

			void* GetMemory(uint32 sizeInBytes, size_t alignment = mash::_g_globalMemoryAlignment)
			{
				void *tempMem = malloc(sizeInBytes);
				if (!tempMem)
				{
					assert(0);
				}

				return tempMem;
			}

			void FreeMemory(void *ptr)
			{
				free(ptr);
			}

			//! Does nothing.
			void Clear(){}
			void Destroy(){}
		};
	public:
		enum eERROR_LEVEL
		{
			aERROR_LEVEL_ERROR = 1,
			aERROR_LEVEL_WARNING = 2,
			aERROR_LEVEL_INFORMATION = 4,
			aERROR_LEVEL_USER = 8,
			aERROR_LEVEL_ALL = 0xFFFFFFFF
		};

		struct sReceiver
		{
			MashLogEventFunctor callback;
			uint32 id;

			sReceiver(MashLogEventFunctor &c, uint32 i):callback(c), id(i){}
			sReceiver():id(0){}
		};
	private:
		MashLog();
		~MashLog();
		
		FILE *m_log;
		uint32 m_errorLevelFlags;
		static MashLog *m_instance;

		MashArray<sReceiver, LogMemoryPool> m_receivers;
		uint32 m_receiverID;
        bool m_suppressMessages;

		/*!
			Creates a new log.
			\param fileName Name of the log file to create.
		*/
		void CreateLog();

		/*!
			Closes the current log if it is open.
		*/
		void CloseLog();
	public:
		static MashLog* Instance();
		static void DestroyInstance();

        //! Sets the errors that will be logged.
        /*!
            Flags use bitwise MashLog::eERROR_LEVEL.
            
            \param flags Bitwise MashLog::eERROR_LEVEL to log.
        */
		void SetErrorLevelFlag(uint32 flags);
        
        //! Gets the errors that will be logged.
        /*!
            \return Bitwise MashLog::eERROR_LEVEL.
        */
		uint32 GetErrorLevelFlags()const;

        //! Writes a new messge to the current log file.
		/*!
            \param Error level of the message.
			\param msg Messge to log.
            \param functionName Function the message came from.
		*/
		void WriteToLog(eERROR_LEVEL level, const int8 *msg, const int8 *functionName);
        
        //! Writes a new messge to the current log file.
		/*!
            Functions like sprintf.
         
            \param Error level of the message.
            \param functionName Function the message came from.
            \param msg Messge to log. This can include printf identifiers such as %d % s
            \param Extra data to fill any %s %d etc used.
         */
		void WriteToLogEx(eERROR_LEVEL level, const int8 *functionName, const int8 *msg, ...);

        //! Adds a reciever to that will be notified of events.
        /*!
			Be sure to call RemoveReceiver() before your callback is destroyed.

            \param callback Event receiver.
            \return Receiver index that can be used to remove it.
        */
		uint32 AddReceiver(MashLogEventFunctor callback);
        
        //! Removes a receiver from this log.
        /*!
            \param receiverId Receiver id.
        */
		void RemoveReceiver(uint32 receiverId);
        
        //! Stops messages being sent to file.
        /*!
            \param val True to suppress messages or false to reenable.
        */
        void SuppressMessages(bool val);

		void WriteBoundsError(int32 value, int32 minVal, int32 maxVal, int8 *valName, int8* functionName);
	};

/*!
    Macros can be used to compile out logging.
*/
#ifdef MASH_LOG_ENABLED
#define MASH_WRITE_TO_LOG(errorLevel, message, functionName)(mash::MashLog::Instance()->WriteToLog(errorLevel, message, functionName))
#define MASH_WRITE_TO_LOG_EX(errorLevel, functionName, message, ...)(mash::MashLog::Instance()->WriteToLogEx(errorLevel, functionName, message, __VA_ARGS__))

#define MASH_LOG_BOUNDS_ERROR(value, minVal, maxVal, valName, functionName) mash::MashLog::Instance()->WriteBoundsError(value, minVal, maxVal, valName, functionName);
#else
#define MASH_WRITE_TO_LOG(errorLevel, message, functionName)
#define MASH_WRITE_TO_LOG_EX(errorLevel, functionName, message, ...)
#define MASH_LOG_BOUNDS_ERROR(value, minVal, maxVal, valName, functionName)
#endif
}

#endif