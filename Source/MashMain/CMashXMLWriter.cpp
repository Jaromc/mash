//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashXMLWriter.h"
#include "MashFileManager.h"
#include "MashFileStream.h"
#include "MashLog.h"

namespace mash
{
	CMashXMLWriter::CMashXMLWriter(MashFileManager *fileManager):MashXMLWriter(), m_pRoot(), m_sFileName(""),
        m_fileManager(fileManager)
	{

	}

	CMashXMLWriter::~CMashXMLWriter()
	{

	}

	bool CMashXMLWriter::LoadFile(const int8 *sFileName, const int8 *sRootNodeName, bool append)
	{
		if (!m_pRoot)
		{
			m_sFileName = sFileName;

			if (append)
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
                
				/*if (m_document.LoadFile(sFileName))
				{
					TiXmlHandle documentHandle(&m_document);

					TiXmlElement *pElement = documentHandle.FirstChildElement().ToElement();
					if (!pElement)
					{
						return false;
					}

					m_elementStack.push(pElement);
				}*/
			}

			/*
				If we are loading/overwriting a file or a
				previous file is not found
			*/
			if (!append || m_elementStack.empty())
			{
				m_pRoot = new TiXmlElement(sRootNodeName);
				m_document.LinkEndChild(m_pRoot);
				m_elementStack.push(m_pRoot);
			}
		}

		return true;
	}

	bool CMashXMLWriter::WriteChild(const int8 *sName)
	{
		TiXmlElement *pCurrentElement = new TiXmlElement(sName);

		if (!m_elementStack.empty())
		{
			m_elementStack.top()->LinkEndChild(pCurrentElement);
		}

		m_elementStack.push(pCurrentElement);

		return true;
	}

	void CMashXMLWriter::PopChild()
	{
		/*
			Never pop the root element
		*/
		if (m_elementStack.size() > 1)
			m_elementStack.pop();
	}

	bool CMashXMLWriter::WriteAttributeInt(const int8 *sName, int32 iValue)
	{
		m_elementStack.top()->SetAttribute(sName, iValue);
		return true;
	}

	bool CMashXMLWriter::WriteAttributeDouble(const int8 *sName, f64 dValue)
	{
		m_elementStack.top()->SetDoubleAttribute(sName, dValue);
		return true;
	}

	bool CMashXMLWriter::WriteAttributeString(const int8 *sName, const int8 *sValue)
	{
		m_elementStack.top()->SetAttribute(sName, sValue);
		return true;
	}

	bool CMashXMLWriter::WriteComment(const int8 *sComment)
	{
		TiXmlComment *pComment = new TiXmlComment();
		pComment->SetValue(sComment);
		m_elementStack.top()->LinkEndChild(pComment);

		return true;
	}

	bool CMashXMLWriter::WriteText(const int8 *sText)
	{
		TiXmlText *pText = new TiXmlText(sText);
		m_elementStack.top()->LinkEndChild(pText);

		return true;
	}

	void CMashXMLWriter::SaveAndDestroy()
	{
		TiXmlPrinter printer;
        m_document.Accept(&printer);
  
        MashFileStream *fileStream = m_fileManager->CreateFileStream();
        fileStream->AppendStringToStream(printer.CStr());
        fileStream->SaveFile(m_sFileName.GetCString(), aFILE_IO_TEXT);
        fileStream->Destroy();
		Destroy();
	}

	void CMashXMLWriter::Destroy()
	{
		MASH_DELETE this;
	}

	void CMashXMLWriter::GetFileAsString(MashStringc &out)
	{
		TiXmlPrinter printer;
		m_document.Accept(&printer);
		out = printer.CStr();
	}

	void CMashXMLWriter::RemoveCurrentNode()
	{
		//don't delete thr root element
		if (m_elementStack.size() > 1)
		{
			TiXmlElement *toRemove = m_elementStack.top();
			m_elementStack.pop();

			m_elementStack.top()->RemoveChild(toRemove);
		}
	}

	bool CMashXMLWriter::MoveToFirstChild()
	{
		TiXmlHandle handle(m_elementStack.top());
		TiXmlElement *pElement = handle.FirstChild().ToElement();
		if (!pElement)
			return false;

		m_elementStack.push(pElement);

		return true;
	}

	bool CMashXMLWriter::MoveToFirstChild(const int8 *sName)
	{
		TiXmlHandle handle(m_elementStack.top());
		TiXmlElement *pElement = handle.FirstChild(sName).ToElement();
		if (!pElement)
			return false;

		m_elementStack.push(pElement);

		return true;
	}

	bool CMashXMLWriter::MoveToNextSibling()
	{
		TiXmlElement *pElement = m_elementStack.top()->NextSiblingElement();
		if (!pElement)
			return false;

		m_elementStack.top() = pElement;

		return true;
	}

	bool CMashXMLWriter::MoveToNextSibling(const int8 *sName)
	{
		TiXmlElement *pElement = m_elementStack.top()->NextSiblingElement(sName);
		if (!pElement)
			return false;

		m_elementStack.top() = pElement;

		return true;
	}

	bool CMashXMLWriter::GetName(MashStringc &sValue)const
	{
		const int8 *s = m_elementStack.top()->Value();
		if (!s)
			return false;

		sValue = s;

		return true;
	}

	bool CMashXMLWriter::GetText(MashStringc &sValue)const
	{
		const int8 *s = m_elementStack.top()->GetText();
		if (!s)
			return false;

		sValue = s;

		return true;
	}

	bool CMashXMLWriter::GetAttributeString(const int8 *sName, MashStringc &sValue)const
	{
		const int8 *v = m_elementStack.top()->Attribute(sName);
		if (!v)
			return false;

		sValue = v;

		return true;
	}

	bool CMashXMLWriter::GetAttributeFloat(const int8 *sName, f32 &fValue)const
	{
		if (m_elementStack.top()->QueryFloatAttribute(sName, &fValue) != TIXML_SUCCESS)
			return false;

		return true;
	}

	bool CMashXMLWriter::GetAttributeDouble(const int8 *sName, f64 &dValue)const
	{
		if (m_elementStack.top()->QueryDoubleAttribute(sName, &dValue) != TIXML_SUCCESS)
			return false;

		return true;
	}

	bool CMashXMLWriter::GetAttributeInt(const int8 *sName, int32 &iValue)const
	{
		if (m_elementStack.top()->QueryIntAttribute(sName, &iValue) != TIXML_SUCCESS)
			return false;

		return true;
	}
}