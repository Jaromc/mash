//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_XML_WRITER_H_
#define _C_MASH_XML_WRITER_H_

#include "MashXMLWriter.h"
#include <stack>
#include "tinystr.h"
#include "tinyxml.h"

namespace mash
{
    class MashFileManager;
    
	class CMashXMLWriter : public MashXMLWriter
	{
	private:
        MashFileManager *m_fileManager;
		MashStringc m_sFileName;
		TiXmlDocument m_document;
		TiXmlElement *m_pRoot;
		std::stack<TiXmlElement*> m_elementStack;
	public:
		CMashXMLWriter(MashFileManager *fileManager);
		~CMashXMLWriter();

		bool LoadFile(const int8 *sFileName, const int8 *sRootNodeName, bool append = false);

		void SaveAndDestroy();
		void Destroy();
		void GetFileAsString(MashStringc &out);

		bool WriteChild(const int8 *sName);
		void PopChild();

		bool WriteAttributeInt(const int8 *sName, int32 iValue);
		bool WriteAttributeDouble(const int8 *sName, f64 dValue);
		bool WriteAttributeString(const int8 *sName, const int8 *sValue);
		bool WriteComment(const int8 *sComment);
		bool WriteText(const int8 *sText);

		void RemoveCurrentNode();
		bool MoveToFirstChild();
		bool MoveToFirstChild(const int8 *sName);
		bool MoveToNextSibling();
		bool MoveToNextSibling(const int8 *sName);
		bool GetName(MashStringc &sValue)const;
		bool GetText(MashStringc &sValue)const;
		bool GetAttributeString(const int8 *sName, MashStringc &sValue)const;
		bool GetAttributeFloat(const int8 *sName, f32 &fValue)const;
		bool GetAttributeDouble(const int8 *sName, f64 &dValue)const;
		bool GetAttributeInt(const int8 *sName, int32 &iValue)const;
	};
}

#endif