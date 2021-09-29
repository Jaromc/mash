//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashGeometryHelper.h"

#include "MashMatrix4.h"
#include "MashAABB.h"
#include "MashSphere.h"
#include "MashPlane.h"
#include "MashRay.h"
#include "MashOBB.h"
#include "MashTriangle.h"
#include "MashVector3.h"
#include "MashRay.h"
#include "MashLog.h"
#include "MashMathHelper.h"

namespace mash
{
    namespace collision
    {
        bool AABB_AABB(const MashAABB &a, const MashAABB &b)
        {
            if (a.max.x < b.min.x || a.min.x > b.max.x)
                return false;
            if (a.max.y < b.min.y || a.min.y > b.max.y)
                return false;
            if (a.max.z < b.min.z || a.min.z > b.max.z)
                return false;

            return true;
        }

        bool AABB_Sphere(const MashAABB &a, const MashSphere &b)
        {
            mash::MashVector3 vClosestPt;

            a.ClosestPoint(b.center, vClosestPt);
            f32 fDistSqr = b.center.GetDistanceToSQ(vClosestPt);

            return fDistSqr <= b.radius * b.radius;
        }

        bool AABB_Plane(const MashAABB &a, const MashPlane &b)
        {
            mash::MashVector3 vCenter = (a.max + a.min) * 0.5f;
            mash::MashVector3 vExtent = a.max - vCenter;

            f32 fInterval = vExtent.x * math::_abs<f32>(b.normal.x) + 
                            vExtent.y * math::_abs<f32>(b.normal.y) + 
                            vExtent.z * math::_abs<f32>(b.normal.z);

            f32 fCenterDist = b.normal.Dot(vCenter) - b.dist;

            return math::_abs<f32>(fCenterDist) <= fInterval;
        }

        bool AABB_Triangle(const MashAABB &aabb, const MashVector3 &pointA, const MashVector3 &pointB, const MashVector3 &pointC)
        {
            f32 p0, p2, r;

            MashVector3 c = (aabb.min + aabb.max) * 0.5f;
            f32 e0 = (aabb.max.x - aabb.min.x) * 0.5f;
            f32 e1 = (aabb.max.y - aabb.min.y) * 0.5f;
            f32 e2 = (aabb.max.z - aabb.min.z) * 0.5f;

            MashVector3 v0 = pointA - c;
            MashVector3 v1 = pointB - c;
            MashVector3 v2 = pointC - c;

            MashVector3 f0 = v1 - v0, f1 = v2 - v1, f2 = v0 - v2;

            p0 = v0.z*v1.y - v0.y*v1.z;
            p2 = v2.z*(v1.y - v0.y) - v2.y*(v1.z - v0.z);
            r = (e1 * fabs(f0.z)) + (e2 * fabs(f0.y));
            if (math::Max<f32>(-math::Max<f32>(p0, p2), math::Min<f32>(p0, p2)) > r) 
                return false;


            if ((math::Max<f32>(v0.x, v1.x, v2.x) < -e0) || (math::Min<f32>(v0.x, v1.x, v2.x) > e0))
                return false;

            if ((math::Max<f32>(v0.y, v1.y, v2.y) < -e1) || (math::Min<f32>(v0.y, v1.y, v2.y) > e1))
                return false;

            if ((math::Max<f32>(v0.z, v1.z, v2.z) < -e2) || (math::Min<f32>(v0.z, v1.z, v2.z) > e2))
                return false;

            MashPlane p;
            p.normal = f0.Cross(f1).Normalize();
            p.dist = -p.normal.Dot(v0);

            MashAABB b2(aabb.min - c, aabb.max - c);
            return AABB_Plane(b2, p);
        }

        bool Ray_AABB(const MashAABB &a, const MashRay &b)
        {
            /*
                TODO : Fast ray-aabb test without distance calculation
            */
            f32 tempDistance;
            return Ray_AABB(a, b, tempDistance);
        }

        bool Sphere_Sphere(const MashSphere &a, const MashSphere &b)
        {
            mash::MashVector3 vDist = a.center - b.center;
            f32 fDist = vDist.Dot(vDist);

            f32 fRadiusSum = a.radius + b.radius;
            return fDist <= fRadiusSum * fRadiusSum;
        }

        bool Sphere_OBB(const MashSphere &a, const MashOBB &b)
        {
            MashVector3 vClosestPoint;
            b.ClosestPoint(a.center, vClosestPoint);

            mash::MashVector3 vDist = vClosestPoint - a.center;
            return vDist.Dot(vDist) <= a.radius * a.radius;
        }

