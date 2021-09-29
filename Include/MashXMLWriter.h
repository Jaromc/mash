//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_XML_WRITER_H_
#define _MASH_XML_WRITER_H_

#include "MashReferenceCounter.h"
#include "MashString.h"

namespace mash
{
    /*!
        An XML writer can be created from MashFileManager::CreateXMLWriter().
        When your done with it call Destroy() or SaveAndDestroy().
     
        Example usage:
     
        <RootNode>
            <Hand HandName="MyBigHand" HandSize="10.0">
                <Finger FingerName="FirstFinger" />
                <Finger FingerName="SecondFinger" />
            </Hand>
        </RootNode>
     
        MashXMLWriter *xmlWriter = fileManager->CreateXMLWriter(savePath, "RootNode");
        xmlWriter->WriteChild("Hand");
        xmlWriter->WriteAttributeString("HandName", "MyBigHand");
        xmlWriter->WriteAttributeDouble("HandSize", 10.0);
            xmlWriter->WriteChild("Finger");
            xmlWriter->WriteAttributeString("FingerName", "FirstFinger");
            xmlWriter->PopChild();
     
            xmlWriter->WriteChild("Finger");
            xmlWriter->WriteAttributeString("FingerName", "SecondFinger");
            xmlWriter->PopChild();
        xmlWriter->PopChild();
        xmlWriter->SaveAndDestroy();
    */
	class MashXMLWriter : public MashReferenceCounter
	{
	public:
		MashXMLWriter():MashReferenceCounter(){}
		virtual ~MashXMLWriter(){}

        //! Savings the file to disk and destroys this writer.
		virtual void SaveAndDestroy() = 0;
        
        //! Destroys this writer without writing the file to disk.
		virtual void Destroy() = 0;
        
        //! Gets the current XML file as a string.
        /*!
            \param out XML file will be written here.
        */
		virtual void GetFileAsString(MashStringc &out) = 0;

        //! Writes a new child.
        /*!
            \param name Node name.
            \return False if any errors.
        */
		virtual bool WriteChild(const int8 *name) = 0;

        //! Pops out of a child node.
		/*
			This must be called after WriteChild() to return to the parent nodes.
		*/
		virtual void PopChild() = 0;

        //! Writes an int32.
        /*!
            \param name Attribute name.
            \param value Attribute value.
            \return False if any errors.
        */
		virtual bool WriteAttributeInt(const int8 *name, int32 value) = 0;
        
        //! Writes a f64.
        /*!
            \param name Attribute name.
            \param value Attribute value.
            \return False if any errors.
        */
		virtual bool WriteAttributeDouble(const int8 *name, f64 value) = 0;
        
        //! Writes a string.
        /*!
            \param name Attribute name.
            \param value Attribute value.
            \return False if any errors.
        */
		virtual bool WriteAttributeString(const int8 *name, const int8 *value) = 0;
        
        //! Writes an xml comment.
        /*!
            \param comment Comment string.
            \return False if any errors.
        */
		virtual bool WriteComment(const int8 *comment) = 0;
        
        //! Writes text.
        /*!
            Example <Node>This is some text</Node>.
         
            \param text Text to add to the node.
            \return False if any errors.
        */
		virtual bool WriteText(const int8 *text) = 0;
	};
}

#endif