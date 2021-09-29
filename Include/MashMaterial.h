//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MATERIAL_H_
#define _MASH_MATERIAL_H_

#include "MashReferenceCounter.h"
#include "MashMaterialDependentResource.h"
#include "MashString.h"
#include "MashArray.h"
#include <map>

namespace mash
{
	class MashFileManager;
	class MashVideo;
	class MashSceneManager;
	class MashTexture;
    class MashTextureState;
	class MashTechniqueInstance;
	class MashTechnique;
	class MashCustomRenderPath;
	class MashVertex;
    struct sEffectMacro;
	
    /*!
		Materials hold techniques for shader and render state information.
        Objects using the same material or an instance of the same material
        are grouped together to improve rendering speed. Created from
        MashMaterialManager::AddMaterial().
     
        Users can set callback here through MashMaterial::SetCustomRenderPath() 
        that will be called before meshes are rendered. This allows for custom
        rendering methods and batching.
     
        Materials hold technique groups. Groups can be created for high and low quality
        settings. Then within each group are techniques that are created for different
        shader profiles and/or APIs. Techniques hold the bulk of the rendering state
        information.
     
        Techniques can also be set for lodding. As scene objects move they can tell their
        materials how far away from the camera they are. The material then searches through
        the active groups techniques to find a technique that has been set for that distance.
        MashMaterial::SetLodStartDistances sets the lod distances. Further lod information needs
        to be set in the techniques.
     
        MashMaterial::OnSet is an important function for rendering. It rarely needs
        to be called by the user unless you are calling one of the MashVideo::Drawxxx 
        functions directly. Example:
     
        \code
        if (myMaterial->OnSet())
            renderer->DrawIndexed(...);
        \endcode
	*/
	class MashMaterial : public MashReferenceCounter, public MashMaterialDependentResource<MashMaterial>
	{
	public:
		MashMaterial():MashReferenceCounter(){}
		~MashMaterial(){}

        //! Creates an instance of this material.
        /*!
            An instance will hold all the same shader pointers as the original. This basically
            allows you to set different texture information to objects that use the same
            shaders.
            The engine will attempt to render similar materials together to reduce state changes.
         
            \param name New instance name. Must be unique. Leave NULL to create a default name.
            \param copyTextures Copies texture pointers over into the new instance.
            \return Material instance.
        */
		virtual MashMaterial* CreateInstance(const int8 *name, bool copyTextures = true) = 0;
        
        //! Creates a unique copy of this material.
        /*!
            The new material will have new shaders generated from the original.
            This function should rarely be used. Instead look at CreateInstance().
         
            \param name New material name. Must be unique. Leave NULL to create a default name.
            \param copyTextures Copies texture pointers over into the new material.
            \return New material.
        */
		virtual MashMaterial* CreateIndependentCopy(const int8 *name, bool copyTextures = true) = 0;

        //! Called before rendering a mesh.
        /*!
            This sends shader information to the gpu before rendering. This include auto params and render states.
            It rarely needs to be called directly by the user unless you are accessing
            the MashVideo::Drawxxx functions directly.

			If this This function returns failed then it simply means the current technique is not valid for 
			the current pass (shadow or scene pass), and the MashVideo::Drawxxx function should not be called. 
			It does not mean that an error has occured. For example,

			if (myMaterial->OnSet())
				renderer->DrawIndexed(...);
         
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS OnSet() = 0;

        //! Gets the current active technique.
		/*
            If a technique is not set then this function will
            search for any valid techniques it can set.
         
            \return active technique.
		*/
		virtual MashTechniqueInstance* GetActiveTechnique() = 0;
        
        //! Gets the first technique found.
        /*!
            \return First found technique whether it's valid or not.
        */
		virtual MashTechniqueInstance* GetFirstTechnique()const = 0;

        //! Returns true if auto lodding is enabled.
        /*!
            \return True if auto lodding is enabled, false otherwise.
        */
		virtual bool GetAutoLodEnabled()const = 0;
        
        //! Enables or disables auto lodding.
        /*!
            Set this to true if you want the engine to change techniques based on
            object distance automatically. This is what you normally want. If you 
            want control of what technique is set then set this to false.
         
            \param value Enable or disable auto lod.
        */
		virtual void SetAutoLod(bool value) = 0;

        //! Gets the name of this material.
        /*!
            \return Material name.
        */
		virtual const MashStringc& GetMaterialName()const = 0;
        
        //! Returns true if at least on technique is valid.
        /*!
            This is only valid after the material has been compiled.
         
            \return true if at least one technique is valid. False otherwise.
        */
		virtual bool IsValid()const = 0;
        
        //! Returns true if this material has been compiled.
        /*!
            This will be valid after a materials compile function has been called.
            If all techniques were not valid, this will still be true. To check if
            the material is valid call IsValid().
         
            \return True if the compile function has been called. False otherwise.
        */
		virtual bool IsCompiled()const = 0;

        //! Sets the active technique group.
        /*!
            Groups can be handy for grouping high and low quality shaders together
            for quality settings in your app. The active group will be searched when
            auto lodding.
         
            \param group name.
            \return Ok if the group is exists. Failed otherwise.
        */
		virtual eMASH_STATUS SetActiveGroup(const int8 *group) = 0;
        
