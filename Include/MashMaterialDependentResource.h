//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MATERIAL_DEPENDENT_RESOURCE_H_
#define _MASH_MATERIAL_DEPENDENT_RESOURCE_H_

namespace mash
{
	class MashVideo;

	/*!
		This class is used internally by various objects that are dependent on
		particular API objects to be compiled. The root of this dependency is the material.

		This helps to stall loading of API specific elemenets until all material and scene
        initialization is complete to eliminate unneeded compiling. 
     
        This class should never need to be accessed by the user.
	*/
	class MashMaterialDependentResourceBase
	{
	public:
		MashMaterialDependentResourceBase(){}
		virtual ~MashMaterialDependentResourceBase(){}

		//! Called after the dependency this object is waiting on is compiled.
        /*!
            Once the dependency is compiled, then this object can compile.
         
            \param renderer Renderer.
            \param dependency Resource this object is waiting on.
        */
		virtual void OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency) = 0;
        
        //! Returns true when the API object has been compiled.
        /*!
            \return True when the API object has been compiled. False if it has not yet been compiled.
        */
		virtual bool IsValid()const = 0;

		//! Increments a resources reference counter.
		virtual void GrabResource() = 0;
        
        //! Drops a resources reference counter.
        /*!
            \return True if it was deleted. False otherwise.
        */
		virtual bool DropResource() = 0;
	};

    /*!
        Intermediate class. All deriving should be done from here.
    */
	template<class T>
	class MashMaterialDependentResource : public MashMaterialDependentResourceBase
	{
	public:
		MashMaterialDependentResource(){}
		virtual ~MashMaterialDependentResource(){}

		//These simply call the derived methods Grab and Drop methods.
		void GrabResource(){((T*)this)->Grab();}
		bool DropResource(){return ((T*)this)->Drop();}
	};
}

#endif