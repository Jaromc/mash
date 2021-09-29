//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MATH_HELPER_H_
#define _MASH_MATH_HELPER_H_

#include "MashDataTypes.h"
#include <limits.h>
#include <float.h>
#include <math.h>
#include <cstdlib>

namespace mash
{
	class MashVector3;
    
    namespace math
    {
        //Constants
        
        inline f32 Pi()
        {
            return 3.1415926f;
        }
        
        inline f32 TwoPi()
        {
            return Pi() * 2.0f;
        }
        
        inline f32 MaxFloat()
        {
            return FLT_MAX;
        }
        
        inline f32 MinFloat()
        {
            return -FLT_MAX;
        }
        
        inline int8 MaxInt8()
        {
            return SCHAR_MAX;
        }
        
        inline int8 MinInt8()
        {
            return SCHAR_MIN;
        }
        
        inline uint8 MaxUInt8()
        {
            return UCHAR_MAX;
        }
        
        inline int16 MaxInt16()
        {
            return SHRT_MAX;
        }
        
        inline int16 MinInt16()
        {
            return SHRT_MIN;
        }
        
        inline uint16 MaxUInt16()
        {
            return USHRT_MAX;
        }
        
        inline uint32 MaxUInt32()
        {
            return 0xFFFFFFFF;
        }
        
        inline int32 MaxInt32()
        {
            return INT_MAX;
        }
        
        inline int32 MinInt32()
        {
            return INT_MIN;
        }
        
        inline int64 MaxInt64()
        {
            return LLONG_MAX;
        }
        
        inline int64 MinInt64()
        {
            return LLONG_MIN;
        }
        
        inline uint64 MaxUInt64()
        {
            return ULLONG_MAX;
        }

        //! Returns the max of two numbers.
        template<class T>
        inline T Max(T a, T b)
        {
            return a > b ? a : b;
        }

        //! Returns the max of three numbers.
        template<class T>
        inline T Max(T a, T b, T c)
        {
            f32 r1 = a > b ? a : b;
            return r1 > c ? r1 : c;
        }

        //! Returns the min of two numbers.
        template<class T>
        inline T Min(T a, T b)
        {
            return a < b ? a : b;
        }

        //! Returns the min of three numbers.
        template<class T>
        inline T Min(T a, T b, T c)
        {
            f32 r1 = a < b ? a : b;
            return r1 < c ? r1 : c;
        }

        //! Returns a vector3 that is created from the minimum of two vectors.
        /*!
            It is assumed that all arrays have 3 elements.

            \param a First vector to test.
            \param b Second vector to test.
            \param out The result will be stored here.
        */
        template<class T>
        inline void MinVec3(const T *a, const T *b, T *out)
        {
            out[0] = a[0] < b[0] ? a[0] : b[0];
            out[1] = a[1] < b[1] ? a[1] : b[1];
            out[2] = a[2] < b[2] ? a[2] : b[2];
        }

        //! Returns a vector3 that is created from the maximum of two vectors.
        /*!
            It is assumed that all arrays have 3 elements.

            \param a First vector to test.
            \param b Second vector to test.
            \param out The result will be stored here.
         */
        template<class T>
        inline void MaxVec3(const T *a, const T *b, T *out)
        {
            out[0] = a[0] > b[0] ? a[0] : b[0];
            out[1] = a[1] > b[1] ? a[1] : b[1];
            out[2] = a[2] > b[2] ? a[2] : b[2];
        }

        //! Clamps a value between min and max.
        /*!
            \param minVal Min value.
            \param maxVal Max value.
            \param clampVal Value to clamp.
            \return Clamped value.
        */
        template<class T>
        inline T Clamp(const T minVal, const T maxVal, const T clampVal)
        {	
            if (clampVal < minVal)
            {
                return minVal;
            }

            if (clampVal > maxVal)
            {
                return maxVal;
            }

            return clampVal;
        }

        //! Return a random int32 between min and max.
        inline int32 RandomInt(int32 min, int32 max)
        {
            return  (rand()%(max-min+1)+min);
        }

        //! Returns a random f32.
        inline f32 RandomFloat()
        {
            return ((f32)rand()/(RAND_MAX+1.f));
        }
        
        //! Returns a random f32 between min and max.
        inline f32 RandomFloat(f32 min, f32 max)
        {
            return (min + RandomFloat() * (max-min));
        }

