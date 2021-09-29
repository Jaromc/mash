//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_XML_READER_H_
#define _C_MASH_XML_READER_H_

#include "MashXMLReader.h"
#include <stack>
#include "tinystr.h"
#include "tinyxml.h"

namespace mash
{
	class MashFileManager;
	class CMashXMLReader : public MashXMLReader
	{
	private:
		TiXmlDocument m_document;
		MashFileManager *m_fileManager;
		std::stack<TiXmlElement*> m_elementStack;
	public:
		CMashXMLReader(MashFileManager *fileManager);
		~CMashXMLReader();

		bool LoadFile(const int8 *sFileName);
		void Destroy();
		void GetFileAsString(MashStringc &out);

		bool MoveToFirstChild();
		bool MoveToFirstChild(const int8 *sName);
		bool MoveToNextSibling();
		bool MoveToNextSibling(const int8 *sName);

		/*
			For every call to MoveToFirstChild there must be a
			call to PopChild(). Calling PopChild after
			MoveToNextSibling is NOT needed.
		*/
		bool PopChild();

		bool GetName(MashStringc &sValue)const;
		bool GetText(MashStringc &sValue)const;

		const int8* GetNameRaw()const;
		const int8* GetTextRaw()const;

		bool GetAttributeString(const int8 *sName, MashStringc &sValue)const;
		bool GetAttributeFloat(const int8 *sName, f32 &fValue)const;
		bool GetAttributeDouble(const int8 *sName, f64 &dValue)const;
		bool GetAttributeInt(const int8 *sName, int32 &iValue)const;
		const int8* GetAttributeRaw(const int8 *sName)const;
	};
}

#endif