//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_RECTANGLE2_H_
#define _MASH_RECTANGLE2_H_

#include "MashCompileSettings.h"
#include "MashEnum.h"

namespace mash
{
	class MashVector2;

	/*!
        Defines a 2D rectangle.
    */
	class _MASH_EXPORT MashRectangle2
	{
	public:
		f32 left;
		f32 top;
		f32 right;			
		f32 bottom;

	public:
		//! Constructor.
		MashRectangle2();

		//! Constructor.
		MashRectangle2(f32 _left, f32 _top, f32 _right, f32 _bottom);
		//! Copy constructor.
		MashRectangle2(const MashRectangle2 &copy);
        
        //! Equals to.
		bool operator==(const MashRectangle2 &other)const;

        //! Sets this rect to the max inverse bounds.
		void MaxInvBounds();
        
        //! Sets this rect to the max bounds.
		void MaxBounds();
        
        //! Sets all elements to zero.
		void Zero();
        
        //! Returns true if all elements are zero.
		bool IsZero()const;

		//! Returns 4 points of this rect.
        /*!
            Points start from left bottom and wind clockwise.
         
            \param points An array of 4 vector2s.
        */
		void GetPointsAsSquare(MashVector2 *points)const;
        
        //! Returns 6 points that represent two triangles.
        /*!
            \param points An array of 6 vector2s.
        */
		void GetPointsAsTris(MashVector2 *points)const;

        //! Intersection test in screen coords.
		/*
            This function operates in screen coords. Top left (0,0) Bottom right (height, width).
         
			\param point Screen space points.
            \return True if intersecting, false otherwise.
		*/
		bool IntersectsGUI(const MashVector2 &point)const;
        
        //! Expands this rect if needed by the point.
        /*!
            This function operates in screen coords. Top left (0,0) Bottom right (height, width).
         
            \param point Point to expand this rect by.
        */
		void AddGUI(const MashVector2 &point);
        
        //! Merges with another rect.
        /*!
            This function operates in screen coords. Top left (0,0) Bottom right (height, width).
         
            \param rect Rect to merge.
        */
		void MergeGUI(const MashRectangle2 &rect);

        //! Tests THIS rect against another rect.
        /*!
            This function operates in screen coords. Top left (0,0) Bottom right (height, width).
         
            \param other Rect to test against.
        */
		eCULL ClassifyGUI(const MashRectangle2 &other)const;
        
        //! Clips THIS rect against another.
        /*!
            This function operates in screen coords. Top left (0,0) Bottom right (height, width).
         
            \param other Rect to test against.
        */
		eCULL ClipGUI(const MashRectangle2 &other);
	};
}

#endif