        //! Returns a f32 between -1 and 1
        inline f32 RandomBinominal()
        {
            return (RandomFloat() - RandomFloat());
        }

        //! Convert degrees to radians.
        inline f32 DegsToRads(f32 fDeg)
        {
          return fDeg * 0.0174532925f;
        }
        
        //! Convert radians to degrees.
        inline f32 RadsToDegs(f32 fRad)
        {
          return fRad * 57.2957795f;
        }

        //! Keeps a number within a range.
        /*!
            Note this is a quick method of wrapping. It doesn't calculate the difference
            past the boundaries and add it to the wrap value.
         
            \param min Min wrap value.
            \param max Max wrap value.
            \param value Value to test.
            \return Wrapped value.
        */
        template<class T>
        inline T LoopNumberQuick(T min, T max, T value)
        {
            if (value > max)
                return min;
            if (value < min)
                return max;

            return value;
        }

        //! Finds the smallest angle needed to turn from orig to target and clamps between -pi and pi.
        inline f32 MapToRange(f32 target, f32 orig)
        {
            f32 f = fmodf(target - orig, orig);
            if (f < -mash::math::Pi())
                return (f + mash::math::TwoPi());
            else if (f >= mash::math::Pi())
                return (f - mash::math::TwoPi());

            return f;
        }

        //! Lerps between two numbers.
        /*!
            \param start Start value.
            \param target End value.
            \param alpha Lerp amount.
            \return Final lerp amount.
        */
        inline f32 Lerp(f32 start, f32 target, f32 alpha)
        {
            return (start + (alpha * (target - start)));
        }
        
        //! Float compare method within an epsilon value.
        inline bool FloatEqualTo(f32 a, f32 b, f32 ep = 0.00001f)
        {
            return (fabs(b - a) <= ep);
        }

        //! Sets a number to zero unless it's greater than a threshold.
        /*!
            \param threshold Lowest min value.
            \param valueToFilter Value to filter.
            \return Filtered value.
        */
        template<class T>
        inline T FilterIntValue(T threshold, T valueToFilter)
        {
            if (abs(valueToFilter) < threshold)
                return 0;
            else
                return valueToFilter;
        }
        
        //! Sets a number to zero unless it's greater than a threshold.
        /*!
            \param threshold Lowest min value.
            \param valueToFilter Value to filter.
            \return Filtered value.
         */
        inline f32 FilterFloatValue(f32 threshold, f32 valueToFilter)
        {
            if (fabs(valueToFilter) < threshold)
                return 0.0f;
            else
                return valueToFilter;
        }

        //! A templated abs function.
        template<class T>
        inline T _abs(const T &a)
        {
            return a < (T)0 ? -a : a;
        }

        //! A templated is negative function.
        template<class T>
        inline bool IsNegative(const T &a)
        {
            return a < (T)0 ? true : false;
        }

        //! A templated function to swap two values.
        template<class T>
        inline void Swap(T &a, T &b)
        {
            T temp = b;
            b = a;
            a = temp;
        }
        
        //! Convertes an int32 colour to a f32 colour.
        /*
            \param col Must be in the range of 0 - 255.
            \return Float colour in the range of 0.0f - 1.0f;
        */
        inline f32 IntColourToFloat(uint32 col)
        {
            return (f32)col / 255.0f;
        }

        //! Converts a f32 colour into an int32 colour.
        /*!
            \param col Float colour in the range of 0.0f - 1.0f;
            \return Int colour in the range of 0 - 255.
        */
        inline uint32 FloatColourToInt(f32 col)
        {
            return (uint32)(col * 255.0f);
        }

        //! A safe version of acos that first clamps the value within -1.0f - 1.0f.
        inline f32 Safeacos(f32 f)
        {
            return acos(math::Clamp<f32>(-1.0f, 1.0f, f));
        }

        //! A safe version of asin that first clamps the value within -1.0f - 1.0f.
        inline f32 Safeasin(f32 f)
        {
            return asin(math::Clamp<f32>(-1.0f, 1.0f, f));
        }

        //! Computes the face normal of 3 triangle points. The returned value is normalized.
        mash::MashVector3 ComputeFaceNormal(const mash::MashVector3 &a,
            const mash::MashVector3 &b,
            const mash::MashVector3 &c);
    }
}

#endif