//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PHYSICS_H_
#define _MASH_PHYSICS_H_

#include "MashReferenceCounter.h"
#include "MashCreationParameters.h"
#include "MashPhysicsCommon.h"
#include "MashString.h"

namespace mash
{
	class MashTriangleBuffer;
	class MashModel;
	class MashEntity;
	class MashVector3;
	class MashPhysicsRigidBody;
	class MashPhysicsCollisionShape;
	class MashPhysicsNode;
	class MashSceneNode;
	class MashVideo;
	class MashSceneManager;

    /*!
        Scene nodes can be added to this manager to move in a realistic maner, with or
        without collisions.
     
        Call AddRigidBody() to add a rigid physics object to the simulator. If you want
        this object to collide with the scene then pass in a collsion shape into this
        function. 
     
        Collision shapes can be created for moving or static non-moving shapes. 
        Dynamic shapes are good for characters, cars, moving barrels, etc..
        Static shapes are good for landscapes and buildings. Use primitive convex shapes
        for anythings thats static and requires basic collision detection. Triangle buffers
        can create collision shapes for complex meshes.
     
        Scene nodes still need to be updated and drawn through the scene manager like normal.
        The nodes will be updated after _Simulate() is called, this is after MashGameLoop::Update()
        and before MashGameLoop::LateUpdate().
    */
	class MashPhysics : public MashReferenceCounter
	{
	public:
		enum ePHYSICS_DEBUG_RENDER
		{
			aPDR_WIREFRAME,
			aPDR_AABB
		};
	public:
		MashPhysics():MashReferenceCounter(){}
		virtual ~MashPhysics(){}

        //! Adds a rigid body to the simulator.
        /*!
            The returned point must not be dropped. Call RemoveRigidBody() to remove.
         
            \param node Scene node to simulate.
            \param rigidBodyConstructor Collision and movement settings.
            \param collisionObject Collision object. This object can be shared by many rigid bodies.
                Set to NULL to stop collisions with this object.
            \return New physics object.
        */
		virtual MashPhysicsRigidBody* AddRigidBody(MashSceneNode *node, const sRigisBodyConstruction &rigidBodyConstructor, MashPhysicsCollisionShape *collisionObject) = 0;

        //! Creates a basic convex collision shape.
        /*!
            A collision shape can be shared by one or many rigid bodies. The returned point
            will need to be dropped when you are done with it.
         
            These shapes offer faster collision detection than shapes made up from triangle buffers.
            Use these where possible for static or dynamic objects.
         
            \param collisionObj Collision constructor.
            \return Collision shape.
        */
		virtual MashPhysicsCollisionShape* CreateCollisionShape(const sCollisionObjectConstruction &collisionObj) = 0;
        
        //! Creates a complex collision shape from triangle buffers.
        /*!
            A collision shape can be shared by one or many rigid bodies. The returned point
            will need to be dropped when you are done with it.
         
            Creates a dynamic collision shape based on a complex mesh. Use CreateStatcxxx() to
            create large static collision shapes such as landscapes.
         
            \param triangleBuffers A list of triangle buffers to create this shape from.
            \param bufferCount Number of buffers in the array.
            \param optimize Set to true to optimze the final mesh. False to keep the current layout.
            \return Collision shape.
        */
		virtual MashPhysicsCollisionShape* CreateCollisionShape(MashTriangleBuffer **triangleBuffers, uint32 bufferCount, bool optimize = true) = 0;
        
        //! Creates a complex collision shape from a model.
        /*!
            Helper method that calls on CreateCollisionShape(). See CreateCollisionShape() for more info.
         
            \param model Model to extract triangle buffers from.
            \param lod Lod to grab the triangle buffers from.
            \param generateTriangleBufferIfNull Generates triangle buffers if not present in a mesh.
            \param optimize Set to true to optimze the final mesh. False to keep the current layout.
            \return Collision shape.
        */
		virtual MashPhysicsCollisionShape* CreateCollisionShape(MashModel *model, uint32 lod = 0, bool generateTriangleBufferIfNull = true, bool optimize = true) = 0;

        //! Creates a static complex collision shape.
		/*!
            A collision shape can be shared by one or many rigid bodies. The returned point
            will need to be dropped when you are done with it.
         
            These shapes used for non-moving complex meshes such as landscapes. They share
            data from the triangle buffers to reduce memory consumption.
         
            \param triangleBuffers List of triangles to create this shape from.
            \bufferCount Number of buffers in the array.
            \return Collision shape.
		*/
		virtual MashPhysicsCollisionShape* CreateStaticTriangleCollisionShape(MashTriangleBuffer **triangleBuffers, uint32 bufferCount) = 0;
        
        //! Creates a static complex collision shape from a mode.
        /*!
            A collision shape can be shared by one or many rigid bodies. The returned point
            will need to be dropped when you are done with it.
         
            These shapes used for non-moving complex meshes such as landscapes. They share
            data from the triangle buffers to reduce memory consumption.
         
            \param model Model to extract triangle buffers from.
            \param lod Lod to grab the triangle buffers from.
            \param generateTriangleBufferIfNull Generates triangle buffers if not present in a mesh.
            \return Collision shape.
        */
		virtual MashPhysicsCollisionShape* CreateStaticTriangleCollisionShape(MashModel *model, uint32 lod = 0, bool generateTriangleBufferIfNull = true) = 0;

        //! Gets a physics node by scene node name.
        /*!
            \param name This is the name within MashSceneNode.
            \return Physics node with the given name. NULL if the node was not found.
        */
		virtual MashPhysicsNode* GetPhysicsNodeByName(const MashStringc &name)const = 0;
        
        //! Gets a physics node by scene node id.
        /*!
            \param nodeId This is the id from MashSceneNode::GetNodeId().
            \return Physics node with the given id. NULL if the node was not found.
        */
		virtual MashPhysicsNode* GetPhysicsNodeById(uint32 nodeId)const = 0;

        //! Sets the global gravity.
        /*!
            This force will affect all objects in the scene.
         
            \param gravity Global gravity.
        */
		virtual void SetGravity(const MashVector3 &gravity) = 0;
        
        //! Removes a rigid body from the physics manager.
        /*!
            This will not remove the scene node from the scene manager.
            Removes the rigid body from this manager and drops its reference.
         
            \param rigidBody Rigid body to remove.
        */
        virtual void RemoveRigidBody(MashPhysicsNode *rigidBody) = 0;
        
        //! Removes all reigid bodies from this manager and drops their reference.
        /*!
            This will not remove scene nodes from the scene manager.
        */
		virtual void RemoveAllRigidBodies() = 0;
        
        //! Draws the collision bounds for debugging.
        /*!
            This must be called within MashGameLoop::Render().
         
            \param type Describes how to render the debug objects.
        */
        virtual void DrawDebug(ePHYSICS_DEBUG_RENDER type = aPDR_AABB) = 0;

        //! Called internally to simulate the current scene.
        /*!
            This is called after MashGameLoop::Update() and before NseGameLoop::LateUpdate().
         
            \param dt Variable time since last update.
        */
		virtual void _Simulate(f32 dt) = 0;

        //! Called internally to initialise this manager.
        /*!
            \param renderer Renderer.
            \param sceneManager Scene manager.
        */
		virtual void _Initialise(MashVideo *renderer, MashSceneManager *sceneManager, const sMashDeviceSettings &settings) = 0;
	};

    //! Called internally to create a physics manager.
	_MASH_EXPORT MashPhysics* CreateMashPhysics();
}

#endif