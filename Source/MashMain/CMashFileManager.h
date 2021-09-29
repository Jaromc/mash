//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_FILE_MANAGER_H_
#define _C_MASH_FILE_MANAGER_H_

#include "MashDataTypes.h"
#include "MashFileManager.h"
#include "MashLog.h"
#include <map>
#include <set>
#include "MashArchiveCommon.h"
#include "MashString.h"

namespace mash
{
	class CMashFileManager : public MashFileManager
	{
	private:
		struct sFileData
		{
			void *pData;
			uint32 iTotalSizeInBytes;
			
			uint32 _ARCFileLocation;
			uint32 _ARCPartialDataSizeInBytes;
			int32 _ARCFileHeaderIndex;

			sFileData():pData(0), iTotalSizeInBytes(0), _ARCFileLocation(0), 
				_ARCPartialDataSizeInBytes(0), _ARCFileHeaderIndex(-1){}
		};

	private:
		std::map<MashStringc, sFileData> m_fileData;
		MashArray<sARCFileHeader> m_arcFileHeaders;
		MashArray<MashStringc> m_rootDirectories;
		uint32 m_iVirtualFileSystemMemorySize;
		sFileData* _GetFileDataFromVirtualFileSystem(const int8 *sFileName)const;

		eMASH_STATUS LoadArchive(const int8 *sFileName, bool &archiveFound);
		eMASH_STATUS _ReadArchive(const sFileData *pFileData, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes);

		void GetExtension(const int8 *sFileName, MashStringc &out)const;
		eMASH_STATUS _ReadFile(const int8 *sFileName, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes);
		eMASH_STATUS _WriteFile(const int8 *sFileName, eFILE_IO_MODE mode, void *pData, uint32 iDataSizeInBytes);

	protected:
		bool _APIDoesFileExist(const int8 *sFileName)const;
		eMASH_STATUS _APIReadFile(const int8 *sFileName, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes);
        eMASH_STATUS _APIWriteFile(const int8 *sFileName, eFILE_IO_MODE mode, void *pData, uint32 iDataSizeInBytes);
	public:
		CMashFileManager();
		virtual ~CMashFileManager();

		bool APISetWorkingDirectory(const int8 *sDir);
        bool APICreateDirectory(const int8 *sDir);
        void APIGetCurrentDirectory(MashStringc &out);
        void APIGetDirectoryStructure(const int8 *sDirectory, MashArray<sFileAttributes> &out);

		eMASH_STATUS Initialise();

		void AddRootPath(const int8 *path);

		MashXMLWriter* CreateXMLWriter(const int8 *sFileName, const int8 *sRootNodeName, bool append = false);
		MashXMLReader* CreateXMLReader(const int8 *sFileName);

		MashFileStream* CreateFileStream();

		 //! Creates a VFS.
        /*!
            Creats a virtual file system from a current API file structure.
            This archive can then be used to provide untiy across APIs and
            security (somewhat) for your application files.
            
            \param creationInfo Archive creation data.
            \return Status of the function.
        */
		eMASH_STATUS CreateArchive(const sMashArchiveCreationInfo &creationInfo);

		bool DoesFileExist(const int8 *sFileName)const;
        void GetAbsoutePath(const int8 *fileName, MashStringc &absPath)const;

		eMASH_STATUS AddFileToVirtualFileSystem(const int8 *sFileName, const void *pData, uint32 iDataSizeInBytes);
		eMASH_STATUS AddStringToVirtualFileSystem(const int8 *sFileName, const int8 *string);
		const void* GetFileDataFromVirtualFileSystem(const int8 *sFileName, uint32 *iOutDataSizeInBytes = 0)const;
		uint32 GetVirtualFileSystemSize()const;

		eMASH_STATUS ReadFile(const int8 *sFileName, eFILE_IO_MODE mode, void **pOutData, uint32 &iOutDataSizeInBytes);
		eMASH_STATUS WriteFile(const int8 *sFileName, eFILE_IO_MODE mode, void *pData, uint32 iDataSizeInBytes);
	};

	inline uint32 CMashFileManager::GetVirtualFileSystemSize()const
	{
		return m_iVirtualFileSystemMemorySize;
	}
}

#endif