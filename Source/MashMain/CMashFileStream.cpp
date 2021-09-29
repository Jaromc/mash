//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashFileStream.h"
#include "MashFileManager.h"
#include <cstring>
#include "MashLog.h"

namespace mash
{
	CMashFileStream::CMashFileStream(MashFileManager *pFileManager):MashFileStream(),m_pFileManager(pFileManager),
		m_pData(0), m_iCurrentDataSizeInBytes(0), m_iReservedSizeInBytes(0)
	{
	}

	CMashFileStream::~CMashFileStream()
	{
		if (m_pData)
		{
			MASH_FREE(m_pData);
			m_pData = 0;
		}
	}

	eMASH_STATUS CMashFileStream::LoadFile(const int8 *sFileName, eFILE_IO_MODE mode)
	{
		if (!sFileName)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Invalid file name pointer.", 
					"CMashFileStream::LoadFile");

			return aMASH_FAILED;
		}

		if (m_pData)
			MASH_FREE(m_pData);

		if (m_pFileManager->ReadFile(sFileName, mode, &m_pData, m_iReservedSizeInBytes) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"CMashFileStream::LoadFile",
					"Failed to load file '%s'.", sFileName);

			return aMASH_FAILED;
		}

		m_iCurrentDataSizeInBytes = m_iReservedSizeInBytes;

		return aMASH_OK;
	}

	eMASH_STATUS CMashFileStream::SaveFile(const int8 *sFileName, eFILE_IO_MODE mode)
	{
		if (!sFileName)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Invalid file name pointer.", 
					"CMashFileStream::SaveFile");

			return aMASH_FAILED;
		}

        if (mode == aFILE_IO_TEXT)
        {
            if (m_pData)
            {
                if (((int8*)m_pData)[m_iCurrentDataSizeInBytes] != '\0')
                {
                    AppendToStream("\0", 1);
                }
            }
        }

        if (m_pFileManager->WriteFile(sFileName, mode, m_pData, m_iCurrentDataSizeInBytes) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
					"CMashFileStream::SaveFile",
					"Failed to save file '%s'.", sFileName);

			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	void CMashFileStream::ResizeArray(uint32 iNewSize)
	{
		void *pNewArray = MASH_ALLOC_COMMON(iNewSize);
		memset(pNewArray, 0, iNewSize);

		if (m_pData)
		{
			memcpy(pNewArray, m_pData, m_iReservedSizeInBytes);
			MASH_FREE(m_pData);
		}

		m_pData = pNewArray;
		m_iReservedSizeInBytes = iNewSize;
	}

	void CMashFileStream::ClearStream()
	{
		m_iCurrentDataSizeInBytes = 0;
        memset(m_pData, 0, m_iReservedSizeInBytes);
	}

	void CMashFileStream::AppendStringToStream(const int8 *s)
	{
		if (!s)
			return;

		const uint32 actualStringLength = strlen(s);
		if (actualStringLength == 0)
			return;

		const uint32 stringSize = actualStringLength/* + 1*/;

		uint32 iSizeNeeded = (m_iCurrentDataSizeInBytes + stringSize);

		if (m_iReservedSizeInBytes <= iSizeNeeded)
			ResizeArray(iSizeNeeded * 2);

		memcpy(&((int8*)m_pData)[m_iCurrentDataSizeInBytes], s, sizeof(int8) * stringSize);

		m_iCurrentDataSizeInBytes += stringSize;
	}

	void CMashFileStream::AppendToStream(const void *pData, uint32 iSizeInBytes)
	{
		if (!pData || (iSizeInBytes == 0))
			return;

		uint32 iSizeNeeded = (m_iCurrentDataSizeInBytes + iSizeInBytes);
		if (m_iReservedSizeInBytes <= iSizeNeeded)
			ResizeArray(iSizeNeeded * 2);

		memcpy(&((uint8*)m_pData)[m_iCurrentDataSizeInBytes], pData, iSizeInBytes);
		m_iCurrentDataSizeInBytes += iSizeInBytes;
	}

	void CMashFileStream::Destroy()
	{
		MASH_DELETE this;
	}
}