        bool Sphere_Plane(const MashSphere &a, const MashPlane &b)
        {
            f32 fDist = a.center.Dot(b.normal) - b.dist;

            return math::_abs(fDist) <= a.radius;
        }

        bool OBB_Plane(const MashOBB &a, const MashPlane &b)
        {
            f32 fInterval = a.halfWidth.x * math::_abs<f32>(b.normal.Dot(a.localAxis[0])) + 
                            a.halfWidth.y * math::_abs<f32>(b.normal.Dot(a.localAxis[1])) + 
                            a.halfWidth.z * math::_abs<f32>(b.normal.Dot(a.localAxis[2]));

            //distance of box center from the plane
            f32 fCenterDist = b.normal.Dot(a.center) - b.dist;

            return math::_abs(fCenterDist) <= fInterval;
        }

        bool Ray_OBB(const MashOBB &a, const MashRay &b, f32 &t)
        {
            const f32 fEpsilon = 0.00001f;
            f32 tMin = 0.0f;
            f32 tMax = 99999999.f;
            
            mash::MashVector3 vRelCenter = a.center - b.origin;

            for (int32 i = 0; i < 3; i++)
            {
                f32 e = a.localAxis[i].Dot(vRelCenter);
                f32 f = a.localAxis[i].Dot(b.dir);

                if (fabs(f) < fEpsilon)
                {
                    if (((-e - a.halfWidth.v[i]) > 0.0f) || ((-e + a.halfWidth.v[i]) < 0.0f))
                        return false;
                }
                else
                {
                    f32 t1 = (e + a.halfWidth.v[i]) / f;
                    f32 t2 = (e - a.halfWidth.v[i]) / f;

                    if (t1 > t2)
                    {
                        f32 temp = t1;
                        t1 = t2;
                        t2 = temp;
                    }

                    if (t1 > tMin)
                        tMin = t1;
                    if (t2 > tMax)
                        tMax = t2;

                    if (tMin > tMax)
                        return false;
                    if (tMax < 0.0f)
                        return false;
                }
            }

            if (tMin > 0.0f)
                t = tMin;
            else
                t = tMax;

            return true;
        }

        bool Ray_Plane(const MashPlane &a, const MashRay &b, f32 &t)
        {
            f32 fD = a.normal.Dot(b.dir);

            //ray is parallel to the plane
            if (fabs(fD) < 0.00001f)
                return false;

            //plane normal points away from the ray direction
            if (fD > 0.0f)
                return false;

            f32 f1 = -((a.normal.Dot(b.origin)) + a.dist);

            f32 fTempT = f1/fD;

            //intersection before ray origin
            if (fTempT < 0.0f)
                return false;

            t = fTempT;

            return true;
        }

        bool Ray_AABB(const MashAABB &a, const MashRay &b, f32 &t)
        {
            const f32 fEpsilon = 0.00001f;
            f32 tMin = 0.0f;
            f32 tMax = 99999999.f;

            for (int32 i = 0; i < 3; i++)
            {
                if (fabs(b.dir.v[i]) < fEpsilon)
                {
                    if ((b.origin.v[i] < a.min.v[i]) || (b.origin.v[i] > a.max.v[i]))
                        return false;
                }
                else
                {
                    f32 ood = 1.0f/b.dir.v[i];
                    f32 t1 = (a.min.v[i] - b.origin.v[i])*ood;
                    f32 t2 = (a.max.v[i] - b.origin.v[i])*ood;

                    if (t1 > t2)
                    {
                        f32 temp = t1;
                        t1 = t2;
                        t2 = temp;
                    }

                    if (t1 > tMin)
                        tMin = t1;
                    if (t2 < tMax)
                        tMax = t2;

                    if (tMin > tMax)
                        return false;
                }
            }

            t = tMin;

            return true;
        }

        f32 GetDistanceToAABB(const mash::MashVector3 &vPosition, const MashAABB &aabb)
        {
            mash::MashRay ray(vPosition, aabb.GetCenter() - vPosition);
            f32 fDistance = 0.0f;
            Ray_AABB(aabb, ray, fDistance);

            return fDistance;
        }

