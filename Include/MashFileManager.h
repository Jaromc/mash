//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_FILE_MANAGER_H_
#define _MASH_FILE_MANAGER_H_

#include "MashEnum.h"
#include "MashReferenceCounter.h"
#include "MashArray.h"
#include "MashString.h"

namespace mash
{
	class MashLuaScript;
	class MashFileStream;
	class MashXMLReader;
	class MashXMLWriter;
    
    /*!
        This class deals with loading/saving files from different API's and API formats,
        and automatically handing loading from API or Virtual file system (VFS) directories.
     
        The VFS can be used for any purpose including adding files from an archive using 
        AddFileToVirtualFileSystem(). The files can then be read from a MashFileStream or ReadFile().
    */
	class MashFileManager : public MashReferenceCounter
	{
	public:
		enum eFILE_ATTRIBUTES
		{
			aFILE_ATTRIB_DIR = 1,
			aFILE_ATTRIB_READONLY = 2,
			aFILE_ATTRIB_PARENT_DIR = 4
		};

		struct sFileAttributes
		{
			MashStringc relativeFilePath;
			MashStringc absoluteFilePath;
			uint32 flags;

			sFileAttributes():relativeFilePath(""), absoluteFilePath(""), flags(0){}
		};
	public:
		MashFileManager():MashReferenceCounter(){}
		virtual ~MashFileManager(){}
        
        //! Converts a relative path into an absoulte path.
        /*!
            \param fileName Relative path.
            \param absPath Absoulte path will be written here.
        */
        virtual void GetAbsoutePath(const int8 *fileName, MashStringc &absPath)const = 0;

        //! Adds a search path for file loading.
        /*!
            Root paths are appended to paths when file loading.
            The root path may exist in the API or virtual file system.
         
            \param path Root path to add.
        */
		virtual void AddRootPath(const int8 *path) = 0;

        //! Creates a new directory in the current API.
        /*!
            \param dir New file directory.
        */
		virtual bool APICreateDirectory(const int8 *dir) = 0;
        
        //! Sets the current directory for the current API.
        /*!
            \param dir Current working directory.
        */
		virtual bool APISetWorkingDirectory(const int8 *dir) = 0;
        
        //! Gets the current directory for the current API.
        /*!
            \param out Current working directory.
         */
		virtual void APIGetCurrentDirectory(MashStringc &out) = 0;
        
        //! Gets the current directory structure for the current API.
        /*!
            The directory passed in must be relative to the current working
            API directory. Use APIGetCurrentDirectory() to find this.
         
            \param dir Directory structure to return.
            \param out Directory details.
         */
		virtual void APIGetDirectoryStructure(const int8 *dir, MashArray<sFileAttributes> &out) = 0;

        //! Creates a new XML writer.
		/*!
			If append is true then a previous file is loaded and
			data can be appended or overriden, rootNodeName is not
			used in this case.

			If append is false or no previous file is found then one will be created
			and returned.
         
            \param fileName File to create or load.
            \param rootNodeName If a new file is created then this is the name of the root node.
            \param append If a file exists should the file be wiped or new data be appended.
            \return XML writer.
		*/
		virtual MashXMLWriter* CreateXMLWriter(const int8 *fileName, const int8 *rootNodeName, bool append = false) = 0;
        
        //! Creates a new XML reader.
        /*!
            Loads an existing file for reading.
            
            \param fileName Existing XML file to load.
            \return XML reader.
        */
		virtual MashXMLReader* CreateXMLReader(const int8 *fileName) = 0;

        //! Adds a text file to the VFS.
        /*!
            \param fileName File name and directory of the data.
            \param data File data.
            \return Status of the function.
        */
		virtual eMASH_STATUS AddStringToVirtualFileSystem(const int8 *fileName, const int8 *data) = 0;
        
        //! Adds a data file to the VFS.
        /*!
            \param fileName File name and directory of the data.
            \param data File data.
            \param dataSizeInBytes Data size in bytes.
            \return Status of the function.
         */
		virtual eMASH_STATUS AddFileToVirtualFileSystem(const int8 *fileName, const void *data, uint32 dataSizeInBytes) = 0;
        
        //! Current size of the VFS.
        /*!
            \return Current size of the VFS in bytes.
        */
		virtual uint32 GetVirtualFileSystemSize()const = 0;
        
        //! Returns if the file exists in the VFS or the API file system.
        /*!
            This function uses the root paths to search for the file.
         
            \param fileName File to search for.
            \return True if it exists, false otherwise.
        */
		virtual bool DoesFileExist(const int8 *fileName)const = 0;

        //! Creates a file stream for normal data reading/writing.
        /*!
            A fileStream is simply a helper class that utilizes some of the reading/writing 
            functions in this class.
         
            \return A new file stream.
        */
		virtual MashFileStream* CreateFileStream() = 0;

        //! Opens a file for reading.
        /*!
            Opens a normal data file for reading from the VFS or API FS.
            The returned data must be deleted by the user.
            A fileStream should be used to simplify the process.
            
            \param fileName File to open.
            \param mode Reading mode.
            \param outData File data.
            \param outDataSizeInBytes Data size in bytes.
        */
		virtual eMASH_STATUS ReadFile(const int8 *fileName, eFILE_IO_MODE mode, void **outData, uint32 &outDataSizeInBytes) = 0;

		//! Writes data to the API FS.
        /*!
            \param fileName File to open.
            \param mode Writing mode.
            \param data Data to write.
            \param dataSizeInBytes Data size in bytes.
        */
		virtual eMASH_STATUS WriteFile(const int8 *fileName, eFILE_IO_MODE mode, void *data, uint32 dataSizeInBytes) = 0;
	};
}

#endif