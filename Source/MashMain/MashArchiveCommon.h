//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ARCHIVE_COMMON_H_
#define _MASH_ARCHIVE_COMMON_H_

#include "MashArray.h"
#include "MashString.h"

namespace mash
{
	class MashARCExtensionHandler
	{
	public:
		MashARCExtensionHandler(){}
		virtual ~MashARCExtensionHandler(){}

		virtual const int8* GetFileExtension()const = 0;
		virtual void GetFileContents(const int8 *sFileName, void **pDataOut, uint32 &iSizeInBytesOut) = 0;
		virtual void ReadFileContents(const int8 *sData, uint32 iSizeInBytes) = 0;
	};

	static const uint32 g_aeroARCCharBufferSize = 100;
	static const int8 *const g_aeroARCFileExtention = "arc";
	static const int8 *const g_aeroARCFileName = "arc_";
	static const int8 *const g_aeroARCRootDir = "./arc";

	struct sMashArchiveCreationInfo
	{
		MashArray<MashARCExtensionHandler*> extensionHandlers;
		MashArray<MashStringc> extensionExlusionList;
		MashArray<MashStringc> includePaths;
		MashStringc archiveRootPath;
		MashStringc offlineSaveLocation;
		uint32 maxFileSizeInBytes;

		sMashArchiveCreationInfo():archiveRootPath(g_aeroARCRootDir),
			offlineSaveLocation(g_aeroARCRootDir),
			maxFileSizeInBytes(0)
		{
			extensionExlusionList.PushBack("exe");
			extensionExlusionList.PushBack("lib");
			extensionExlusionList.PushBack("dll");
		}
	};

	struct sARCFileInfo
	{
		int8 fileDirectory[g_aeroARCCharBufferSize];
		uint32 fileLocation;
		uint32 partialDataSizeInBytes;
		uint32 totalDataSizeInBytes;

		sARCFileInfo()
		{
			memset(fileDirectory, 0, g_aeroARCCharBufferSize);
			fileLocation = 0;
			partialDataSizeInBytes = 0;
			totalDataSizeInBytes = 0;
		}
	};

	struct sARCFileHeader
	{
		int8 fileName[g_aeroARCCharBufferSize];
		int8 currentFileDir[g_aeroARCCharBufferSize];
		int8 nextFileDir[g_aeroARCCharBufferSize];
		uint32 directoryFileCount;
		uint32 dataSizeInBytes;

		sARCFileHeader()
		{
			memset(fileName, 0, g_aeroARCCharBufferSize);
			memset(currentFileDir, 0, g_aeroARCCharBufferSize);
			memset(nextFileDir, 0, g_aeroARCCharBufferSize);
			directoryFileCount = 0;
			dataSizeInBytes = 0;
		}
	};
		

	struct sMashARCDirectoryInfo
	{
		sARCFileHeader archiveHeader;
		sARCFileInfo *fileList;
		uint8 *data;

		sMashARCDirectoryInfo():fileList(0), data(0)
		{
		}
	};
}

#endif