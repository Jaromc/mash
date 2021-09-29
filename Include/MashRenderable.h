//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_RENDERABLE_H_
#define _MASH_RENDERABLE_H_


namespace mash
{
	class MashAABB;
	class MashMaterial;

    /*!
        Implimented by renderable scene objects.
     
        Scene nodes aren't necessarily the renderable objects, some impliement smaller
        objects that are rendered. This class exposes things that are needed to render
        the object in a scene.
    */
	class MashRenderable
	{
	public:
		MashRenderable(){}
		virtual ~MashRenderable(){}

        //! Returns a bounding box that encompases a scene node and its children.
		virtual const MashAABB& GetTotalWorldBoundingBox()const = 0;
        
		//! Returns a bounding box that encompases a scene nodes object only.
		virtual const MashAABB& GetWorldBoundingBox()const = 0;
        
        //! Returns the active material for rendering.
		virtual MashMaterial* GetMaterial()const = 0;
        
        //! Called to draw this object to the screen.
        /*!
            This should call one of the MashVideo::Drawxxx methods or 
            MashCustomRenderPath::AddObject().
        */
		virtual void Draw() = 0;
	};
}

#endif