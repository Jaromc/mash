//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SCENE_PICK_H_
#define _MASH_SCENE_PICK_H_

#include "MashCompileSettings.h"
#include "MashTypes.h"
#include "MashArray.h"

namespace mash
{
	class MashSceneNode;
	class MashRay;

    /*!
        Collision method for testing a ray against a scene node and its children.
    */
	class _MASH_EXPORT MashScenePick
	{
	public:
		MashScenePick(){}
		virtual ~MashScenePick(){}

        //! Gets all the nodes that intersect an array
        /*!
            \param scene Scene graph to test.
            \param ray Ray to test in world space.
            \param typesToTest Bitwise eNODE_TYPE to test.
            \param out Contains all the nodes whose bounds intersect the ray.
            \return True if any nodes intersected the ray. 
        */
		static bool GetNodesByBounds(MashSceneNode *scene,
			const MashRay &ray,
			uint32 typesToTest,
			MashArray<MashSceneNode*> &out);

        //! Gets all the triangles that intersect an array.
        /*!
            Triangle colliders must be added to nodes for this to work.
         
            \param scene Scene graph to test.
            \param ray Ray to test in world space.
            \param typesToTest Bitwise eNODE_TYPE to test.
            \param backFaceCull Set to true to cull triangle backfaces when testing.
            \param out Contains all the triangles that intersect the ray.
            \return True if any triangles intersected the ray. 
        */
		static bool GetTrianglesFromScene(MashSceneNode *scene,
			const MashRay &ray,
			uint32 typesToTest,
			bool backFaceCull,
			MashArray<sTriPickResult> &out);
		
        //! Gets all the closest node to the array.
        /*!         
            \param scene Scene graph to test.
            \param ray Ray to test in world space.
            \param typesToTest Bitwise eNODE_TYPE to test.
            \param maximumRayLength Pass in the max distance from the ray to test. The distance to 
                the node found is returned here.
            \param out Contains the node that intersect the ray.
            \return True if any nodes intersected the ray. 
         */
		static bool GetClosestNodeByBounds(MashSceneNode *scene,
			const MashRay &ray,
			uint32 typesToTest,
			f32 &maximumRayLength,
			MashSceneNode **out);

        //! Gets the closest triangle to the ray.
		/*!
            Triangle colliders must be added to nodes for this to work.
         
            \param scene Scene graph to test.
            \param ray Ray to test in world space.
            \param typesToTest Bitwise eNODE_TYPE to test.
            \param backFaceCull Set to true to cull triangle backfaces when testing.
            \param out Contains the triangle that intersects the ray.
            \return True if any triangles intersected the ray. 
		*/
		static bool GetClosestTriFromScene(MashSceneNode *scene,
			const MashRay &ray,
			uint32 typesToTest,
			bool backFaceCull,
			sTriPickResult &out);
	};	
}

#endif