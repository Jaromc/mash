//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_XML_READER_H_
#define _MASH_XML_READER_H_

#include "MashReferenceCounter.h"
#include "MashString.h"

namespace mash
{
    /*!
        An XML reader can be created from MashFileManager::CreateXMLReader().
        When your done with it call Destroy().
     
        This reader assumes you know the general layout of the XML file.
     
        Example usage:
     
        <RootNode>
            <Hand HandName="MyBigHand" HandSize="10.0">
                <Finger FingerName="FirstFinger" />
                <Finger FingerName="SecondFinger" />
            </Hand>
        </RootNode>
     
        if (MoveToFirstChild("Hand"))
        {
            MashStringc handName;
            f32 handSize = 0.0f;
     
            GetAttributeString("HandName", handName);
            GetAttributeFloat("HandSize", handSize);
     
            if (MoveToFirstChild("Finger"))
            {
                do
                {
                    MashStringc fingerName;
                    GetAttributeString("FingerName", fingerName);
                    
                }while(MoveToNextSibling("Finger"));
     
                PopChild();
            }
     
            PopChild();
        }
    */
	class MashXMLReader : public MashReferenceCounter
	{
	public:
		MashXMLReader():MashReferenceCounter(){}
		virtual ~MashXMLReader(){}

        //! Call this to destroy the reader.
		virtual void Destroy() = 0;
        
        //! Gets the XML file string.
        /*!
            \param out The file will be filled here.
        */
		virtual void GetFileAsString(MashStringc &out) = 0;

        //! Moves to the first child.
        /*!
            This is usually the first function called when starting to read
            a file. This just moves to the first child, regardless of name.
         
            Call PopChild() when you leave the child.
         
            \return True if there was a child to move to.
        */
		virtual bool MoveToFirstChild() = 0;
        
        //! Moves to the next child with the given name.
        /*!
            Call PopChild() when you leave the child.
         
            \param name Sibling to move to.
            \return True if there was a child with the given name to move to.
        */
		virtual bool MoveToFirstChild(const int8 *name) = 0;
        
        //! Moves to the next sibling.
        /*!
            \return True if there was a sibling to move to.
        */
		virtual bool MoveToNextSibling() = 0;
        
        //! Moves to the next sibling with the given name.
        /*!
            \param name Sibling to move to.
            \return True if there was a sibling with the given name to move to.
         */
		virtual bool MoveToNextSibling(const int8 *name) = 0;

        //! Pops out of a child.
		/*
			For every call to MoveToFirstChild() there must be a call to PopChild(). Calling PopChild() after
			MoveToNextSibling() is NOT needed.
         
            \return True if the pop was valid. False otherwise.
		*/
		virtual bool PopChild() = 0;

		//! Gets the name of the current node.
        /*!
            \param value Node name will be filled here.
            \return False if any errors.
        */
		virtual bool GetName(MashStringc &value)const = 0;
        
        //! Gets <Node>This is some text</Node>.
        /*
            \param value Nodes text will be filled here.
            \return False if any errors.
        */
        virtual bool GetText(MashStringc &value)const = 0;
        
        //! Gets a string.
        /*!
            \param name Attribute to get.
            \param value Attributes value will be filled here.
            \return True if the attribute exists.
        */
        virtual bool GetAttributeString(const int8 *name, MashStringc &value)const = 0;
        
        //! Gets a f32.
        /*!
            \param name Attribute to get.
            \param value Attributes value will be filled here.
            \return True if the attribute exists.
        */
		virtual bool GetAttributeFloat(const int8 *name, f32 &value)const = 0;
        
        //! Gets a f64.
        /*!
            \param name Attribute to get.
            \param value Attributes value will be filled here.
            \return True if the attribute exists.
        */
		virtual bool GetAttributeDouble(const int8 *name, f64 &value)const = 0;
        
        //! Gets an int32.
        /*!
            \param name Attribute to get.
            \param value Attributes value will be filled here.
            \return True if the attribute exists.
        */
		virtual bool GetAttributeInt(const int8 *name, int32 &value)const = 0;
        
        //! Returns a pointer to the name of the current node.
        /*!
            The returned pointer must not be altered or deleted.
        */
		virtual const int8* GetNameRaw()const = 0;
        
        //! Returns a pointer to the text of the current node.
        /*!
            Gets <Node>This is some text</Node>.
            The returned pointer must not be altered or deleted.
        */
		virtual const int8* GetTextRaw()const = 0;
        
        //! Returns a pointer to an attributes data.
        /*!
            The returned pointer must not be altered or deleted.
         
            \param name Attribute to get.
        */
		virtual const int8* GetAttributeRaw(const int8 *name)const = 0;
	};
}

#endif