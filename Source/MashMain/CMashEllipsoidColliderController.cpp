//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashEllipsoidColliderController.h"
#include "MashTriangleCollider.h"
#include "MashTriangleBuffer.h"
#include "MashSceneNode.h"
#include "MashPlane.h"

namespace mash
{
	CMashEllipsoidColliderController::CMashEllipsoidColliderController(MashSceneNode *sceneNode, 
			MashSceneNode *collisionScene, 
			const MashVector3 &radius,
			const MashVector3 &gravity):MashEllipsoidColliderController(),
		m_collisionScene(collisionScene), m_radius(radius), m_gravity(gravity)
	{
		if (sceneNode)//should always be provided
			m_lastPosition = sceneNode->GetUpdatedWorldTransformState().translation;

		//to stop divide by zeros
		m_radius.x = math::Max<f32>(0.001f, m_radius.x);
		m_radius.y = math::Max<f32>(0.001f, m_radius.y);
		m_radius.z = math::Max<f32>(0.001f, m_radius.z);
	}
    
	CMashEllipsoidColliderController::~CMashEllipsoidColliderController()
	{
	}

	void CMashEllipsoidColliderController::SetCollisionScene(MashSceneNode *collisionScene)
	{
        /*
            Note, collision is not grabbed because if sever nodes share the collision scene then
            somenode will never be destroyed because each node grabs the other.
        */

		m_collisionScene = collisionScene;
	}

	bool CMashEllipsoidColliderController::CheckPointInTriangle(const MashVector3 &vPoint,
													const MashVector3 &pa,
													const MashVector3 &pb,
													const MashVector3 &pc)
	{
		MashVector3 e10 = pb-pa;
		MashVector3 e20 = pc-pa;

		f32 a = e10.Dot(e10);
		f32 b = e10.Dot(e20);
		f32 c = e20.Dot(e20);
		f32 ac_bb = (a*c)-(b*b);
		MashVector3 vp(vPoint.x-pa.x, vPoint.y-pa.y, vPoint.z-pa.z);

		f32 d = vp.Dot(e10);
		f32 e = vp.Dot(e20);
		f32 x = (d*c)-(e*b);
		f32 y = (e*a)-(d*b);
		f32 z = x+y-ac_bb;

		return ((((uint32&)z)& ~(((uint32&)x)|((uint32&)y))) & 0x80000000)!=0;
	}

	bool CMashEllipsoidColliderController::GetLowestRoot(f32 a, f32 b, f32 c, f32 maxR, f32 &root)
	{
		//check if a solution exists
		f32 fDeterminant = b*b - 4.0f*a*c;

		if (fDeterminant < 0.0f)
			return false;

		f32 fSqrtD = sqrtf(fDeterminant);
		f32 r1 = (-b - fSqrtD) / (2*a);
		f32 r2 = (-b + fSqrtD) / (2*a);

		if (r1 > r2)
		{
			f32 temp = r2;
			r2 = r1;
			r1 = temp;
		}

		//get lowest root
		if ((r1 > 0.0f) && (r1 < maxR))
		{
			root = r1;
			return true;
		}

		if ((r2 > 0.0f) && (r2 < maxR))
		{
			root = r2;
			return true;
		}

		return false;
	}

