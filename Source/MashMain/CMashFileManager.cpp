//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashFileManager.h"
#include "CMashXMLWriter.h"
#include "CMashXMLReader.h"
#include "CMashFileStream.h"
#include "MashLog.h"
#include "CMashARCWriter.h"
#include "MashMathHelper.h"
#include "MashString.h"
#include <cstring>
#include "MashHelper.h"
#include "MashStringHelper.h"
#ifdef MASH_WINDOWS
#include "windows.h"
#include <direct.h>
#include <errno.h>
#elif defined (MASH_APPLE)
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <stdio.h>
#include <dirent.h>

#elif defined (MASH_LINUX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <stdio.h>
#include <dirent.h>
#endif

namespace mash
{
	CMashFileManager::CMashFileManager():MashFileManager(),m_iVirtualFileSystemMemorySize()
	{
	}

	CMashFileManager::~CMashFileManager()
	{
		std::map<MashStringc, sFileData>::const_iterator iter = m_fileData.begin();
		std::map<MashStringc, sFileData>::const_iterator end = m_fileData.end();
		for(; iter != end; ++iter)
		{
			if (iter->second.pData)
			{
				MASH_FREE(iter->second.pData);
			}
		}

		m_fileData.clear();
	}

	void CMashFileManager::GetExtension(const int8 *sFileName, MashStringc &out)const
	{
		out = "";
		const uint32 iFilePathLen = math::Max<int32>(0, strlen(sFileName) - 1);
		for(uint32 i = iFilePathLen; i >= 0; --i)
		{
			if (sFileName[i] == '.')
				break;
			else
				out.Insert(out.Begin(), sFileName[i]);
		}
	}

