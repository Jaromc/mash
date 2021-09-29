//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SUB_ENTITY_H_
#define _MASH_SUB_ENTITY_H_

#include "MashRenderable.h"

namespace mash
{
	class MashMesh;
	class MashMaterial;

    /*!
        Sub entities are used by MashEntity.
     
        Entities instance out models, and sub entities instance out meshes.
        Sub entities are not scene nodes, these are the renderable objects
        added to the scene manager each frame for rendering.
    */
	class MashSubEntity : public MashReferenceCounter, public MashRenderable
	{
	public:
		MashSubEntity():MashReferenceCounter(), MashRenderable(){}
		virtual ~MashSubEntity(){}

        //! Enables or disables this sub entity for rendering.
        /*!
            Setting this to false will cause this sub entity to not render.
         
            \param active True to render this object, false to keep it hidden.
        */
		virtual void SetIsActive(bool active) = 0;
        
        //! Gets the render state of this object.
        /*!
            \return True if rendering is enabled, false otherwise.
        */
		virtual bool GetIsActive()const = 0;
        
        //! Sets the material that will be used to render the mesh assigned to this object.
        /*!
            If the materials vertex declaration is different to what the mesh currently
            has set, then the mesh will be rebuilt with the new delcaration. Beware if this mesh
            is used elsewhere then it will be affected there to.
         
            \param material Material to set.
            \return Ok on success, failed otherwise.            
        */
		virtual eMASH_STATUS SetMaterial(MashMaterial *material) = 0;
		
        //! Gets the mesh assigned to this sub entity.
		virtual MashMesh* GetMesh() = 0;
	};
}

#endif