//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_VERTEX_H_
#define _MASH_VERTEX_H_

#include "MashReferenceCounter.h"
#include "MashTypes.h"

namespace mash
{
	class MashVideo;
	class MashEffect;

	/*!
        Holds vertex data used in mesh rendering.
     
        Stream 0 is known as the geometry stream. This is where all mesh geoemtry is held.
        Other streams may be utilized for insteance data or any other custom data that may
        be accessed from the GPU.
    */
	class MashVertex : public MashReferenceCounter
	{
	public:
		MashVertex():MashReferenceCounter(){}
		virtual ~MashVertex(){}
        
        //! Gets all the elements of this vertex. Call GetVertexElementCount() to get the number of elements.
		virtual const sMashVertexElement* GetVertexElements()const = 0;
        
        //! Gets the number of elements in this vertex.
		virtual uint32 GetVertexElementCount()const = 0;

        //! Returns true if this vertex contains a particular usage.
        /*!
            \param check Usage to check.
            \param Stream to check.
            \return True if the usage is contained within the given stream. False otherwise. 
        */
		virtual bool Contains(eVERTEX_DECLUSAGE check, uint32 stream = 0)const = 0;
        
        //! Returns true if this vertex is equal to the elements passed in.
        /*!
            \param vertexDecl Vertex element array.
            \param elementCount Number of elements in the vertex array.
            \param stream Stream to test. Set to -1 to test all streams.
            \return True if the vertex decls are equal, false otherwise.
        */
		virtual bool IsEqual(const sMashVertexElement *vertexDecl, uint32 elementCount, int32 stream = -1)const = 0;
        
        //! Returns true if this vertex is equal to another.
        /*!
            \param vertex Vertex decl to test against.
            \param stream Stream to test. Set to -1 to test all streams.
            \return True if the vertex decls are equal, false otherwise.
        */
		virtual bool IsEqual(const MashVertex *vertex, int32 stream = -1)const = 0;
		
        //! Gets an elements stride.
        /*!
            \param stream Stream to search in.
            \param usage Usage to query.
			\return Ok if the usage was found in the given stream. Failed otherwise.
        */
		virtual eMASH_STATUS GetElementStride(uint32 stream, eVERTEX_DECLUSAGE usage, uint32 &out)const = 0;

		//! Gets an element by usage.
		/*!
			\param stream Stream to search in.
            \param usage Usage to return.
			\return Element pointer or NULL is usage was not found.
		*/
		virtual const sMashVertexElement* GetElement(uint32 stream, eVERTEX_DECLUSAGE usage)const = 0;

		//! Gets the number of streams in this declaration.
		virtual uint32 GetStreamCount()const = 0;

		//! Gets a streams size in bytes.
		/*!
			\param stream Stream to query.
			\return Stream size in bytes.
		*/
		virtual uint32 GetStreamSizeInBytes(uint32 stream)const = 0;
	};
}

#endif