	eMASH_STATUS CMashFileManager::LoadArchive(const int8 *sFileName, bool &archiveFound)
	{
		MashArray<MashFileManager::sFileAttributes> fileAttribs;
		APIGetDirectoryStructure(sFileName, fileAttribs);

		MashStringc sCurrentDirectory;
		APIGetCurrentDirectory(sCurrentDirectory);
		MashStringc sFileExtension;
		const uint32 iFileCount = fileAttribs.Size();
		for(uint32 i = 0; i < iFileCount; ++i)
		{
			if (!(fileAttribs[i].flags & aFILE_ATTRIB_DIR) && !!(fileAttribs[i].flags & aFILE_ATTRIB_PARENT_DIR))
			{
				GetExtension(fileAttribs[i].relativeFilePath.GetCString(), sFileExtension);

				if (strcmp(g_aeroARCFileExtention, sFileExtension.GetCString()) == 0)
				{
					archiveFound = true;

					MashFileStream *pFileStream = CreateFileStream();
					if (pFileStream->LoadFile(fileAttribs[i].absoluteFilePath.GetCString(), aFILE_IO_BINARY) == aMASH_FAILED)
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to open archive.", "CMashFileManager::LoadArchiveStructure()");
						return aMASH_FAILED;
					}

					sARCFileHeader newArchiveHeader;
					memcpy(&newArchiveHeader, pFileStream->GetData(), sizeof(sARCFileHeader));

					if (newArchiveHeader.directoryFileCount > 0)
					{
						sARCFileInfo *pFileListData = MASH_ALLOC_T_COMMON(sARCFileInfo, newArchiveHeader.directoryFileCount);
						memcpy(pFileListData, &((const uint8*)pFileStream->GetData())[sizeof(sARCFileHeader)], sizeof(sARCFileInfo) * newArchiveHeader.directoryFileCount);

						m_arcFileHeaders.PushBack(newArchiveHeader);
						const uint32 iFileHeaderIndex = m_arcFileHeaders.Size();
						for(uint32 iFile = 0; iFile < newArchiveHeader.directoryFileCount; ++iFile)
						{
							
							sFileData newVirtualFile;
							newVirtualFile.pData = 0;
							newVirtualFile.iTotalSizeInBytes = pFileListData[iFile].totalDataSizeInBytes;
							newVirtualFile._ARCFileLocation = pFileListData[iFile].fileLocation;
							newVirtualFile._ARCPartialDataSizeInBytes = pFileListData[iFile].partialDataSizeInBytes;
							newVirtualFile._ARCFileHeaderIndex = iFileHeaderIndex;

							/*
								Its possible for multiple archives to have the same filename. This is due to
								the VFS support of splitting files across archives. We only want to store the
								start of the data. In theroy, the first instance of a named file should be the first 
								part of the file data.
							*/
							std::map<MashStringc, sFileData>::iterator iter = m_fileData.find(pFileListData[iFile].fileDirectory);
							if (iter == m_fileData.end())
								m_fileData[pFileListData[iFile].fileDirectory] = newVirtualFile;
						}

						MASH_FREE(pFileListData);
					}
				}
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashFileManager::Initialise()
	{
		/*
			There must at least be one root path
		*/
		m_rootDirectories.PushBack("");
        m_rootDirectories.PushBack("/");
		
		return aMASH_OK;
	}

	void CMashFileManager::AddRootPath(const int8 *path)
	{
		if (!path)
			return;

		//check for duplicates
		const uint32 iPathCount = m_rootDirectories.Size();
		for(uint32 i = 0; i < iPathCount; ++i)
		{
			if (strcmp(m_rootDirectories[i].GetCString(), path) == 0)
				return;
		}

		m_rootDirectories.PushBack(path);
	}

	MashXMLWriter* CMashFileManager::CreateXMLWriter(const int8 *sFileName, const int8 *sRootNodeName, bool append)
	{
		CMashXMLWriter *pNewWriter = MASH_NEW_COMMON CMashXMLWriter(this);
		if (!pNewWriter->LoadFile(sFileName, sRootNodeName, append))
		{
			MASH_DELETE pNewWriter;
			pNewWriter = 0;
		}

		return pNewWriter;
	}

	MashXMLReader* CMashFileManager::CreateXMLReader(const int8 *sFileName)
	{
		CMashXMLReader *pNewReader = MASH_NEW_COMMON CMashXMLReader(this);
		if (!pNewReader->LoadFile(sFileName))
		{
			MASH_DELETE pNewReader;
			pNewReader = 0;
		}

		return pNewReader;
	}

	eMASH_STATUS CMashFileManager::CreateArchive(const sMashArchiveCreationInfo &creationInfo)
	{
		CMashARCWriter *pWriter = MASH_NEW_COMMON CMashARCWriter(this);

		eMASH_STATUS status = pWriter->BuildArchive(creationInfo);

		MASH_DELETE pWriter;

		return status;
	}

	eMASH_STATUS CMashFileManager::AddStringToVirtualFileSystem(const int8 *sFileName, const int8 *string)
	{
		uint32 stringSize = 0;
		if (string)
			stringSize = strlen(string) + 1;

		return AddFileToVirtualFileSystem(sFileName, string, stringSize);
	}

	eMASH_STATUS CMashFileManager::AddFileToVirtualFileSystem(const int8 *sFileName, const void *pData, uint32 iDataSizeInBytes)
	{
		std::map<MashStringc, sFileData>::iterator iter = m_fileData.find(sFileName);
		if (iter != m_fileData.end())
		{
			/*
				Delete the buffer If the required data size is greater than whats already
				allocated, or, the buffer has been set to zero
			*/
			if ((iter->second.iTotalSizeInBytes < iDataSizeInBytes) ||
				(iDataSizeInBytes == 0) || 
				!pData)
			{
				MASH_FREE(iter->second.pData);
				iter->second.pData = 0;

				iter->second.iTotalSizeInBytes = 0;

				m_iVirtualFileSystemMemorySize -= iter->second.iTotalSizeInBytes;
			}

			if ((iDataSizeInBytes > 0) && pData)
			{
				if (iter->second.iTotalSizeInBytes < iDataSizeInBytes)
				{
					//deleteion of old buffer is taken care of above
					iter->second.iTotalSizeInBytes = iDataSizeInBytes;
					iter->second.pData = MASH_ALLOC_COMMON(iDataSizeInBytes);

					m_iVirtualFileSystemMemorySize += iDataSizeInBytes;
				}

				memcpy(iter->second.pData, pData, iDataSizeInBytes);
			}
			else
			{
				//delete entry
				m_fileData.erase(iter);
			}
		}
		else
		{
			sFileData newData;
			newData.iTotalSizeInBytes = iDataSizeInBytes;
			newData.pData = 0;
			if ((iDataSizeInBytes > 0) && pData)
			{
				newData.pData = MASH_ALLOC_COMMON(iDataSizeInBytes);
				memcpy(newData.pData, pData, iDataSizeInBytes);
			}

			m_fileData[sFileName] = newData;

			m_iVirtualFileSystemMemorySize += iDataSizeInBytes;
		}

		return aMASH_OK;
	}

	CMashFileManager::sFileData* CMashFileManager::_GetFileDataFromVirtualFileSystem(const int8 *sFileName)const
	{
		std::map<MashStringc, sFileData>::const_iterator iter = m_fileData.find(sFileName);
		if (iter != m_fileData.end())
		{
			return (CMashFileManager::sFileData*)&iter->second;
		}

		return 0;
	}

	MashFileStream* CMashFileManager::CreateFileStream()
	{
		MashFileStream *pFileStream = MASH_NEW_COMMON CMashFileStream(this);
		return pFileStream;
	}

	eMASH_STATUS CMashFileManager::_ReadArchive(const sFileData *pFileData, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes)
	{
		uint8 *pTempFileData = 0;
		uint32 iTempFileSize = 0;

		*pOutData = MASH_ALLOC_COMMON(pFileData->iTotalSizeInBytes);
		uint8 *pCurrentPointerPos = (uint8*)(*pOutData);
		int32 iBytesRemaining = pFileData->iTotalSizeInBytes;

		uint32 iHeaderIndex = pFileData->_ARCFileHeaderIndex;
		const uint32 iHeaderCount = m_arcFileHeaders.Size();
		while(iBytesRemaining > 0)
		{
			/*
				The archive system supports splitting a file across multiple archives.
				Therefore we may need to open multiple archives to rebuild
				the original data.
			*/
			if (_APIReadFile(m_arcFileHeaders[iHeaderIndex].currentFileDir, mode, (void**)&pTempFileData, iTempFileSize) == aMASH_FAILED)
			{
				MASH_FREE(pOutData);
				return aMASH_FAILED;
			}

			memcpy(pCurrentPointerPos, &pTempFileData[pFileData->_ARCFileLocation], pFileData->_ARCPartialDataSizeInBytes);
			pCurrentPointerPos += pFileData->_ARCPartialDataSizeInBytes;
			iBytesRemaining -= pFileData->_ARCPartialDataSizeInBytes;

			MASH_FREE(pTempFileData);
			iTempFileSize = 0;

			//find the next header if needed
			if (iBytesRemaining > 0)
			{
				for(uint32 i = 0; i < iHeaderCount; ++i)
				{
					if (strcmp(m_arcFileHeaders[i].currentFileDir, m_arcFileHeaders[iHeaderIndex].nextFileDir) == 0)
					{
						iHeaderIndex = i;
						break;
					}
				}
			}
		}

		return aMASH_OK;
	}
    
    void CMashFileManager::GetAbsoutePath(const int8 *fileName, MashStringc &absPath)const
    {
        absPath = "";
#if defined (MASH_APPLE) || defined (MASH_LINUX)
        MashStringc a1 = fileName;
        
        int8* p = 0;
        int8 fpath[4096];
        fpath[0] = 0;
        p = realpath(fileName, fpath);
        
        MashStringc a2 = fpath;
        
        if (p)
        {
            absPath = p;
        }
        else
        {
            if (fpath[0] != 0)
                absPath = fpath;
            else
                absPath = fileName;
        }
#elif defined (MASH_WINDOWS)
	int8* p = 0;
    int8 fpath[4096];
	p = _fullpath(fpath, fileName, sizeof(fpath));
	absPath = p;
	absPath.Replace('\\', '/');
#endif
    }

	bool CMashFileManager::DoesFileExist(const int8 *sFileName)const
	{
		const uint32 iRootPaths = m_rootDirectories.Size();
		MashStringc sNewFileName;
		for(uint32 i = 0; i < iRootPaths; ++i)
		{
			sNewFileName = m_rootDirectories[i];
			sNewFileName += sFileName;

			if (_GetFileDataFromVirtualFileSystem(sNewFileName.GetCString()))
				return true;
			else if (_APIDoesFileExist(sNewFileName.GetCString()))
				return true;
		}

		return false;
	}

	eMASH_STATUS CMashFileManager::ReadFile(const int8 *sFileName, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes)
	{
		MashStringc sNewFileName = sFileName;
		*pOutData = 0;
		iOutDataSizeInBytes = 0;
		const uint32 iRootPaths = m_rootDirectories.Size();

		uint32 iRoot = 0;
		while(!(*pOutData) && (iRoot < iRootPaths))
		{
            ConcatenatePaths(m_rootDirectories[iRoot++].GetCString(), sFileName, sNewFileName);
            
			_ReadFile(sNewFileName.GetCString(), mode, pOutData, iOutDataSizeInBytes);
		}

		if (!*pOutData)
		{
			int8 buffer[256];
			mash::helpers::PrintToBuffer(buffer, 256, "Failed to open file '%s', File not found.", sFileName);
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashFileManager::ReadFile");
			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashFileManager::WriteFile(const int8 *sFileName, eFILE_IO_MODE mode, void *pData, uint32 iDataSizeInBytes)
	{
		MashStringc sNewFileName = sFileName;

		eMASH_STATUS status = aMASH_OK;
        status = _WriteFile(sNewFileName.GetCString(), mode, pData, iDataSizeInBytes);

		if (status == aMASH_FAILED)
		{
			int8 buffer[256];
			mash::helpers::PrintToBuffer(buffer, 256, "Failed writing to file '%s', File not found.", sFileName);
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashFileManager::WriteFile");
		}

		return status;
	}

	eMASH_STATUS CMashFileManager::_ReadFile(const int8 *sFileName, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes)
	{
		iOutDataSizeInBytes = 0;

		//first look in the virtual file system
		const sFileData *pData = _GetFileDataFromVirtualFileSystem(sFileName);
		if (pData && pData->pData)
		{
			iOutDataSizeInBytes = pData->iTotalSizeInBytes;
			*pOutData = MASH_ALLOC_COMMON(iOutDataSizeInBytes);
			memcpy(*pOutData, pData->pData, iOutDataSizeInBytes);
			return aMASH_OK;
		}
		//now look in archive files
		/*
			If the VF has no data then it should be stored in an archive
		*/
		else if (pData)
		{
			/*
				A file was found previously in the VFS but the
				data is contained in an archive, so we need
				to retrieve it.
			*/
            return _ReadArchive(pData, mode, pOutData, iOutDataSizeInBytes);
		}

		//finaly check the api
		return _APIReadFile(sFileName, mode, pOutData, iOutDataSizeInBytes);
	}

	eMASH_STATUS CMashFileManager::_WriteFile(const int8 *sFileName, eFILE_IO_MODE mode, void *pData, uint32 iDataSizeInBytes)
	{
		return _APIWriteFile(sFileName, mode, pData, iDataSizeInBytes);
	}
    
    eMASH_STATUS CMashFileManager::_APIReadFile(const int8 *sFileName, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes)
	{
        MashStringc absPath;
        GetAbsoutePath(sFileName, absPath);
        
		FILE *pFile = 0;
        pFile = fopen(absPath.GetCString(), "rb");
        
		if (pFile == 0)
		{
			return aMASH_FAILED;
		}
        
		fseek(pFile, 0, SEEK_END);
		long iSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
        
        if (iSize == 0)
        {
            *pOutData = 0;
            iOutDataSizeInBytes = 0;
            return aMASH_OK;
        }
        if (mode == aFILE_IO_TEXT)
            iSize += 1; //for EOF
        
		int8 *pData = (int8*)MASH_ALLOC_COMMON((sizeof(int8) * iSize));
		memset(pData, 0, iSize);
		fread(pData, 1, iSize-1, pFile);
        
        if (mode == aFILE_IO_TEXT)
            pData[iSize-1] = '\0';
        
		fclose(pFile);
        
#if defined (MASH_APPLE) || defined (MASH_LINUX)
        bool normalizeLineEndings = true;
        if ((mode == aFILE_IO_TEXT) && normalizeLineEndings)
        {
            MashStringc stringToScan = (const int8*)pData;
            size_t found;
            size_t lastFound = 0;
            do {
                found = stringToScan.Find("\r\n", lastFound);
                if (found != MashStringc::npos)
                {
                    stringToScan.Replace(found, 2, "\n");
                    lastFound = found;
                }
            } while (found != MashStringc::npos);
            
            MASH_FREE(pData);
            iSize = stringToScan.Size() + 1;
            pData = (int8*)MASH_ALLOC_COMMON(iSize);
            memset(pData, 0, iSize);
            strncpy((int8*)pData, stringToScan.GetCString(), iSize-1);
            
        }
#endif
        
		iOutDataSizeInBytes = iSize;
		*pOutData = pData;
        
		return aMASH_OK;
	}
    
	eMASH_STATUS CMashFileManager::_APIWriteFile(const int8 *sFileName, eFILE_IO_MODE mode, void *pData, uint32 iDataSizeInBytes)
	{
		FILE *pFile = 0;
        
        pFile = fopen(sFileName, "wb");
        
		if (pFile == 0)
			return aMASH_FAILED;
        
		if (pData)
			fwrite(pData, 1, iDataSizeInBytes, pFile);
        
		fclose(pFile);
        
		return aMASH_OK;
	}
    
    bool CMashFileManager::_APIDoesFileExist(const int8 *sFileName)const
	{
#ifdef MASH_WINDOWS
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(sFileName, &findFileData);
        
		if (hFind != INVALID_HANDLE_VALUE)
			return true;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
        return (access(sFileName, 0) == -1 ? 0 : 1);
#endif
        
		return false;
	}
    
	bool CMashFileManager::APICreateDirectory(const int8 *sDir)
	{
#ifdef MASH_WINDOWS
		int32 iValue = _mkdir(sDir);
		if ((iValue != 0) && (errno == ENOENT))
			return false;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
        if (mkdir(sDir, S_IRWXU) == 0)
            return true;
        else
            return false;
#endif
        
		return true;
	}
    
	bool CMashFileManager::APISetWorkingDirectory(const int8 *dir)
	{
#ifdef MASH_WINDOWS
		return ((_chdir(dir) == 0) ? 1 : 0);
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
        return ((chdir(dir) == 0) ? 1 : 0);
#endif
	}
    
	void CMashFileManager::APIGetCurrentDirectory(MashStringc &out)
	{
#ifdef MASH_WINDOWS
		int8 buffer[512];
		memset(buffer, 0, 512);
		::GetCurrentDirectory(512, buffer);
		out = buffer;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
        int8 buffer[512];
		memset(buffer, 0, 512);
        getcwd(buffer, 512);
        out = buffer;
#endif
	}
    
	void CMashFileManager::APIGetDirectoryStructure(const int8 *sDirectory, MashArray<sFileAttributes> &out)
	{
#ifdef MASH_WINDOWS
		/*
         TODO : Make these more solid. They will experience problems with unicode to asci
         */
		if (sDirectory)
		{	
			MashStringc sAmendedDir;
            
			mash::ConcatenatePaths(sDirectory, "*", sAmendedDir);
            
			WIN32_FIND_DATA findFileData;
			HANDLE hFind = FindFirstFile(sAmendedDir.GetCString(), &findFileData);
            
			if (hFind != INVALID_HANDLE_VALUE)
			{
				MashStringc sCurrentDirectory = "";
				//store current windows state
				APIGetCurrentDirectory(sCurrentDirectory);
                
				//go into the windows folder to get the correct abs path
				_chdir(sDirectory);
                
				int8 buffer[512];
				do
				{
					//remove this dir from list
					if (strcmp(".", findFileData.cFileName ) != 0)
					{
						GetFullPathName(findFileData.cFileName, 512, buffer, 0);
						
						sFileAttributes newFile;
						newFile.relativeFilePath = findFileData.cFileName;
						newFile.absoluteFilePath = buffer;
						newFile.flags = 0;
                        
						if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							newFile.flags |= aFILE_ATTRIB_DIR;
						if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
							newFile.flags |= aFILE_ATTRIB_READONLY;
						if (strcmp("..", newFile.relativeFilePath.GetCString()) == 0)
							newFile.flags |= aFILE_ATTRIB_PARENT_DIR;
                        
						out.PushBack(newFile);
					}
					
				}while(FindNextFile(hFind, &findFileData) != 0);
                
				//set current windows dir back to its original state
				_chdir(sCurrentDirectory.GetCString());
			}
		}
#else
        DIR *dp;
        struct dirent *ep;     
        dp = opendir (sDirectory);
        
        if (dp != NULL)
        {
            MashStringc originalDir;
            APIGetCurrentDirectory(originalDir);
            
            APISetWorkingDirectory(sDirectory);
            while ((ep = readdir (dp)))
            {
                //remove this dir from list
                if (strcmp(".", ep->d_name) != 0)
                {
                    sFileAttributes newFile;
                    newFile.relativeFilePath = ep->d_name;
                    newFile.absoluteFilePath = ep->d_name;
                    
                    if (strcmp("..", ep->d_name) != 0)
                    {
                        int8 actualpath [PATH_MAX+1];
                        int8 *ptr = realpath(ep->d_name, actualpath);
                        if (ptr)
                            newFile.absoluteFilePath = ptr;
                    }
                    
                    newFile.flags = 0;
                    if (ep->d_type == DT_DIR)  
                        newFile.flags |= aFILE_ATTRIB_DIR;
                    if (strcmp("..", ep->d_name) == 0)
                        newFile.flags |= aFILE_ATTRIB_PARENT_DIR;
                    
                    out.PushBack(newFile);
                }
            }
            
            APISetWorkingDirectory(originalDir.GetCString());
            
            (void) closedir (dp);
        }  
#endif
	}
}