	void CMashEllipsoidColliderController::CheckTriangle(CollisionPacket &colPacket,
											const MashVector3 &p1,
											const MashVector3 &p2,
											const MashVector3 &p3)
	{
		//create the plane containing this triangle
		MashPlane trianglePlane(p1,p2,p3);

		//only check triangles that are facing the velocity vector
		if (trianglePlane.normal.Dot(colPacket.normalizedVelocity) <= 0.0f)
		{
			//get interval of plane intersection
			f32 t0,t1;
			bool bEmbeddedInPlane = false;

			//calculate the distance from the sphere
			//to the triangle plane
			f32 fDistToTriPlane = trianglePlane.Distance(colPacket.basePoint);

			f32 fNormalDotVel = trianglePlane.normal.Dot(colPacket.velocity);

			//test if sphere is travelling parrallel to the plane
			if (math::FloatEqualTo(fNormalDotVel, 0.0f))
			{
				if (fabs(fDistToTriPlane) >= 1.0f)
				{
					//sphere is not embedded in plane therefor no collision
					return;
				}
				else
				{
					//sphere is embedded in plane
					//it intersects in the whole range[0...1]
					bEmbeddedInPlane = true;
					t0 = 0.0f;
					t1 = 1.0f;
				}
			}
			else
			{
				//calculate intersection interval
				t0 = (-1.0f-fDistToTriPlane)/fNormalDotVel;
				t1 = (1.0f-fDistToTriPlane)/fNormalDotVel;

				//make sure t0 < t1
				if (t0 > t1)
				{
					f32 temp = t1;
					t1 = t0;
					t0 = temp;
				}

				//check that at least one result is within range
				if (t0 > 1.0f || t1 < 0.0f)
				{
					//both values are outside [0...1] therefore no collision
					return;
				}

				//clamp values
				t0 = math::Clamp<f32>(0.0f,1.0f,t0);
				t1 = math::Clamp<f32>(0.0f,1.0f,t1);
			}

			MashVector3 vCollisionPoint;
			bool foundCollision = false;
			f32 t = 1.0f;

			//first check the easy case of a collision
			//inside the triangle
			if (!bEmbeddedInPlane)
			{
				MashVector3 vPlaneIntersectionPoint = (colPacket.basePoint - trianglePlane.normal)
					+ (t0*colPacket.velocity);

				if (CheckPointInTriangle(vPlaneIntersectionPoint, p1, p2, p3))
				{
					foundCollision = true;
					t = t0;
					vCollisionPoint = vPlaneIntersectionPoint;
				}
			}

			//if the sphere is not inside the triangle, then we must
			//do a sweep test against the edges and verts of the triangle.
			//Note : the check above will always occur before an edge or vertex
			//collision.
			if (foundCollision == false)
			{
				MashVector3 velocity = colPacket.velocity;
				MashVector3 vBase = colPacket.basePoint;
				f32 fVelLengthSq = velocity.LengthSq();
				f32 a,b,c;
				f32 newT;

				//for each vertex or edge a quadratic equation has to be solved

				//check against points
				a = fVelLengthSq;

				//p1
				b = 2.0f*(velocity.Dot(vBase-p1));
				c = (p1-vBase).LengthSq() - 1.0f;
				if (GetLowestRoot(a,b,c,t,newT))
				{
					t = newT;
					foundCollision = true;
					vCollisionPoint = p1;
				}

				if (foundCollision == false)
				{
					//p2
					b = 2.0f*(velocity.Dot(vBase-p2));
					c = (p2-vBase).LengthSq() - 1.0f;
					if (GetLowestRoot(a,b,c,t,newT))
					{
						t = newT;
						foundCollision = true;
						vCollisionPoint = p2;
					}
				}

				if (foundCollision == false)
				{
					//p3
					b = 2.0f*(velocity.Dot(vBase-p3));
					c = (p3-vBase).LengthSq() - 1.0f;
					if (GetLowestRoot(a,b,c,t,newT))
					{
						t = newT;
						foundCollision = true;
						vCollisionPoint = p3;
					}
				}

				//now check against edges

				//p1->p2
				MashVector3 vEdge = p2-p1;
				MashVector3 vBaseToVertex = p1 - vBase;
				f32 fEdgeLengthSq = vEdge.LengthSq();
				f32 fEdgeDotVelocity = vEdge.Dot(velocity);
				f32 fEdgeDotBaseToVertex = vEdge.Dot(vBaseToVertex);

				a = fEdgeLengthSq*-fVelLengthSq + fEdgeDotVelocity*fEdgeDotVelocity;
				b = fEdgeLengthSq*(2.0f*velocity.Dot(vBaseToVertex)) - 
					2.0f*fEdgeDotVelocity*fEdgeDotBaseToVertex;
				c = fEdgeLengthSq*(1.0f-vBaseToVertex.LengthSq())+
					fEdgeDotBaseToVertex*fEdgeDotBaseToVertex;

				//does the swept sphere collide against infinite edge?
				if (GetLowestRoot(a,b,c,t,newT))
				{
					//check if intersection is within the line segment
					f32 f = (fEdgeDotVelocity*newT-fEdgeDotBaseToVertex)/fEdgeLengthSq;
					if (f >= 0.0f && f <= 1.0f)
					{
						//intersection took place within the line segment
						t = newT;
						foundCollision = true;
						vCollisionPoint = p1+(f*vEdge);
					}
				}

				//p2->p3
				vEdge = p3-p2;
				vBaseToVertex = p2 - vBase;
				fEdgeLengthSq = vEdge.LengthSq();
				fEdgeDotVelocity = vEdge.Dot(velocity);
				fEdgeDotBaseToVertex = vEdge.Dot(vBaseToVertex);

				a = fEdgeLengthSq*-fVelLengthSq + fEdgeDotVelocity*fEdgeDotVelocity;
				b = fEdgeLengthSq*(2.0f*velocity.Dot(vBaseToVertex)) - 
					2.0f*fEdgeDotVelocity*fEdgeDotBaseToVertex;
				c = fEdgeLengthSq*(1.0f-vBaseToVertex.LengthSq())+
					fEdgeDotBaseToVertex*fEdgeDotBaseToVertex;

				//does the swept sphere collide against infinite edge?
				if (GetLowestRoot(a,b,c,t,newT))
				{
					//check if intersection is within the line segment
					f32 f = (fEdgeDotVelocity*newT-fEdgeDotBaseToVertex)/fEdgeLengthSq;
					if (f >= 0.0f && f <= 1.0f)
					{
						//intersection took place within the line segment
						t = newT;
						foundCollision = true;
						vCollisionPoint = p2+(f*vEdge);
					}
				}

				//p3->p1
				vEdge = p1-p3;
				vBaseToVertex = p3 - vBase;
				fEdgeLengthSq = vEdge.LengthSq();
				fEdgeDotVelocity = vEdge.Dot(velocity);
				fEdgeDotBaseToVertex = vEdge.Dot(vBaseToVertex);

				a = fEdgeLengthSq*-fVelLengthSq + fEdgeDotVelocity*fEdgeDotVelocity;
				b = fEdgeLengthSq*(2.0f*velocity.Dot(vBaseToVertex)) - 
					2.0f*fEdgeDotVelocity*fEdgeDotBaseToVertex;
				c = fEdgeLengthSq*(1.0f-vBaseToVertex.LengthSq())+
					fEdgeDotBaseToVertex*fEdgeDotBaseToVertex;

				//does the swept sphere collide against infinite edge?
				if (GetLowestRoot(a,b,c,t,newT))
				{
					//check if intersection is within the line segment
					f32 f = (fEdgeDotVelocity*newT-fEdgeDotBaseToVertex)/fEdgeLengthSq;
					if (f >= 0.0f && f <= 1.0f)
					{
						//intersection took place within the line segment
						t = newT;
						foundCollision = true;
						vCollisionPoint = p3+(f*vEdge);
					}
				}
			}

			//set the result
			if (foundCollision == true)
			{
				//calculate the distance to collision.
				//t == time of collision
				f32 fDistToCollision = t*colPacket.velocity.Length();

				//if this triangle is the closest hit so far OR its the first
				//collision found then update the collision data
				if ((colPacket.foundCollision == false) ||
					(fDistToCollision < colPacket.nearestDistance))
				{
					colPacket.nearestDistance = fDistToCollision;
					colPacket.intersectionPoint = vCollisionPoint;
					colPacket.foundCollision = true;
					//colPacket.collisionNormal = trianglePlane.normal;
				}
			}
		}
	}

