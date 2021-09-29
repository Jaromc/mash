//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_FILE_STREAM_H_
#define _C_MASH_FILE_STREAM_H_

#include "MashFileStream.h"
#include "MashString.h"

namespace mash
{
	class MashFileManager;
	class CMashFileStream : public MashFileStream
	{
	private:
		void *m_pData;
		uint32 m_iCurrentDataSizeInBytes;
		uint32 m_iReservedSizeInBytes;
		MashFileManager *m_pFileManager;

		void ResizeArray(uint32 iNewSize);
	public:
		CMashFileStream(MashFileManager *pFileManager);
		~CMashFileStream();

		eMASH_STATUS LoadFile(const int8 *sFileName, eFILE_IO_MODE mode);
		eMASH_STATUS SaveFile(const int8 *sFileName, eFILE_IO_MODE mode);
		void Destroy();

		void AppendStringToStream(const int8 *s);
		void AppendToStream(const void *pData, uint32 iSizeInBytes);

		const void* GetData()const;
		uint32 GetDataSizeInBytes()const;

		void ClearStream();
	};

	inline const void* CMashFileStream::GetData()const
	{
		return m_pData;
	}

	inline uint32 CMashFileStream::GetDataSizeInBytes()const
	{
		return m_iCurrentDataSizeInBytes;
	}
}

#endif