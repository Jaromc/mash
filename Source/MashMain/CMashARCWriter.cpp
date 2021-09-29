//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashARCWriter.h"
#include "MashLog.h"
#include "MashMathHelper.h"
#include "MashStringHelper.h"
#include "MashHelper.h"
#include "MashFileStream.h"
#include <algorithm>
#include "MashCompileSettings.h"
namespace mash
{
	CMashARCWriter::CMashARCWriter(MashFileManager *pFileManager):MashReferenceCounter(),
		m_pFileManager(pFileManager)
	{

	}

	CMashARCWriter::~CMashARCWriter()
	{

	}

	void CMashARCWriter::GetExtension(const int8 *sFileName, MashStringc &out)const
	{
		out.Clear();
		const uint32 iFilePathLen = math::Clamp<int32>(0, 1, strlen(sFileName) - 1);
		for(uint32 i = iFilePathLen; i >= 0; --i)
		{
			if (sFileName[i] == '.')
				break;
			else
				out.Insert(out.Begin(), sFileName[i]);
		}
	}

	uint8* CMashARCWriter::ResizeArray(uint8 *pArray, uint32 iCurrentArraySize, uint32 iNewSize)
	{
		uint8 *pNewArray = (uint8*)MASH_ALLOC_COMMON(iNewSize);
		memset(pNewArray, 0, iNewSize);

		if (pArray)
		{
			memcpy(pNewArray, pArray, iCurrentArraySize);
			MASH_FREE(pArray);
		}

		return pNewArray;
	}

	void CMashARCWriter::SaveFileData(const MashFileManager::sFileAttributes &fileAttributes,
		sMashARCDirectoryInfo &archiveInfo,
		uint32 &iReservedDataSize,
		uint32 &iReservedFileInfoSize,
		uint32 iPartialDataSizeInBytes,
		uint32 iTotalDataSizeInBytes,
		uint8 *pFileData)
	{
		const uint32 iFileLocation = archiveInfo.archiveHeader.directoryFileCount * sizeof(sARCFileInfo);

		sARCFileInfo newFile;
		strncpy(newFile.fileDirectory, fileAttributes.relativeFilePath.GetCString(), g_aeroARCCharBufferSize);
		newFile.fileLocation = iFileLocation;
		newFile.partialDataSizeInBytes = iPartialDataSizeInBytes;
		newFile.totalDataSizeInBytes = iTotalDataSizeInBytes;

		const uint32 iFileSizeNeeded = sizeof(sARCFileInfo) + (archiveInfo.archiveHeader.directoryFileCount * sizeof(sARCFileInfo));
		if (iReservedFileInfoSize <= iFileSizeNeeded)
		{
			archiveInfo.fileList = (sARCFileInfo*)ResizeArray((uint8*)archiveInfo.fileList, iReservedFileInfoSize, iFileSizeNeeded * 2);
			iReservedFileInfoSize = iFileSizeNeeded * 2;
		}
		
		memcpy(&archiveInfo.fileList[archiveInfo.archiveHeader.directoryFileCount], &newFile, sizeof(sARCFileInfo));
		++archiveInfo.archiveHeader.directoryFileCount;

		const uint32 iDataSizeNeeded = iPartialDataSizeInBytes + archiveInfo.archiveHeader.dataSizeInBytes;
		if (iReservedDataSize <= iDataSizeNeeded)
		{
			archiveInfo.data = ResizeArray(archiveInfo.data, iReservedDataSize, iDataSizeNeeded * 2);
			iReservedDataSize = iDataSizeNeeded * 2;
		}

		memcpy(&archiveInfo.data[archiveInfo.archiveHeader.dataSizeInBytes], pFileData, iPartialDataSizeInBytes);
		archiveInfo.archiveHeader.dataSizeInBytes += iPartialDataSizeInBytes;
	}

