//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashXMLReader.h"
#include "MashFileManager.h"
#include "MashFileStream.h"
#include "MashLog.h"

namespace mash
{
	CMashXMLReader::CMashXMLReader(MashFileManager *fileManager):MashXMLReader(), m_fileManager(fileManager)
	{

	}

	CMashXMLReader::~CMashXMLReader()
	{

	}

	bool CMashXMLReader::LoadFile(const int8 *sFileName)
	{
		MashFileStream *fileStream = m_fileManager->CreateFileStream();
		if (fileStream->LoadFile(sFileName, aFILE_IO_TEXT) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
				"CMashXMLReader::LoadFile", 
				"File not found '%s'", 
				sFileName);

			return aMASH_FAILED;
		}

		m_document.Parse((const int8*)fileStream->GetData());

		fileStream->Destroy();

		TiXmlHandle documentHandle(&m_document);

		TiXmlElement *pElement = documentHandle.FirstChildElement().ToElement();
		if (!pElement)
		{
			return false;
		}

		m_elementStack.push(pElement);

		return true;
	}

	void CMashXMLReader::Destroy()
	{
		MASH_DELETE this;
	}

	void CMashXMLReader::GetFileAsString(MashStringc &out)
	{
		TiXmlPrinter printer;
		m_document.Accept(&printer);
		out = printer.CStr();
	}

	bool CMashXMLReader::PopChild()
	{
		if (m_elementStack.size() > 1)
		{
			m_elementStack.pop();
			return true;
		}

		return false;
	}

	bool CMashXMLReader::MoveToFirstChild()
	{
		TiXmlHandle handle(m_elementStack.top());
		TiXmlElement *pElement = handle.FirstChild().ToElement();
		if (!pElement)
			return false;

		m_elementStack.push(pElement);

		return true;
	}

	bool CMashXMLReader::MoveToFirstChild(const int8 *sName)
	{
		TiXmlHandle handle(m_elementStack.top());
		TiXmlElement *pElement = handle.FirstChild(sName).ToElement();
		if (!pElement)
			return false;

		m_elementStack.push(pElement);

		return true;
	}

	bool CMashXMLReader::MoveToNextSibling()
	{
		TiXmlElement *pElement = m_elementStack.top()->NextSiblingElement();
		if (!pElement)
			return false;

		m_elementStack.top() = pElement;

		return true;
	}

	bool CMashXMLReader::MoveToNextSibling(const int8 *sName)
	{
		TiXmlElement *pElement = m_elementStack.top()->NextSiblingElement(sName);
		if (!pElement)
			return false;

		m_elementStack.top() = pElement;

		return true;
	}

	bool CMashXMLReader::GetName(MashStringc &sValue)const
	{
		const int8 *s = GetNameRaw();
		if (!s)
			return false;

		sValue = s;

		return true;
	}

	bool CMashXMLReader::GetText(MashStringc &sValue)const
	{
		const int8 *s = GetTextRaw();
		if (!s)
			return false;

		sValue = s;

		return true;
	}

	const int8* CMashXMLReader::GetNameRaw()const
	{
		return m_elementStack.top()->Value();
	}

	const int8* CMashXMLReader::GetTextRaw()const
	{
		return m_elementStack.top()->GetText();
	}

	const int8* CMashXMLReader::GetAttributeRaw(const int8 *sName)const
	{
		return m_elementStack.top()->Attribute(sName);;
	}

	bool CMashXMLReader::GetAttributeString(const int8 *sName, MashStringc &sValue)const
	{
		const int8 *v = m_elementStack.top()->Attribute(sName);
		if (!v)
			return false;

		sValue = v;

		return true;
	}

	bool CMashXMLReader::GetAttributeFloat(const int8 *sName, f32 &fValue)const
	{
		if (m_elementStack.top()->QueryFloatAttribute(sName, &fValue) != TIXML_SUCCESS)
			return false;

		return true;
	}

	bool CMashXMLReader::GetAttributeDouble(const int8 *sName, f64 &dValue)const
	{
		if (m_elementStack.top()->QueryDoubleAttribute(sName, &dValue) != TIXML_SUCCESS)
			return false;

		return true;
	}

	bool CMashXMLReader::GetAttributeInt(const int8 *sName, int32 &iValue)const
	{
		if (m_elementStack.top()->QueryIntAttribute(sName, &iValue) != TIXML_SUCCESS)
			return false;

		return true;
	}
}