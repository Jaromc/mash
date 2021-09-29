//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_PHYSICS_COMMON_H_
#define _MASH_PHYSICS_COMMON_H_

namespace mash
{
	enum eMASH_PHYSICS_OBJECT_TYPE
	{
		aPHYSICS_OBJ_RIGIDBODY
	};

	enum eMASH_PHYSICS_SHAPE
	{
		aPHYSICS_SHAPE_PLANE,
		aPHYSICS_SHAPE_CUBE,
		aPHYSICS_SHAPE_CAPSULE,
		aPHYSICS_SHAPE_CYLINDER,
		aPHYSICS_SHAPE_CONE,
		aPHYSICS_SHAPE_SPHERE,

		aPHYSICS_OBJ_COUNT
	};

	struct sRigisBodyConstruction
	{
        /*!
            The more massive an object the more force required to move it (higher inertial state).
            Set this to 0.0 for immovable objects.
        */
		f32 mass;
        
        /*!
            The friction of the collision surface.
        */
		f32 friction;
        
        /*!
            Two objects with a coefficient of restitution == 1 collide
            elasically. No energy is lost in the collision.
            This value should be <= 1.0f;
            Set the restitution to < 1 to remove some energy in collisions.
        */
		f32 restitution;
        
        /*!
            Slows angular velocity a small amount on each update. Basically
            acts as a drag on the angular velocity to slow it down. 
            This should be a value <= 1.0f. Set to 0.0 for no damping. Setting
            this to 1.0 will result in no movement.
        */
		f32 angularDamping;
        
        /*!
            Slows linear velocity a small amount on each update. Basically
            acts as a drag on the linear velocity to slow it down. 
            This should be a value <= 1.0f. Set to 0.0 for no damping. Setting
            this to 1.0 will result in no movement.
        */
		f32 linearDamping;

		sRigisBodyConstruction():mass(1.0f), friction(1.0f), restitution(0.0f),
		angularDamping(0.0f), linearDamping(0.0f){}
	};

    /*!
        Used to create a basic primitive collision shape.
     
        Fill out the correct structure in the union based
        on the eMASH_PHYSICS_SHAPE selected.
    */
	struct sCollisionObjectConstruction
	{
		eMASH_PHYSICS_SHAPE type;

        struct sVector
        {
            f32 x, y, z;

			void Set(f32 _x, f32 _y, f32 _z)
			{
				x = _x;
				y = _y;
				z = _z;
			}
        };
        
		struct sSphereBased
		{
			f32 radius;
			f32 height;
		};

		struct sPlane
		{
			sVector normal;
			f32 d;
		};

		union
		{
            sPlane plane;
            sVector cubeHalfExt;
            sVector cylinderHalfExt;
			sSphereBased capsule;
			sSphereBased cone;
			f32 sphereRadius;
		};
	};
}

#endif