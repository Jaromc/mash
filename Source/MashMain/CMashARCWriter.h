//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_ARC_WRITER_H_
#define _C_MASH_ARC_WRITER_H_

#include "MashReferenceCounter.h"
#include "MashArchiveCommon.h"
#include "MashFileManager.h"

namespace mash
{
	class CMashARCWriter : public MashReferenceCounter
	{
	private:
		MashFileManager *m_pFileManager;

		eMASH_STATUS _BuildArchive(const int8 *sFileName,
			const MashArray<MashARCExtensionHandler*> &extensionHandlers,
			const MashArray<MashStringc> &extensionExlusionList,
			MashArray<sMashARCDirectoryInfo> &files,
			uint32 &iReservedDataSize,
			uint32 &iReservedFileInfoSize,
			uint32 iMaxFileSizeInBytes);

		void SaveFileData(const MashFileManager::sFileAttributes &fileAttributes,
			sMashARCDirectoryInfo &archiveInfo,
			uint32 &iReservedDataSize,
			uint32 &iReservedFileInfoSize,
			uint32 iPartialDataSizeInBytes,
			uint32 iTotalDataSizeInBytes,
			uint8 *pFileData);

		uint8* ResizeArray(uint8 *pArray, uint32 iCurrentArraySize, uint32 iNewSize);
		void GetExtension(const int8 *sFileName, MashStringc &out)const;
	public:
		CMashARCWriter(MashFileManager *pFileManager);
		~CMashARCWriter();

		eMASH_STATUS BuildArchive(const sMashArchiveCreationInfo &creationInfo);
	};
}

#endif