	void CMashEllipsoidColliderController::CollideWithWorldTriangles(MashSceneNode *thisSceneNode, CollisionPacket &collisionPacket, MashSceneNode *node)
	{
		//dont test self
		if (thisSceneNode != node)
		{
			const MashTriangleCollider *collider = node->GetTriangleCollider();
			if (collider)
			{
				m_collidingTriangleBuffer.Clear();

				if (collisionPacket.aabb.Intersects(node->GetWorldBoundingBox()))
				{
					collider->GetIntersectingTriangles(collisionPacket.aabb, node->GetWorldTransformState(), m_collidingTriangleBuffer);
					MashArray<sIntersectingTriangleResult>::Iterator iter = m_collidingTriangleBuffer.Begin();
					MashArray<sIntersectingTriangleResult>::Iterator iterEnd = m_collidingTriangleBuffer.End();
					for(; iter != iterEnd; ++iter)
					{
						const MashTriangleBuffer *triBuffer = collider->GetTriangleBuffer(iter->bufferIndex);
						CheckTriangle(collisionPacket, node->GetWorldTransformState().Transform(triBuffer->GetPoint(iter->triangleIndex, 0)) / collisionPacket.eRadius, 
							node->GetWorldTransformState().Transform(triBuffer->GetPoint(iter->triangleIndex, 1)) / collisionPacket.eRadius,
							node->GetWorldTransformState().Transform(triBuffer->GetPoint(iter->triangleIndex, 2)) / collisionPacket.eRadius);
					}
				}
			}
		}

		MashList<MashSceneNode*>::ConstIterator nodeIter = node->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator nodeIterEnd = node->GetChildren().End();
		for(; nodeIter != nodeIterEnd; ++nodeIter)
		{
			CollideWithWorldTriangles(thisSceneNode, collisionPacket, *nodeIter);
		}
	}

