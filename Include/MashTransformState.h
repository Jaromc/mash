//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TRANSFORM_STATE_H_
#define _MASH_TRANSFORM_STATE_H_

#include "MashVector3.h"
#include "MashQuaternion.h"

namespace mash
{
	class MashMatrix4;

    /*!
        Transform states store a nodes positional data.
        This is mainly used for interpolation between states.
    */
	class MashTransformState
	{
	public:
		MashVector3 translation;
		MashVector3 scale;
		MashQuaternion orientation;

		MashTransformState();
		MashTransformState(const MashVector3 &t, const MashVector3 &s, const MashQuaternion &q);

        //! Transforms a vector by this state.
        /*!
            \param v Vector to transform.
            \return Transformed vector.
        */
		MashVector3 Transform(const MashVector3 &v)const;
        
        //! Transforms a vector by rotation only.
        /*!
            \param v Vector to transform.
            \return Transformed vector.
        */
		MashVector3 TransformRotation(const MashVector3 &v)const;

        //! Transforms a vector by the inverse of this state.
        /*!
            \param v Vector to transform.
            \return Transformed vector.
        */
		MashVector3 TransformInverse(const MashVector3 &v)const;
        
        //! Transforms a vector by the inverse of rotation only.
        /*!
            \param v Vector to transform.
            \return Transformed vector.
        */
		MashVector3 TransformRotationInverse(const MashVector3 &v)const;

        //! Converts this state into matrix form.
		MashMatrix4 ToMatrix()const;
	};


}

#endif