//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashTransformState.h"
#include "MashMatrix4.h"

namespace mash
{
	MashTransformState::MashTransformState():translation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), orientation(){}

	MashTransformState::MashTransformState(const mash::MashVector3 &t, 
		const mash::MashVector3 &s, 
		const mash::MashQuaternion &q):translation(t), scale(s), orientation(q){}

	mash::MashVector3 MashTransformState::Transform(const mash::MashVector3 &v)const
	{
		mash::MashVector3 t = orientation.TransformVector(v);
		t *= scale;
		t += translation;
		return t;
	}

	mash::MashVector3 MashTransformState::TransformRotation(const mash::MashVector3 &v)const
	{
		mash::MashVector3 t = orientation.TransformVector(v);
		t *= scale;
		return t;
	}

	mash::MashVector3 MashTransformState::TransformInverse(const mash::MashVector3 &v)const
	{
		mash::MashVector3 t = translation - v;
		t *= -scale;
		mash::MashQuaternion qt(orientation);
		t = qt.Conjugate().TransformVector(t);
		return t;
	}

	mash::MashVector3 MashTransformState::TransformRotationInverse(const mash::MashVector3 &v)const
	{
		mash::MashQuaternion qt(orientation);
		mash::MashVector3 t =  qt.Conjugate().TransformVector(v);
		t *= scale;
		return t;
	}

	mash::MashMatrix4 MashTransformState::ToMatrix()const
	{
        return MashMatrix4(orientation, translation, scale);
	}
}