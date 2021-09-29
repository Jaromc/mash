/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ERROR_TOKEN = 258,
     IDENTIFIER = 259,
     NUMBER_LITERAL = 260,
     STRING = 261,
     MATERIAL = 262,
     LOD_DISTANCE = 263,
     TECHNIQUE = 264,
     MESH_LOD_ENABLE = 265,
     MATERIAL_USER_STRING = 266,
     BEGIN_CS = 267,
     END_CS = 268,
     COLON = 269,
     TECH_LIGHT_MAPS = 270,
     TOK_VERTEX = 271,
     TECH_LIGHT_PIXEL = 272,
     TECH_LIGHTING_MODE = 273,
     TECH_LIGHT_DEFERRED = 274,
     TECH_LIGHT_AUTO = 275,
     TECH_SHADING_EFFECT = 276,
     TECH_DEFINE_ALL = 277,
     TECH_DEFINE_VERTEX = 278,
     TECH_DEFINE_PIXEL = 279,
     TECH_DEFINE_GEOMETRY = 280,
     TECH_DEFINE_SHADOW_VERTEX = 281,
     TOK_NONE = 282,
     TOK_TRUE = 283,
     TOK_FALSE = 284,
     TECH_SHADOW_VERTEX_PROGRAM = 285,
     TECH_VERTEX_PROGRAM = 286,
     TECH_PIXEL_PROGRAM = 287,
     TECH_GEOMETRY_PROGRAM = 288,
     TECH_LOD_LEVEL = 289,
     TECH_GROUP = 290,
     TECH_DISABLED_SHADOW_EFFECTS = 291,
     SAMPLER_MIN_MAG_FILTER = 292,
     SAMPLER_MIP_FILTER = 293,
     SAMPLER_ADDRESS_U = 294,
     SAMPLER_ADDRESS_V = 295,
     SAMPLER_FILTER_LINEAR = 296,
     SAMPLER_FILTER_POINT = 297,
     SAMPLER_ADDRESSMODE_CLAMP = 298,
     SAMPLER_ADDRESSMODE_WRAP = 299,
     SAMPLER2D = 300,
     SAMPLERCUBE = 301,
     SAMPLER_TEXTURE_FILE = 302,
     SAMPLER_INDEX = 303,
     VERTEX_DECL_USAGE_POSITION = 304,
     VERTEX_DECL_USAGE_BLENDWEIGHT = 305,
     VERTEX_DECL_USAGE_BLENDINDICES = 306,
     VERTEX_DECL_USAGE_COLOUR = 307,
     VERTEX_DECL_USAGE_NORMAL = 308,
     VERTEX_DECL_USAGE_TEXCOORD = 309,
     VERTEX_DECL_USAGE_CUSTOM = 310,
     VERTEX_DECL_USAGE_TANGENT = 311,
     VERTEX_DECL_TYPE_R32_FLOAT = 312,
     VERTEX_DECL_TYPE_R32G32_FLOAT = 313,
     VERTEX_DECL_TYPE_R32G32B32_FLOAT = 314,
     VERTEX_DECL_TYPE_R32G32B32A32_FLOAT = 315,
     VERTEX_DECL_TYPE_R8G8B8A8_UNORM = 316,
     VERTEX_DECL_TYPE_R8G8B8A8_UINT = 317,
     VERTEX_DECL_TYPE_R16G16_SINT = 318,
     VERTEX_DECL_TYPE_R16G16B16A16_SINT = 319,
     BLEND_ENABLED = 320,
     BLEND_STATE_DECLARATION = 321,
     BLEND_SOURCE = 322,
     BLEND_DEST = 323,
     BLEND_OP = 324,
     BLEND_SOURCE_ALPHA = 325,
     BLEND_DEST_ALPHA = 326,
     BLEND_BLEND_OP_ALPHA = 327,
     BLEND_MASK = 328,
     BLEND_OPT_SRC_ALPHA = 329,
     BLEND_OPT_INV_SRC_ALPHA = 330,
     BLEND_OPT_DEST_ALPHA = 331,
     BLEND_OPT_DEST_COLOR = 332,
     BLEND_OPT_INV_DEST_COLOR = 333,
     BLEND_OPT_INV_DEST_ALPHA = 334,
     BLEND_OPT_INV_SRC_COLOR = 335,
     BLEND_OPT_ONE = 336,
     BLEND_OPT_SRC_ALPHA_SAT = 337,
     BLEND_OPT_SRC_COLOR = 338,
     BLEND_OPT_ZERO = 339,
     BLEND_OP_ADD = 340,
     BLEND_OP_MAX = 341,
     BLEND_OP_MIN = 342,
     BLEND_OP_REV_SUBTRACT = 343,
     BLEND_OP_SUBTRACT = 344,
     MASK_RED = 345,
     MASK_GREEN = 346,
     MASK_BLUE = 347,
     MASK_ALPHA = 348,
     MASK_ALL = 349,
     FILL_MODE_WIRE_FRAME = 350,
     FILL_MODE_SOLID = 351,
     CULL_MODE_CW = 352,
     CULL_MODE_CCW = 353,
     DEPTH_CMP_NEVER = 354,
     DEPTH_CMP_LESS = 355,
     DEPTH_CMP_EQUAL = 356,
     DEPTH_CMP_LESS_EQUAL = 357,
     DEPTH_CMP_GREATER = 358,
     DEPTH_CMP_NOT_EQUAL = 359,
     DEPTH_CMP_GREATER_EQUAL = 360,
     DEPTH_CMP_ALWAYS = 361,
     RASTERIZER_DECLARATION = 362,
     RAST_FILL_MODE = 363,
     RAST_CULL_MODE = 364,
     RAST_DEPTH_TEST_ENABLED = 365,
     RAST_DEPTH_WRITE_ENABLED = 366,
     RAST_DEPTH_CMP = 367,
     RAST_DEPTH_BIAS = 368,
     RAST_DEPTH_BIAS_CLAMP = 369,
     RAST_SLOPE_SCALED_DEPTH_BIAS = 370
   };