        //! Gets the active group name.
        /*!
            \return Active group name.
        */
		virtual const MashStringc& GetActiveGroup()const = 0;

        //! Sets the texture to all techniques.
		/*!
			\param index Technique index.
            \param texture Texture pointer.
		*/
		virtual void SetTexture(uint32 index, MashTexture *texture) = 0;
        
        //! Sets the texture state to all techniques.
        /*!
            \param index Texture index.
            \param state Texture state.
        */
		virtual void SetTextureState(uint32 index, MashTextureState *state) = 0;

		//! Adds a new technique instance to this material.
        /*!
            \param groupName Group to add it to.
            \param techniqueName New technique name.
            \param refTechnique Technique to create an instance from. Maybe NULL.
            \return New technique.
        */
		virtual MashTechniqueInstance* AddTechniqueToGroup(const int8 *groupName, const int8 *techniqueName, MashTechnique *refTechnique = 0) = 0;
        
        //! Gets a technique by name.
        /*!
            \return Technique found.
        */
		virtual MashTechniqueInstance* GetTechniqueByName(const int8 *name) = 0;

        //! Finds the first technique in the active group that is valid for a lod.
        /*!
            \param lod Lod to set.
            \return Technique that was found.
        */
		virtual MashTechniqueInstance* GetBestTechniqueForLod(uint32 lod) = 0;
        
        //! Finds the first technique that is valid for a lod.
        /*!
            \param group Group to search.
            \param lod Lod to set.
            \return Technique that was found.
        */
		virtual MashTechniqueInstance* GetBestTechniqueForLodByGroup(const int8 *group, uint32 lod) = 0;

        //! Sets the distances where each lod will activate.
        /*!
            There should be one element per lod. Each element will contain the starting distance for that lod.
            The distances should be in assending order. A distance of 0 is automatically added, so the array
            should contain distances of greater than 0 for the following lods.
         
            \param distances An array that holds a lods distance in each element.
            \param lodCount Number of elements in the array.
        */
		virtual eMASH_STATUS SetLodStartDistances(const uint32 *distances, uint32 lodCount) = 0;
        
        //! Gets the lod for a distance.
        /*!
            \param distance Lod distance to query.
            \return Lod level for the distance.
        */
		virtual uint32 GetLodFromDistance(uint32 distance)const = 0;
        
        //! Gets the lod distance list.
        /*!
            This list should not be altered from here.
            \return Lod distance list.
        */
        virtual const MashArray<uint32>& GetLodList()const = 0;

        //! Compiles the techniques within this material.
        /*!
            This is called automatically if the material has been created before the users
            Initialise() function. If a material is loaded after that then this will need
            to be called manually.
         
            IsValid() should be checked after to make sure the material is valid.
         
            \param fileManager File manager.
            \param sceneManager Scene manager.
            \param compileFlags bitwise eMATERIAL_COMPILER_FLAGS.
            \param args Args to pass the the techniques for compiling. May be NULL.
            \param argCount Number of arguments in the array.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS CompileTechniques(MashFileManager *pFileManager, MashSceneManager *sceneManager, uint32 compileFlags, const sEffectMacro *args = 0, uint32 argCount = 0) = 0;

		//! Returns true if this material has lod levels other than 0.
        /*!
            \return True if this material has lod levels other than 0. False otherwise.
        */
		virtual bool GetHasMultipleLodLevels()const = 0;
        
        //! Updates the active technique based on distance.
        /*!
            If auto lodding is not enabled then this function will have no effect.
            
            \param distance Distance for lodding.
            \return Ok on success. Failed otherwise.
        */
		virtual eMASH_STATUS UpdateActiveTechnique(uint32 distance = 0) = 0;

        //! Sets a custom render path.
        /*!
            This allows custom rendering and batching. The callback will be called before
            an object is rendered instead of sending it to the screen.
         
            Any previous paths will be dropped and the new one grabbed.
         
            \param renderPath Render path. May be null to drop the current path.
        */
		virtual void SetCustomRenderPath(MashCustomRenderPath *renderPath) = 0;
        
        //! Returns the current render path.
        /*!
            \return Current render path.
        */
		virtual MashCustomRenderPath* GetCustomRenderPath()const = 0;

        
		//! Gets the vertex declaration that all the techniques within this material share.
        /*!
            \return Materials vertex declaration.
        */
		virtual MashVertex* GetVertexDeclaration()const = 0;

        //! Returns the technique list.
        /*!
            This list should not be altered from here.
            \return Technique list.
        */
		virtual const std::map<MashStringc, MashArray<MashTechniqueInstance*> >& GetTechniqueList()const = 0;

        //! Sets vertex declaration.
        /*!
            Internal method for setting this materials vertex declaration.
            This should match the declaration that all the techniques within
            this material use.
         
            \param vertexDeclaration Vertex declaration.
        */
        virtual void _SetVertexDeclaration(MashVertex *vertexDeclaration) = 0;
        
        //! Clears this material so its ready for a reload.
        virtual void _OnReload() = 0;
	};
}

#endif