        bool Ray_Sphere(const MashSphere &a, const MashRay &b, f32 &t)
        {
            mash::MashVector3 m = b.origin - a.center;
            f32 d = m.Dot(b.dir);
            f32 c = m.Dot(m) - a.radius * a.radius;

            //exit if rays origin is outside sphere and pointing
            //away from the sphere
            if (c > 0.0f && d > 0.0f)
                return false;

            f32 fDiscr = d*d - c;

            //does ray miss sphere?
            if (fDiscr < 0.0f)
                return false;

            //ray now intersects sphere

            t = -d - sqrt(fDiscr);
            //if t is negative than ray started inside sphere so clamp
            //t to zero
            if (t < 0.0f)
                t = 0.0f;

            return true;
        }

        bool Ray_Triangle(const MashVector3 &pointA, const MashVector3 &pointB, const MashVector3 &pointC, const MashRay &ray)
        {
            const f32 fEpsilon = 0.00001f;
            MashVector3 ab = pointB - pointA;
            MashVector3 ac = pointC - pointA;
            MashVector3 qp = -ray.dir;

            MashVector3 n = ab.Cross(ac);

            f32 d = qp.Dot(n);
            if (d < fEpsilon)//ray points away
                return false;

            MashVector3 ap = ray.origin - pointA;
            f32 fap = ap.Dot(n);
            if (fap < 0.0f)
                return false;

            MashVector3 e = qp.Cross(ap);
            f32 v = ac.Dot(e);
            if ((v < 0.0f) || (v > d))
                return false;
            f32 w = -(ab.Dot(e));
            if ((w < 0.0f) || ((v+w) > d))
                return false;

            return true;
        }

        bool Ray_Triangle(const MashTriangle &a, const MashRay &b)
        {
            return Ray_Triangle(a.pointA, a.pointB, a.pointC, b);
        }

        bool Ray_Triangle(const MashVector3 &pointA, const MashVector3 &pointB, const MashVector3 &pointC, const MashRay &ray, 
                f32 &u, f32 &v, f32 &w, f32 &t)
        {
            const f32 fEpsilon = 0.00001f;
            MashVector3 ab = pointB - pointA;
            MashVector3 ac = pointC - pointA;
            MashVector3 qp = -ray.dir;

            MashVector3 n = ab.Cross(ac);
            f32 d = qp.Dot(n);
            if ((d < fEpsilon))//ray points away
                return false;

            MashVector3 ap = ray.origin - pointA;
            t = ap.Dot(n);
            if (t < 0.0f)
                return false;

            MashVector3 e = qp.Cross(ap);
            v = ac.Dot(e);
            if ((v < 0.0f) || (v > d))
                return false;
            w = -(ab.Dot(e));
            if ((w < 0.0f) || ((v+w) > d))
                return false;

            f32 ood = 1.0f / d;
            t *= ood;
            v *= ood;
            w *= ood;
            u = 1.0f - v - w;

            return true;
        }

        bool Ray_Triangle(const MashTriangle &a, const MashRay &b, f32 &u, f32 &v, f32 &w, f32 &t)
        {
            return Ray_Triangle(a.pointA, a.pointB, a.pointC, b, u, v, w, t);
        }

        bool AABB_OBB(const MashAABB &a, const MashOBB &b)
        {
            MashOBB newObb;
            newObb.center = (a.max + a.min) * 0.5f;
            newObb.halfWidth = (a.max - a.min) * 0.5f;
            newObb.localAxis[0] = MashVector3(1,0,0);//right
            newObb.localAxis[1] = MashVector3(0,1,0);//up
            newObb.localAxis[2] = MashVector3(0,0,1);//forward

            return OBB_OBB(newObb, b);
        }

