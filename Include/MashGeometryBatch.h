//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GEOMETRY_BATCH_H_
#define _MASH_GEOMETRY_BATCH_H_

#include "MashDataTypes.h"
#include "MashEnum.h"
#include "MashReferenceCounter.h"

namespace mash
{
	class MashRenderable;
	class MashSkin;

	/*!
		Can be used to batch render objects using the same material.
        A user may derive from this to create their own custom render implimentation.
		See MashMaterial::SetGeometryBatch.
	*/
	class MashCustomRenderPath : public MashReferenceCounter
	{
	private:
		bool m_registeredForFlush;
	public:
		MashCustomRenderPath():MashReferenceCounter(), m_registeredForFlush(false){}
		virtual ~MashCustomRenderPath(){}

		/*!
			Once an object has passed culling and is about to render, this is called
			by the engine instead of rendering the object directly to the target.
         
            \param renderable Object to render.
		*/
		virtual void AddObject(MashRenderable *renderable) = 0;

		/*!
			Called when the engine wants this buffer to be rendered to the screen.
            This is usually on a material change or end of rendering.
		*/
		virtual void Flush() = 0;

		//! Used internally
		void _SetRegisteredForFlushFlag(bool state){m_registeredForFlush = state;}
		bool _IsRegisteredForFlush()const{return m_registeredForFlush;}
	};

	/*!
        These can be created from MashVideo::CreateGeometryBatch.
     
		This is a prebuilt way of rendering a large amount of primitives in 
        an efficient way.
	*/
	class MashGeometryBatch : public MashReferenceCounter
	{
	public:
		enum eBATCH_TYPE
		{
			aSTATIC,
			aDYNAMIC
		};
	public:
		MashGeometryBatch():MashReferenceCounter(){}
		virtual ~MashGeometryBatch(){}

        //! Adds points to the batch.
        /*!
            \param points array for vertices.
            \param count Vertex count.
            \return Ok on success. Failed otherwise. 
        */
		virtual eMASH_STATUS AddPoints(const uint8 *points, uint32 count) = 0;
        
        //! Flushes the buffer to the screen.
		virtual eMASH_STATUS Flush() = 0;
	};
}

#endif