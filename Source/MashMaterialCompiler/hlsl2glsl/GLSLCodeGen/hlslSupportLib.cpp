//
//Copyright (C) 2005-2006  ATI Research, Inc.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of ATI Research, Inc. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//

//=================================================================================================================================
//
// ATI Research, Inc.
//
// Implementation of support library to generate GLSL functions to support HLSL functions that don't map to built-ins
//=================================================================================================================================

#include <map>

//=================================================================================================================================
//
//          Includes / defines / typedefs / static variable initialization block
//
//=================================================================================================================================
#include "hlslSupportLib.h"

typedef std::map<TOperator,std::string> CodeMap;
static CodeMap *hlslSupportLib = 0;

//=================================================================================================================================
//
//          Public Functions
//
//=================================================================================================================================

//=========================================================================================================
/// Initialize the library by loading the necessary functions
//=========================================================================================================
void initializeHLSLSupportLibrary() 
{
   assert( hlslSupportLib == 0);

   hlslSupportLib = new CodeMap;

   // Initialize GLSL code for the op codes that require support helper functions

   hlslSupportLib->insert( CodeMap::value_type( EOpNull, ""));

   hlslSupportLib->insert( CodeMap::value_type( EOpAbs,
      "mat2 xlat_lib_abs(mat2 m) {\n"
      "  return mat2( abs(m[0]), abs(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_abs(mat3 m) {\n"
      "  return mat3( abs(m[0]), abs(m[1]), abs(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_abs(mat4 m) {\n"
      "  return mat4( abs(m[0]), abs(m[1]), abs(m[2]), abs(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpAcos,
      "mat2 xlat_lib_acos(mat2 m) {\n"
      "  return mat2( acos(m[0]), acos(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_acos(mat3 m) {\n"
      "  return mat3( acos(m[0]), acos(m[1]), acos(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_acos(mat4 m) {\n"
      "  return mat4( acos(m[0]), acos(m[1]), acos(m[2]), acos(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpCos,
      "mat2 xlat_lib_cos(mat2 m) {\n"
      "  return mat2( cos(m[0]), cos(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_cos(mat3 m) {\n"
      "  return mat3( cos(m[0]), cos(m[1]), cos(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_cos(mat4 m) {\n"
      "  return mat4( cos(m[0]), cos(m[1]), cos(m[2]), cos(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpAsin,
      "mat2 xlat_lib_asin(mat2 m) {\n"
      "  return mat2( asin(m[0]), asin(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_asin(mat3 m) {\n"
      "  return mat3( asin(m[0]), asin(m[1]), asin(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_asin(mat4 m) {\n"
      "  return mat4( asin(m[0]), asin(m[1]), asin(m[2]), asin(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpSin,
      "mat2 xlat_lib_sin(mat2 m) {\n"
      "  return mat2( sin(m[0]), sin(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_sin(mat3 m) {\n"
      "  return mat3( sin(m[0]), sin(m[1]), sin(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_sin(mat4 m) {\n"
      "  return mat4( sin(m[0]), sin(m[1]), sin(m[2]), sin(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpDPdx,
      "mat2 xlat_lib_dFdx(mat2 m) {\n"
      "  return mat2( dFdx(m[0]), dFdx(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_dFdx(mat3 m) {\n"
      "  return mat3( dFdx(m[0]), dFdx(m[1]), dFdx(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_dFdx(mat4 m) {\n"
      "  return mat4( dFdx(m[0]), dFdx(m[1]), dFdx(m[2]), dFdx(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpDPdy,
      "mat2 xlat_lib_dFdy(mat2 m) {\n"
      "  return mat2( dFdy(m[0]), dFdy(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_dFdy(mat3 m) {\n"
      "  return mat3( dFdy(m[0]), dFdy(m[1]), dFdy(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_dFdy(mat4 m) {\n"
      "  return mat4( dFdy(m[0]), dFdy(m[1]), dFdy(m[2]), dFdy(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpExp,
      "mat2 xlat_lib_exp(mat2 m) {\n"
      "  return mat2( exp(m[0]), exp(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_exp(mat3 m) {\n"
      "  return mat3( exp(m[0]), exp(m[1]), exp(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_exp(mat4 m) {\n"
      "  return mat4( exp(m[0]), exp(m[1]), exp(m[2]), exp(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpExp2,
      "mat2 xlat_lib_exp2(mat2 m) {\n"
      "  return mat2( exp2(m[0]), exp2(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_exp2(mat3 m) {\n"
      "  return mat3( exp2(m[0]), exp2(m[1]), exp2(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_exp2(mat4 m) {\n"
      "  return mat4( exp2(m[0]), exp2(m[1]), exp2(m[2]), exp2(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpLog,
      "mat2 xlat_lib_log(mat2 m) {\n"
      "  return mat2( log(m[0]), log(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_log(mat3 m) {\n"
      "  return mat3( log(m[0]), log(m[1]), log(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_log(mat4 m) {\n"
      "  return mat4( log(m[0]), log(m[1]), log(m[2]), log(m[3]));\n"
      "}\n\n")
      );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpLog2,
      "mat2 xlat_lib_log2(mat2 m) {\n"
      "  return mat2( log2(m[0]), log2(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_log2(mat3 m) {\n"
      "  return mat3( log2(m[0]), log2(m[1]), log2(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_log2(mat4 m) {\n"
      "  return mat4( log2(m[0]), log2(m[1]), log2(m[2]), log2(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpTan,
      "mat2 xlat_lib_tan(mat2 m) {\n"
      "  return mat2( tan(m[0]), tan(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_tan(mat3 m) {\n"
      "  return mat3( tan(m[0]), tan(m[1]), tan(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_tan(mat4 m) {\n"
      "  return mat4( tan(m[0]), tan(m[1]), tan(m[2]), tan(m[3]));\n"
      "}\n\n")
      );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpAtan,
      "mat2 xlat_lib_atan(mat2 m) {\n"
      "  return mat2( atan(m[0]), atan(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_atan(mat3 m) {\n"
      "  return mat3( atan(m[0]), atan(m[1]), atan(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_atan(mat4 m) {\n"
      "  return mat4( atan(m[0]), atan(m[1]), atan(m[2]), atan(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpDegrees,
      "mat2 xlat_lib_degrees(mat2 m) {\n"
      "  return mat2( degrees(m[0]), degrees(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_degrees(mat3 m) {\n"
      "  return mat3( degrees(m[0]), degrees(m[1]), degrees(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_degrees(mat4 m) {\n"
      "  return mat4( degrees(m[0]), degrees(m[1]), degrees(m[2]), degrees(m[3]));\n"
      "}\n\n")
      );

    hlslSupportLib->insert( CodeMap::value_type( EOpRadians,
      "mat2 xlat_lib_radians(mat2 m) {\n"
      "  return mat2( radians(m[0]), radians(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_radians(mat3 m) {\n"
      "  return mat3( radians(m[0]), radians(m[1]), radians(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_radians(mat4 m) {\n"
      "  return mat4( radians(m[0]), radians(m[1]), radians(m[2]), radians(m[3]));\n"
      "}\n\n")
      );

    hlslSupportLib->insert( CodeMap::value_type( EOpSqrt,
      "mat2 xlat_lib_sqrt(mat2 m) {\n"
      "  return mat2( sqrt(m[0]), sqrt(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_sqrt(mat3 m) {\n"
      "  return mat3( sqrt(m[0]), sqrt(m[1]), sqrt(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_sqrt(mat4 m) {\n"
      "  return mat4( sqrt(m[0]), sqrt(m[1]), sqrt(m[2]), sqrt(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpInverseSqrt,
      "mat2 xlat_lib_inversesqrt(mat2 m) {\n"
      "  return mat2( inversesqrt(m[0]), inversesqrt(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_inversesqrt(mat3 m) {\n"
      "  return mat3( inversesqrt(m[0]), inversesqrt(m[1]), inversesqrt(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_inversesqrt(mat4 m) {\n"
      "  return mat4( inversesqrt(m[0]), inversesqrt(m[1]), inversesqrt(m[2]), inversesqrt(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpFloor,
      "mat2 xlat_lib_floor(mat2 m) {\n"
      "  return mat2( floor(m[0]), floor(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_floor(mat3 m) {\n"
      "  return mat3( floor(m[0]), floor(m[1]), floor(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_floor(mat4 m) {\n"
      "  return mat4( floor(m[0]), floor(m[1]), floor(m[2]), floor(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpSign,
      "mat2 xlat_lib_sign(mat2 m) {\n"
      "  return mat2( sign(m[0]), sign(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_sign(mat3 m) {\n"
      "  return mat3( sign(m[0]), sign(m[1]), sign(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_sign(mat4 m) {\n"
      "  return mat4( sign(m[0]), sign(m[1]), sign(m[2]), sign(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpCeil,
      "mat2 xlat_lib_ceil(mat2 m) {\n"
      "  return mat2( ceil(m[0]), ceil(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_ceil(mat3 m) {\n"
      "  return mat3( ceil(m[0]), ceil(m[1]), ceil(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_ceil(mat4 m) {\n"
      "  return mat4( ceil(m[0]), ceil(m[1]), ceil(m[2]), ceil(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpFract,
      "mat2 xlat_lib_fract(mat2 m) {\n"
      "  return mat2( fract(m[0]), fract(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_fract(mat3 m) {\n"
      "  return mat3( fract(m[0]), fract(m[1]), fract(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_fract(mat4 m) {\n"
      "  return mat4( fract(m[0]), fract(m[1]), fract(m[2]), fract(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpFwidth,
      "mat2 xlat_lib_fwidth(mat2 m) {\n"
      "  return mat2( fwidth(m[0]), fwidth(m[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_fwidth(mat3 m) {\n"
      "  return mat3( fwidth(m[0]), fwidth(m[1]), fwidth(m[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_fwidth(mat4 m) {\n"
      "  return mat4( fwidth(m[0]), fwidth(m[1]), fwidth(m[2]), fwidth(m[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpPow,
      "mat2 xlat_lib_pow(mat2 m, mat2 y) {\n"
      "  return mat2( pow(m[0],y[0]), pow(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_pow(mat3 m, mat3 y) {\n"
      "  return mat3( pow(m[0],y[0]), pow(m[1],y[1]), pow(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_pow(mat4 m, mat4 y) {\n"
      "  return mat4( pow(m[0],y[0]), pow(m[1],y[1]), pow(m[2],y[2]), pow(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpAtan2,
      "mat2 xlat_lib_atan2(mat2 m, mat2 y) {\n"
      "  return mat2( atan(m[0],y[0]), atan(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_atan2(mat3 m, mat3 y) {\n"
      "  return mat3( atan(m[0],y[0]), atan(m[1],y[1]), atan(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_atan2(mat4 m, mat4 y) {\n"
      "  return mat4( atan(m[0],y[0]), atan(m[1],y[1]), atan(m[2],y[2]), atan(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpMin,
      "mat2 xlat_lib_min(mat2 m, mat2 y) {\n"
      "  return mat2( min(m[0],y[0]), min(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_min(mat3 m, mat3 y) {\n"
      "  return mat3( min(m[0],y[0]), min(m[1],y[1]), min(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_min(mat4 m, mat4 y) {\n"
      "  return mat4( min(m[0],y[0]), min(m[1],y[1]), min(m[2],y[2]), min(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpMax,
      "mat2 xlat_lib_max(mat2 m, mat2 y) {\n"
      "  return mat2( max(m[0],y[0]), max(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_max(mat3 m, mat3 y) {\n"
      "  return mat3( max(m[0],y[0]), max(m[1],y[1]), max(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_max(mat4 m, mat4 y) {\n"
      "  return mat4( max(m[0],y[0]), max(m[1],y[1]), max(m[2],y[2]), max(m[3],y[3]));\n"
      "}\n\n")
      );

   hlslSupportLib->insert( CodeMap::value_type( EOpTranspose,
        "mat2 xlat_lib_transpose(mat2 m) {\n"
        "  return mat2( m[0][0], m[1][0], m[0][1], m[1][1]);\n"
        "}\n\n"
        "mat3 xlat_lib_transpose(mat3 m) {\n"
        "  return mat3( m[0][0], m[1][0], m[2][0],\n"
        "               m[0][1], m[1][1], m[2][1],\n"
        "               m[0][2], m[1][2], m[2][2]);\n"
        "}\n\n"
        "mat4 xlat_lib_transpose(mat4 m) {\n"
        "  return mat4( m[0][0], m[1][0], m[2][0], m[3][0],\n"
        "               m[0][1], m[1][1], m[2][1], m[3][1],\n"
        "               m[0][2], m[1][2], m[2][2], m[3][2],\n"
        "               m[0][3], m[1][3], m[2][3], m[3][3]);\n"
        "}\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpConstructMat2FromMat,
        "mat2 xlat_lib_constructMat2( mat3 m) {\n"
        "  return mat2( vec2( m[0]), vec2( m[1]));\n"
        "}\n\n"
        "mat2 xlat_lib_constructMat2( mat4 m) {\n"
        "  return mat2( vec2( m[0]), vec2( m[1]));\n"
        "}\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpConstructMat3FromMat,
        "mat3 xlat_lib_constructMat3( mat4 m) {\n"
        "  return mat3( vec3( m[0]), vec3( m[1]), vec3( m[2]));\n"
        "}\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpDeterminant,
        "float xlat_lib_determinant( mat2 m) {\n"
        "    return m[0][0]*m[1][1] - m[0][1]*m[1][0];\n"
        "}\n\n"
        "float xlat_lib_determinant( mat3 m) {\n"
        "    vec3 temp;\n"    
        "    temp.x = m[1][1]*m[2][2] - m[1][2]*m[2][1];\n"
        "    temp.y = - (m[0][1]*m[2][2] - m[0][2]*m[2][1]);\n"
        "    temp.z = m[0][1]*m[1][2] - m[0][2]*m[1][1];\n"
        "    return dot( m[0], temp);\n"
        "}\n\n"
        "float xlat_lib_determinant( mat4 m) {\n"
        "    vec4 temp;\n"
        "    temp.x = xlat_lib_determinant( mat3( m[1].yzw, m[2].yzw, m[3].yzw));\n"
        "    temp.y = -xlat_lib_determinant( mat3( m[0].yzw, m[2].yzw, m[3].yzw));\n"
        "    temp.z = xlat_lib_determinant( mat3( m[0].yzw, m[1].yzw, m[3].yzw));\n"
        "    temp.w = -xlat_lib_determinant( mat3( m[0].yzw, m[1].yzw, m[2].yzw));\n"    
        "    return dot( m[0], temp);\n"
        "}\n")
        );

  // hlslSupportLib->insert( CodeMap::value_type( EOpNormalize,
  //      "vec2 xlat_lib_normalize( vec2 v) {\n"
  //      "	float l = length(v);\n"
		//"	if (l > 0.001)\n"
		//"	{\n"
		//"		return vec2(v.x / l, v.y / l);\n"
		//"	}\n"
		//"	return vec2(0.0, 0.0);\n"
  //      "}\n\n"
  //      "vec3 xlat_lib_normalize( vec3 v) {\n"
  //      "	float l = length(v);\n"
		//"	if (l > 0.001)\n"
		//"	{\n"
		//"		return vec3(v.x / l, v.y / l, v.z / l);\n"
		//"	}\n"
		//"	return vec3(0.0, 0.0, 0.0);\n"
  //      "}\n\n"
  //      "vec4 xlat_lib_normalize( vec4 v) {\n"
  //      "	float l = length(v);\n"
		//"	if (l > 0.001)\n"
		//"	{\n"
		//"		return vec4(v.x / l, v.y / l, v.z / l, v.w / l);\n"
		//"	}\n"
		//"	return vec4(0.0, 0.0, 0.0, 0.0);\n"
  //      "}\n\n")
  //      );

   hlslSupportLib->insert( CodeMap::value_type( EOpSaturate,
        "float xlat_lib_saturate( float x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "vec2 xlat_lib_saturate( vec2 x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "vec3 xlat_lib_saturate( vec3 x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "vec4 xlat_lib_saturate( vec4 x) {\n"
        "  return clamp( x, 0.0, 1.0);\n"
        "}\n\n"
        "mat2 xlat_lib_saturate(mat2 m) {\n"
        "  return mat2( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0));\n"
        "}\n\n"
        "mat3 xlat_lib_saturate(mat3 m) {\n"
        "  return mat3( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0), clamp(m[2], 0.0, 1.0));\n"
        "}\n\n"
        "mat4 xlat_lib_saturate(mat4 m) {\n"
        "  return mat4( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0), clamp(m[2], 0.0, 1.0), clamp(m[3], 0.0, 1.0));\n"
        "}\n\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpModf,
        "float xlat_lib_modf( float x, out int ip) {\n"
        "  float ret = fract ( x );\n"
        "  ip = int (x - ret);\n"
        "  return ret;\n"
        "}\n\n"
        "float xlat_lib_modf( float x, out float ip) {\n"
        "  float ret = fract ( x );\n"
        "  ip = x - ret;\n"
        "  return ret;\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpLdexp,
        "float xlat_lib_ldexp( float x, float expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float2 xlat_lib_ldexp( vec2 x, vec2 expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float3 xlat_lib_ldexp( vec3 x, vec3 expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float4 xlat_lib_ldexp( vec4 x, vec4 expon) {\n"
        "  return x * exp2 ( expon );\n"
        "}\n\n"
        "float2x2 xlat_lib_ldexp( mat2 x, mat2 expon) {\n"
        "  return x * mat2 ( exp2 ( expon[0] ), exp2 ( expon[1] ) );\n"
        "}\n\n"
        "float3x3 xlat_lib_ldexp( mat3 x, mat3 expon) {\n"
        "  return x * mat3 ( exp2 ( expon[0] ), exp2 ( expon[1] ), exp2 ( expon[2] ) );\n"
        "}\n\n"
        "float4x4 xlat_lib_ldexp( mat4 x, mat4 expon) {\n"
        "  return x * mat4 ( exp2 ( expon[0] ), exp2 ( expon[1] ), exp2 ( expon[2] ), exp2 ( expon[3] ) );\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpSinCos,
        "void xlat_lib_sincos( float x, out float s, out float c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"
        "void xlat_lib_sincos( vec2 x, out vec2 s, out vec2 c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"        
        "void xlat_lib_sincos( vec3 x, out vec3 s, out vec3 c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"        
        "void xlat_lib_sincos( vec4 x, out vec4 s, out vec4 c) {\n"
        "  s = sin(x); \n"
        "  c = cos(x); \n"
        "}\n\n"        
        "void xlat_lib_sincos( mat2 x, out mat2 s, out mat2 c) {\n"
        "  s = mat2 ( sin ( x[0] ), sin ( x[1] ) ); \n"
        "  c = mat2 ( cos ( x[0] ), cos ( x[1] ) ); \n"
        "}\n\n"        
        "void xlat_lib_sincos( mat3 x, out mat3 s, out mat3 c) {\n"
        "  s = mat3 ( sin ( x[0] ), sin ( x[1] ), sin ( x[2] ) ); \n"
        "  c = mat3 ( cos ( x[0] ), cos ( x[1] ), cos ( x[2] ) ); \n"
        "}\n\n"        
        "void xlat_lib_sincos( mat4 x, out mat4 s, out mat4 c) {\n"
        "  s = mat4 ( sin ( x[0] ), sin ( x[1] ), sin ( x[2] ), sin ( x[3] ) ); \n"
        "  c = mat4 ( cos ( x[0] ), cos ( x[1] ), cos ( x[2] ), cos ( x[3] ) ); \n"
        "}\n\n" )
        );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpLog10,
        "float xlat_lib_log10( float x ) {\n"
        "  return log2 ( x ) / 3.32192809; \n"
        "}\n\n"
        "vec2 xlat_lib_log10( vec2 x ) {\n"
        "  return log2 ( x ) / vec2 ( 3.32192809 ); \n"
        "}\n\n"
        "vec3 xlat_lib_log10( vec3 x ) {\n"
        "  return log2 ( x ) / vec3 ( 3.32192809 ); \n"
        "}\n\n"
        "vec4 xlat_lib_log10( vec4 x ) {\n"
        "  return log2 ( x ) / vec4 ( 3.32192809 ); \n"
        "}\n\n"
        "mat2 xlat_lib_log10(mat2 m) {\n"
        "  return mat2( xlat_lib_log10(m[0]), xlat_lib_log10(m[1]));\n"
        "}\n\n"
        "mat3 xlat_lib_log10(mat3 m) {\n"
        "  return mat3( xlat_lib_log10(m[0]), xlat_lib_log10(m[1]), xlat_lib_log10(m[2]));\n"
        "}\n\n"
        "mat4 xlat_lib_log10(mat4 m) {\n"
        "  return mat4( xlat_lib_log10(m[0]), xlat_lib_log10(m[1]), xlat_lib_log10(m[2]), xlat_lib_log10(m[3]));\n"
        "}\n\n")
        );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpMix,
        "mat2 xlat_lib_mix( mat2 x, mat2 y, mat2 s ) {\n"
        "  return mat2 ( mix(x[0],y[0],s[0]), mix(x[1],y[1],s[1]) ); \n"
        "}\n\n"
        "mat3 xlat_lib_mix( mat3 x, mat3 y, mat3 s ) {\n"
        "  return mat3 ( mix(x[0],y[0],s[0]), mix(x[1],y[1],s[1]), mix(x[2],y[2],s[2]) ); \n"
        "}\n\n"
        "mat4 xlat_lib_mix( mat4 x, mat4 y, mat4 s ) {\n"
        "  return mat4 ( mix(x[0],y[0],s[0]), mix(x[1],y[1],s[1]), mix(x[2],y[2],s[2]), mix(x[3],y[3],s[3]) ); \n"
        "}\n\n")      
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpSmoothStep,
        "mat2 xlat_lib_smoothstep( mat2 x, mat2 y, mat2 s ) {\n"
        "  return mat2 ( smoothstep(x[0],y[0],s[0]), smoothstep(x[1],y[1],s[1]) ); \n"
        "}\n\n"
        "mat3 xlat_lib_smoothstep( mat3 x, mat3 y, mat3 s ) {\n"
        "  return mat3 ( smoothstep(x[0],y[0],s[0]), smoothstep(x[1],y[1],s[1]), smoothstep(x[2],y[2],s[2]) ); \n"
        "}\n\n"
        "mat4 xlat_lib_smoothstep( mat4 x, mat4 y, mat4 s ) {\n"
        "  return mat4 ( smoothstep(x[0],y[0],s[0]), smoothstep(x[1],y[1],s[1]), smoothstep(x[2],y[2],s[2]), smoothstep(x[3],y[3],s[3]) ); \n"
        "}\n\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpClamp,
        "mat2 xlat_lib_clamp( mat2 x, mat2 y, mat2 s ) {\n"
        "  return mat2 ( clamp(x[0],y[0],s[0]), clamp(x[1],y[1],s[1]) ); \n"
        "}\n\n"
        "mat3 xlat_lib_clamp( mat3 x, mat3 y, mat3 s ) {\n"
        "  return mat3 ( clamp(x[0],y[0],s[0]), clamp(x[1],y[1],s[1]), clamp(x[2],y[2],s[2]) ); \n"
        "}\n\n"
        "mat4 xlat_lib_clamp( mat4 x, mat4 y, mat4 s ) {\n"
        "  return mat4 ( clamp(x[0],y[0],s[0]), clamp(x[1],y[1],s[1]), clamp(x[2],y[2],s[2]), clamp(x[3],y[3],s[3]) ); \n"
        "}\n\n")
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpStep,
      "mat2 xlat_lib_step(mat2 m, mat2 y) {\n"
      "  return mat2( step(m[0],y[0]), step(m[1],y[1]));\n"
      "}\n\n"
      "mat3 xlat_lib_step(mat3 m, mat3 y) {\n"
      "  return mat3( step(m[0],y[0]), step(m[1],y[1]), step(m[2],y[2]));\n"
      "}\n\n"
      "mat4 xlat_lib_step(mat4 m, mat4 y) {\n"
      "  return mat4( step(m[0],y[0]), step(m[1],y[1]), step(m[2],y[2]), step(m[3],y[3]));\n"
      "}\n\n")
      );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpTex1DBias,
        "vec4 xlat_lib_tex1Dbias(sampler1D s, vec4 coord) {\n"
        "  return texture1D( s, coord.x, coord.w);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTex1DLod,
        "vec4 xlat_lib_tex1Dlod(sampler1D s, vec4 coord) {\n"
        "  return textureLod( s, coord.x, coord.w);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTex1DGrad,
        "#extension GL_ATI_shader_texture_lod : require\n"
        "vec4 xlat_lib_tex1Dgrad(sampler1D s, float coord, float ddx, float ddy) {\n"
        "  return texture1D_ATI( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpTex2DBias,
        "vec4 xlat_lib_tex2Dbias(sampler2D s, vec4 coord) {\n"
        "  return texture2D( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );
   
   hlslSupportLib->insert( CodeMap::value_type( EOpTex2DLod,
        "vec4 xlat_lib_tex2Dlod(sampler2D s, vec4 coord) {\n"
        "   return textureLod( s, coord.xy, coord.w);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTex2DGrad,
        "#extension GL_ATI_shader_texture_lod : require\n"
        "vec4 xlat_lib_tex2Dgrad(sampler2D s, vec2 coord, vec2 ddx, vec2 ddy) {\n"
        "   return texture2D_ATI( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTex3DBias,
        "vec4 xlat_lib_tex3Dbias(sampler3D s, vec4 coord) {\n"
        "  return texture3D( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTex3DLod,
        "vec4 xlat_lib_tex3Dlod(sampler3D s, vec4 coord) {\n"
        "  return textureLod( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTex3DGrad,
        "#extension GL_ATI_shader_texture_lod : require\n"
        "vec4 xlat_lib_tex3Dgrad(sampler3D s, vec3 coord, vec3 ddx, vec3 ddy) {\n"
        "  return texture3D_ATI( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeBias,   
        "vec4 xlat_lib_texCUBEbias(samplerCube s, vec4 coord) {\n"
        "  return textureCube( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeLod,   
        "vec4 xlat_lib_texCUBElod(samplerCube s, vec4 coord) {\n"
        "  return textureCubeLod( s, coord.xyz, coord.w);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpTexCubeGrad,  
        "#extension GL_ATI_shader_texture_lod : require\n"
        "vec4 xlat_lib_texCUBEgrad(samplerCUBE s, vec3 coord, vec3 ddx, vec3 ddy) {\n"
        "  return textureCube_ATI( s, coord, ddx, ddy);\n"
        "}\n\n" )
        );

   hlslSupportLib->insert( CodeMap::value_type( EOpD3DCOLORtoUBYTE4,  
        "ivec4 xlat_lib_D3DCOLORtoUBYTE4(vec4 x) {\n"
        "  return ivec4 ( x.zyxw * 255.001953 );\n"
        "}\n\n" )
        );

}

//=========================================================================================================
/// Free internal data structures
//=========================================================================================================
void finalizeHLSLSupportLibrary() 
{
   delete hlslSupportLib;
   hlslSupportLib = 0;
}

//=========================================================================================================
/// \param op
///   Opcode to get the support code for
/// \return
///    Return the code string supporting the operation
//=========================================================================================================
const std::string& getHLSLSupportCode( TOperator op ) 
{
   assert( hlslSupportLib);
   CodeMap::iterator it = hlslSupportLib->find( op );

   if ( it == hlslSupportLib->end())
      it = hlslSupportLib->find( EOpNull ); // this always exists

   return it->second;
}