	MashVector3 CMashEllipsoidColliderController::CollideWithWorld(MashSceneNode *thisSceneNode, 
		CollisionPacket &collisionPackage, const MashVector3 &pos, const MashVector3 &vel)
	{
		// All hard-coded distances in this function is
		// scaled to fit the setting above..
		//f32 unitScale = unitsPerMeter / 100.0f;
		f32 veryCloseDistance = 0.005f;// * unitScale;
		// do we need to worry?
		if (collisionRecursionDepth > 5)
			return pos;

		// Ok, we need to worry:
		collisionPackage.velocity = vel;
		collisionPackage.normalizedVelocity = vel;
		collisionPackage.normalizedVelocity.Normalize();
		collisionPackage.basePoint = pos;
		collisionPackage.foundCollision = false;
		collisionPackage.nearestDistance = mash::math::MaxFloat();

		const MashVector3 newPosWorldSpace = (pos + vel) * collisionPackage.eRadius;
		collisionPackage.aabb.min = newPosWorldSpace - (collisionPackage.eRadius);
		collisionPackage.aabb.max = newPosWorldSpace + (collisionPackage.eRadius);

		// Check for collision (calls the collision routines)
		// Application specific!!
		//world->checkCollision(collisionPackage);

		CollideWithWorldTriangles(thisSceneNode, collisionPackage, m_collisionScene);

		// If no collision we just move along the velocity
		if (collisionPackage.foundCollision == false) 
		{
			return pos + vel;
		}

		// *** Collision occured ***
		// The original destination point
		MashVector3 destinationPoint = pos + vel;
		MashVector3 newBasePoint = pos;
		// only update if we are not already very close
		// and if so we only move very close to intersection..not
		// to the exact spot.

		//collisionPackage.nearestDistance = math::Max<f32>(veryCloseDistance, collisionPackage.nearestDistance);

		if (collisionPackage.nearestDistance >= veryCloseDistance)
		{
			MashVector3 V = vel;
			V.Normalize();
			V *= (collisionPackage.nearestDistance - veryCloseDistance);
			//V.SetLength(collisionPackage.nearestDistance - veryCloseDistance);
			newBasePoint = collisionPackage.basePoint + V;
			// Adjust polygon intersection point (so sliding
			// plane will be unaffected by the fact that we
			// move slightly less than collision tells us)
			V.Normalize();
			collisionPackage.intersectionPoint -= (veryCloseDistance * V);
		}

		// Determine the sliding plane
		MashVector3 slidePlaneOrigin = collisionPackage.intersectionPoint;
		MashVector3 slidePlaneNormal = newBasePoint-collisionPackage.intersectionPoint;
		MashPlane slidingPlane(slidePlaneNormal, slidePlaneOrigin);
		// Again, sorry about formatting.. but look carefully ;)
		MashVector3 newDestinationPoint = destinationPoint - (slidingPlane.Distance(destinationPoint) * slidingPlane.normal);
		// Generate the slide vector, which will become our new
		// velocity vector for the next iteration
		MashVector3 newVelocityVector = newDestinationPoint - collisionPackage.intersectionPoint;
		// Recurse:
		// dont recurse if the new velocity is very small
		if (newVelocityVector.LengthSq() < (veryCloseDistance * veryCloseDistance)) 
		{
			return newBasePoint;
		}

		++collisionRecursionDepth;
		return CollideWithWorld(thisSceneNode, collisionPackage, newBasePoint, newVelocityVector);
	}

    void CMashEllipsoidColliderController::OnNodeUpdate(MashSceneNode *sceneNode, f32 dt)
    {
        if (dt == 0.0f)
			return;

		CollisionPacket collisionPacket;

		collisionPacket.eRadius = m_radius;
		collisionPacket.R3Position = m_lastPosition;
		collisionPacket.R3Velocity = (sceneNode->GetUpdatedWorldTransformState().translation - m_lastPosition);

		MashVector3 eSpacePosition = collisionPacket.R3Position / collisionPacket.eRadius;
		MashVector3 eSpaceVelocity = collisionPacket.R3Velocity / collisionPacket.eRadius;

		collisionRecursionDepth = 0;

		MashVector3 finalPosition = CollideWithWorld(sceneNode, collisionPacket, eSpacePosition, eSpaceVelocity);

		//add gravity

		//Set the new R3 position (convert back from eSpace to R3
		collisionPacket.R3Position = finalPosition * collisionPacket.eRadius;
		collisionPacket.R3Velocity = m_gravity;
		eSpaceVelocity = m_gravity / collisionPacket.eRadius;

		collisionRecursionDepth = 0;
		finalPosition = CollideWithWorld(sceneNode, collisionPacket, finalPosition, eSpaceVelocity);

		//Convert final result back to R3:
		finalPosition = finalPosition * collisionPacket.eRadius;

		m_lastPosition = finalPosition;

		sceneNode->SetPosition(finalPosition);
    }
    
}