	eMASH_STATUS CMashARCWriter::_BuildArchive(const int8 *sFileName,
		const MashArray<MashARCExtensionHandler*> &extensionHandlers,
		const MashArray<MashStringc> &extensionExlusionList,
		MashArray<sMashARCDirectoryInfo> &files,
		uint32 &iReservedDataSize,
		uint32 &iReservedFileInfoSize,
		uint32 iMaxFileSizeInBytes)
	{
		MashArray<MashFileManager::sFileAttributes> fileAttribs;
		m_pFileManager->APIGetDirectoryStructure(sFileName, fileAttribs);

		MashStringc sCurrentDir;
		m_pFileManager->APIGetCurrentDirectory(sCurrentDir);

		const uint32 iFileCount = fileAttribs.Size();
		const uint32 iExtHandlerCount = extensionHandlers.Size();
		const uint32 iExclusionListCount = extensionExlusionList.Size();
		MashStringc sFileExtension;
		uint32 iFileDataSizeInBytes = 0;
		uint8 *pFileData = 0;
		for(uint32 i = 0; i < iFileCount; ++i)
		{
			//dont recurse into parent directory
			if (fileAttribs[i].flags & MashFileManager::aFILE_ATTRIB_PARENT_DIR)
				continue;

			if (fileAttribs[i].flags & MashFileManager::aFILE_ATTRIB_DIR)
			{
				if (_BuildArchive(fileAttribs[i].absoluteFilePath.GetCString(), extensionHandlers, extensionExlusionList, files, iReservedDataSize, iReservedFileInfoSize, iMaxFileSizeInBytes) == aMASH_FAILED)
					return aMASH_FAILED;
			}
			else
			{
				bool bHandled = false;
				GetExtension(fileAttribs[i].absoluteFilePath.GetCString(), sFileExtension);

				for(uint32 exclusion = 0; exclusion < iExclusionListCount; ++exclusion)
				{
					if (strcmp(extensionExlusionList[exclusion].GetCString(), sFileExtension.GetCString()) == 0)
					{
						bHandled = true;
						break;
					}
				}

				if (!bHandled)
				{
					for(uint32 ext = 0; ext < iExtHandlerCount; ++ext)
					{
						if (strcmp(extensionHandlers[ext]->GetFileExtension(), sFileExtension.GetCString()) == 0)
						{
							extensionHandlers[ext]->GetFileContents(fileAttribs[i].relativeFilePath.GetCString(), (void**)&pFileData, iFileDataSizeInBytes);
							bHandled = true;
							break;
						}
					}
				}

				if (!bHandled)
				{
					MashFileStream *pReadFileStream = m_pFileManager->CreateFileStream();
					if (pReadFileStream->LoadFile(fileAttribs[i].absoluteFilePath.GetCString(), aFILE_IO_BINARY) == aMASH_FAILED)
					{
						MashStringc msg = fileAttribs[i].absoluteFilePath;
						msg += " - Could not load file for saving to ARC.";
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, msg.GetCString(), "CMashARCWriter::_BuildArchive");
						return aMASH_FAILED;
					}

					iFileDataSizeInBytes = pReadFileStream->GetDataSizeInBytes();
					pFileData = MASH_ALLOC_T_COMMON(uint8, iFileDataSizeInBytes);

					memcpy(pFileData, pReadFileStream->GetData(), iFileDataSizeInBytes);

					pReadFileStream->Destroy();
				}

				if (pFileData)
				{
					sMashARCDirectoryInfo &archiveInfo = files.Back();

					if ((iMaxFileSizeInBytes == 0) || ((iFileDataSizeInBytes + archiveInfo.archiveHeader.dataSizeInBytes) < iMaxFileSizeInBytes))
					{
						SaveFileData(fileAttribs[i],
							archiveInfo,
							iReservedDataSize,
							iReservedFileInfoSize,
							iFileDataSizeInBytes,
							iFileDataSizeInBytes,
							pFileData);
					}
					else
					{
						uint8 *pPartialLocation = pFileData;
						uint32 iRemainingDataSizeInBytes = iFileDataSizeInBytes;
						while ((iRemainingDataSizeInBytes + archiveInfo.archiveHeader.dataSizeInBytes) > iMaxFileSizeInBytes)
						{
							uint32 iPartialDataSize = (iFileDataSizeInBytes + archiveInfo.archiveHeader.dataSizeInBytes) - iMaxFileSizeInBytes;
							SaveFileData(fileAttribs[i],
								archiveInfo,
								iReservedDataSize,
								iReservedFileInfoSize,
								iPartialDataSize,
								iFileDataSizeInBytes,
								pPartialLocation);

							files.PushBack(sMashARCDirectoryInfo());
							
							archiveInfo = files.Back();

							iRemainingDataSizeInBytes -= iPartialDataSize;
							pPartialLocation += iPartialDataSize;
						}				
					}

					MASH_FREE (pFileData);
					pFileData = 0;
				}
				
				iFileDataSizeInBytes = 0;
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashARCWriter::BuildArchive(const sMashArchiveCreationInfo &creationInfo)
	{
		if (creationInfo.includePaths.Empty())
			return aMASH_OK;

		MashArray<MashStringc> includePaths = creationInfo.includePaths;
		//TODO : Re-impliment this
		MASH_ASSERT(0);//MashArray<MashStringc>::Iterator newEndIter = std::unique(includePaths.begin(), includePaths.end());

		MashArray<sMashARCDirectoryInfo> files;
		files.PushBack(sMashARCDirectoryInfo());

		uint32 iReservedDataSize = 0;
		uint32 iReservedFileInfoSize = 0;

		const uint32 iRootPathCount = includePaths.Size();
		for(uint32 i = 0; i < iRootPathCount; ++i)
		{
			MashStringc t;
			m_pFileManager->APIGetCurrentDirectory(t);
			m_pFileManager->APIGetCurrentDirectory(t);

			_BuildArchive(includePaths[i].GetCString(),
				creationInfo.extensionHandlers, 
				creationInfo.extensionExlusionList,
				files,
				iReservedDataSize,
				iReservedFileInfoSize,
				creationInfo.maxFileSizeInBytes);
		}

		if (files[0].archiveHeader.directoryFileCount > 0)
		{
			m_pFileManager->APICreateDirectory(creationInfo.offlineSaveLocation.GetCString());

			uint32 iFileNumber = 0;

			int8 buffer[256];
			memset(buffer, 0, 256);
			MashStringc sFileName = g_aeroARCFileName;
			sFileName += mash::helpers::NumberToString(buffer, 256, iFileNumber++);
			sFileName += ".";
			sFileName += g_aeroARCFileExtention;

			memset(files[0].archiveHeader.fileName, 0, g_aeroARCCharBufferSize);
			strncpy(files[0].archiveHeader.fileName, sFileName.GetCString(), g_aeroARCCharBufferSize);

			sFileName = creationInfo.archiveRootPath;
			sFileName += "/";
			sFileName += files[0].archiveHeader.fileName;

			memset(files[0].archiveHeader.nextFileDir, 0, g_aeroARCCharBufferSize);
			strncpy(files[0].archiveHeader.currentFileDir, sFileName.GetCString(), g_aeroARCCharBufferSize);

			const uint32 iFileCount = files.Size();
			for(uint32 i = 1; i < iFileCount; ++i)
			{
				memset(buffer, 0, 256);
				sFileName = g_aeroARCFileName;
				sFileName += mash::helpers::NumberToString(buffer, 256, iFileNumber++);
				sFileName += ".";
				sFileName += g_aeroARCFileExtention;

				memset(files[i].archiveHeader.fileName, 0, g_aeroARCCharBufferSize);
				strncpy(files[i].archiveHeader.fileName, sFileName.GetCString(), g_aeroARCCharBufferSize);

				ConcatenatePaths(creationInfo.archiveRootPath.GetCString(), files[i].archiveHeader.fileName, sFileName);

				//fill in the current name
				strncpy(files[i].archiveHeader.currentFileDir, sFileName.GetCString(), g_aeroARCCharBufferSize);
				memset(files[i].archiveHeader.nextFileDir, 0, g_aeroARCCharBufferSize);
				//link this the new file up to the previous
				strncpy(files[i-1].archiveHeader.nextFileDir, files[i].archiveHeader.currentFileDir, g_aeroARCCharBufferSize);
			}

			//write files to disk
			MashFileStream *pWriteFileStream = m_pFileManager->CreateFileStream();
			for(uint32 i = 0; i < iFileCount; ++i)
			{
				//write archive header
				pWriteFileStream->AppendToStream(&files[i].archiveHeader, sizeof(sARCFileHeader));

				//write archive structure
				const uint32 iDirFileCount = files[i].archiveHeader.directoryFileCount;
				pWriteFileStream->AppendToStream(files[i].fileList, iDirFileCount * sizeof(sARCFileInfo));

				//write all file data contained within archive
				pWriteFileStream->AppendToStream(files[i].data, files[i].archiveHeader.dataSizeInBytes);

				

				/*
					If this is being used offline, then the user may want to save the archive
					to a location other than the runtime location. This gives that option.
				*/
				sFileName = "";

				if (creationInfo.offlineSaveLocation == "")
					ConcatenatePaths(g_aeroARCRootDir, files[i].archiveHeader.fileName, sFileName);
				else
					ConcatenatePaths(creationInfo.offlineSaveLocation.GetCString(), files[i].archiveHeader.fileName, sFileName);

				pWriteFileStream->SaveFile(sFileName.GetCString(), aFILE_IO_BINARY);
			}
			
			pWriteFileStream->Destroy();

		}

		files.Clear();

		return aMASH_OK;
	}

}