        bool OBB_OBB(const MashOBB &a, const MashOBB &b)
        {
            const f32 fEpsilon = 0.00001f;
            f32 ra, rb;
            mash::MashMatrix4 r, absR;

            //compute rotation matrix expressing b in a's coordinate frame
            for(int32 i = 0; i < 3; ++i)
                for(int32 j = 0; j < 3; ++j)
                    r.m[i][j] = a.localAxis[i].Dot(b.localAxis[j]);

            //translation vector
            mash::MashVector3 t = b.center - a.center;

            //bring translation into a's coordinate frame
            t.x = t.Dot(a.localAxis[0]);
            t.y = t.Dot(a.localAxis[1]);
            t.z = t.Dot(a.localAxis[2]);

            for(int32 i = 0; i < 3; ++i)
                for(int32 j = 0; j < 3; ++j)
                    absR.m[i][j] = math::_abs<f32>(r.m[i][j]) + fEpsilon;

            for(int32 i = 0; i < 3; ++i)
            {
                ra = a.halfWidth.v[i];
                rb = b.halfWidth.v[0] * absR.m[i][0] + 
                    b.halfWidth.v[1] * absR.m[i][1] +
                    b.halfWidth.v[2] * absR.m[i][2];

                if (math::_abs<f32>(t.v[i]) > ra + rb)
                    return false;
            }

            for(int32 i = 0; i < 3; ++i)
            {
                ra = a.halfWidth.v[0] * absR.m[0][i] + 
                    a.halfWidth.v[1] * absR.m[1][i] + 
                    a.halfWidth.v[2] * absR.m[2][i];
                rb = b.halfWidth.v[i];

                if (math::_abs<f32>(t.v[0] * r.m[0][i] + 
                    t.v[1] * r.m[1][i] +
                    t.v[2] * r.m[2][i]) > ra + rb)
                    return false;
            }

            ra = a.halfWidth.v[1] * absR.m[2][0] + a.halfWidth.v[2] * absR.m[1][0];
            rb = b.halfWidth.v[1] * absR.m[0][2] + b.halfWidth.v[2] * absR.m[0][1];

            if (math::_abs<f32>(t.v[2] * r.m[1][0] - t.v[1] * r.m[2][0]) > ra + rb)
                return false;

            ra = a.halfWidth.v[1] * absR.m[2][1] + a.halfWidth.v[2] * absR.m[1][1];
            rb = b.halfWidth.v[0] * absR.m[0][2] + b.halfWidth.v[2] * absR.m[0][0];

            if (math::_abs<f32>(t.v[2] * r.m[1][1] - t.v[1] * r.m[2][1]) > ra + rb)
                return false;

            ra = a.halfWidth.v[1] * absR.m[2][2] + a.halfWidth.v[2] * absR.m[1][2];
            rb = b.halfWidth.v[0] * absR.m[0][1] + b.halfWidth.v[1] * absR.m[0][0];

            if (math::_abs<f32>(t.v[2] * r.m[1][2] - t.v[1] * r.m[2][2]) > ra + rb)
                return false;

            ra = a.halfWidth.v[0] * absR.m[2][0] + a.halfWidth.v[2] * absR.m[0][0];
            rb = b.halfWidth.v[1] * absR.m[1][2] + b.halfWidth.v[2] * absR.m[1][1];

            if (math::_abs<f32>(t.v[0] * r.m[2][0] - t.v[2] * r.m[0][0]) > ra + rb)
                return false;

            ra = a.halfWidth.v[0] * absR.m[2][1] + a.halfWidth.v[2] * absR.m[0][1];
            rb = b.halfWidth.v[0] * absR.m[1][2] + b.halfWidth.v[2] * absR.m[1][0];

            if (math::_abs<f32>(t.v[0] * r.m[2][1] - t.v[2] * r.m[0][1]) > ra + rb)
                return false;

            ra = a.halfWidth.v[0] * absR.m[2][2] + a.halfWidth.v[2] * absR.m[0][2];
            rb = b.halfWidth.v[0] * absR.m[1][1] + b.halfWidth.v[1] * absR.m[1][0];

            if (math::_abs<f32>(t.v[0] * r.m[2][2] - t.v[2] * r.m[0][2]) > ra + rb)
                return false;

            ra = a.halfWidth.v[0] * absR.m[1][0] + a.halfWidth.v[1] * absR.m[0][0];
            rb = b.halfWidth.v[1] * absR.m[2][2] + b.halfWidth.v[2] * absR.m[2][1];

            if (math::_abs<f32>(t.v[1] * r.m[0][0] - t.v[0] * r.m[1][0]) > ra + rb)
                return false;

            ra = a.halfWidth.v[0] * absR.m[1][1] + a.halfWidth.v[1] * absR.m[0][1];
            rb = b.halfWidth.v[0] * absR.m[2][2] + b.halfWidth.v[2] * absR.m[2][0];

            if (math::_abs<f32>(t.v[1] * r.m[0][1] - t.v[0] * r.m[1][1]) > ra + rb)
                return false;

            ra = a.halfWidth.v[0] * absR.m[1][2] + a.halfWidth.v[1] * absR.m[0][2];
            rb = b.halfWidth.v[0] * absR.m[2][1] + b.halfWidth.v[1] * absR.m[2][0];

            if (math::_abs<f32>(t.v[1] * r.m[0][2] - t.v[0] * r.m[1][2]) > ra + rb)
                return false;

            return true;
        }
    }
}