#endif
/* Tokens.  */
#define ERROR_TOKEN 258
#define IDENTIFIER 259
#define NUMBER_LITERAL 260
#define STRING 261
#define MATERIAL 262
#define LOD_DISTANCE 263
#define TECHNIQUE 264
#define MESH_LOD_ENABLE 265
#define MATERIAL_USER_STRING 266
#define BEGIN_CS 267
#define END_CS 268
#define COLON 269
#define TECH_LIGHT_MAPS 270
#define TOK_VERTEX 271
#define TECH_LIGHT_PIXEL 272
#define TECH_LIGHTING_MODE 273
#define TECH_LIGHT_DEFERRED 274
#define TECH_LIGHT_AUTO 275
#define TECH_SHADING_EFFECT 276
#define TECH_DEFINE_ALL 277
#define TECH_DEFINE_VERTEX 278
#define TECH_DEFINE_PIXEL 279
#define TECH_DEFINE_GEOMETRY 280
#define TECH_DEFINE_SHADOW_VERTEX 281
#define TOK_NONE 282
#define TOK_TRUE 283
#define TOK_FALSE 284
#define TECH_SHADOW_VERTEX_PROGRAM 285
#define TECH_VERTEX_PROGRAM 286
#define TECH_PIXEL_PROGRAM 287
#define TECH_GEOMETRY_PROGRAM 288
#define TECH_LOD_LEVEL 289
#define TECH_GROUP 290
#define TECH_DISABLED_SHADOW_EFFECTS 291
#define SAMPLER_MIN_MAG_FILTER 292
#define SAMPLER_MIP_FILTER 293
#define SAMPLER_ADDRESS_U 294
#define SAMPLER_ADDRESS_V 295
#define SAMPLER_FILTER_LINEAR 296
#define SAMPLER_FILTER_POINT 297
#define SAMPLER_ADDRESSMODE_CLAMP 298
#define SAMPLER_ADDRESSMODE_WRAP 299
#define SAMPLER2D 300
#define SAMPLERCUBE 301
#define SAMPLER_TEXTURE_FILE 302
#define SAMPLER_INDEX 303
#define VERTEX_DECL_USAGE_POSITION 304
#define VERTEX_DECL_USAGE_BLENDWEIGHT 305
#define VERTEX_DECL_USAGE_BLENDINDICES 306
#define VERTEX_DECL_USAGE_COLOUR 307
#define VERTEX_DECL_USAGE_NORMAL 308
#define VERTEX_DECL_USAGE_TEXCOORD 309
#define VERTEX_DECL_USAGE_CUSTOM 310
#define VERTEX_DECL_USAGE_TANGENT 311
#define VERTEX_DECL_TYPE_R32_FLOAT 312
#define VERTEX_DECL_TYPE_R32G32_FLOAT 313
#define VERTEX_DECL_TYPE_R32G32B32_FLOAT 314
#define VERTEX_DECL_TYPE_R32G32B32A32_FLOAT 315
#define VERTEX_DECL_TYPE_R8G8B8A8_UNORM 316
#define VERTEX_DECL_TYPE_R8G8B8A8_UINT 317
#define VERTEX_DECL_TYPE_R16G16_SINT 318
#define VERTEX_DECL_TYPE_R16G16B16A16_SINT 319
#define BLEND_ENABLED 320
#define BLEND_STATE_DECLARATION 321
#define BLEND_SOURCE 322
#define BLEND_DEST 323
#define BLEND_OP 324
#define BLEND_SOURCE_ALPHA 325
#define BLEND_DEST_ALPHA 326
#define BLEND_BLEND_OP_ALPHA 327
#define BLEND_MASK 328
#define BLEND_OPT_SRC_ALPHA 329
#define BLEND_OPT_INV_SRC_ALPHA 330
#define BLEND_OPT_DEST_ALPHA 331
#define BLEND_OPT_DEST_COLOR 332
#define BLEND_OPT_INV_DEST_COLOR 333
#define BLEND_OPT_INV_DEST_ALPHA 334
#define BLEND_OPT_INV_SRC_COLOR 335
#define BLEND_OPT_ONE 336
#define BLEND_OPT_SRC_ALPHA_SAT 337
#define BLEND_OPT_SRC_COLOR 338
#define BLEND_OPT_ZERO 339
#define BLEND_OP_ADD 340
#define BLEND_OP_MAX 341
#define BLEND_OP_MIN 342
#define BLEND_OP_REV_SUBTRACT 343
#define BLEND_OP_SUBTRACT 344
#define MASK_RED 345
#define MASK_GREEN 346
#define MASK_BLUE 347
#define MASK_ALPHA 348
#define MASK_ALL 349
#define FILL_MODE_WIRE_FRAME 350
#define FILL_MODE_SOLID 351
#define CULL_MODE_CW 352
#define CULL_MODE_CCW 353
#define DEPTH_CMP_NEVER 354
#define DEPTH_CMP_LESS 355
#define DEPTH_CMP_EQUAL 356
#define DEPTH_CMP_LESS_EQUAL 357
#define DEPTH_CMP_GREATER 358
#define DEPTH_CMP_NOT_EQUAL 359
#define DEPTH_CMP_GREATER_EQUAL 360
#define DEPTH_CMP_ALWAYS 361
#define RASTERIZER_DECLARATION 362
#define RAST_FILL_MODE 363
#define RAST_CULL_MODE 364
#define RAST_DEPTH_TEST_ENABLED 365
#define RAST_DEPTH_WRITE_ENABLED 366
#define RAST_DEPTH_CMP 367
#define RAST_DEPTH_BIAS 368
#define RAST_DEPTH_BIAS_CLAMP 369
#define RAST_SLOPE_SCALED_DEPTH_BIAS 370




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 39 "MaterialParser.y"
{
   char *str;
   //int iNumber;
   float fNumber;
   bool bBool;
   int iNumber;
   mash::eLIGHTING_TYPE lightingType;
   mash::eTEXTURE_FILTERS texFilter;
   mash::eTEXTURE_ADDRESS texAddressValue;
   mash::eSAMPLER samplerType;
    mash::eVERTEX_DECLUSAGE vertexUsage;
   mash::eVERTEX_DECLTYPE vertexType;
   mash::eCULL_MODE cullMode;
   mash::eFILL_MODE fillMode;
   mash::eDEPTH_COMPARISON depthCmp;
   mash::eBLENDOP blendOp;
   mash::eBLEND blendState;
   mash::eCOLOUR_WRITE colourWriteMask;
}
/* Line 1529 of yacc.c.  */
#line 299 "MaterialParser.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE mtrl_lval;

