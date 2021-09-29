%{

#include "MashCompileSettings.h"
#ifdef MASH_WINDOWS
#include <io.h>
#define YY_NO_UNISTD_H
#endif

// Note, Remove unistd.h file if getting isatty errors in mac
#ifndef MASH_WINDOWS
#include <unistd.h>
#endif

#include <string.h>
#include <stack>
#include "Material.h"
#include "MaterialParser.h"

#include "LexerHelper.h"
#include "MashMemoryPoolLexer.h"

namespace mash
{
extern int g_lineno;
extern sShaderCompilerData *g_shaderCompilerData;
}

%}

/* ------------------------------------------------------------------
   Macros
   ------------------------------------------------------------------ */

LETTER   [a-zA-Z_]
DIGIT    [0-9]
IDENT    {LETTER}({LETTER}|{DIGIT})*
STR      \"[^\"]*\"
WSPACE   [ \t]+
FLOAT_LITERAL		[0-9]+\.[0-9]+
SIGN           [+-]

/* ------------------------------------------------------------------
   Lexer rules
   ------------------------------------------------------------------ */
%%
"material"     	{return MATERIAL;}
"technique"		{return TECHNIQUE;}
"vertex"		{return TOK_VERTEX;}
"blendstate"	{return BLEND_STATE_DECLARATION;}
"rasteriser"	{return RASTERIZER_DECLARATION;}
"sampler2D"		{return SAMPLER2D;}
"samplerCUBE"	{return SAMPLERCUBE;}
"true"	{return TOK_TRUE;}
"false"	{return TOK_FALSE;}
"none" {return TOK_NONE;}
"{"      {return BEGIN_CS;}
"}"     {return END_CS;}
"loddistance" {return LOD_DISTANCE;}
"meshlod"	{return MESH_LOD_ENABLE;}
"userstring"	{return MATERIAL_USER_STRING;}

"lighting" {return TECH_LIGHTING_MODE;}

"auto" {return TECH_LIGHT_AUTO;}
"lightmap" {return TECH_LIGHT_MAPS;}
"pixel" {return TECH_LIGHT_PIXEL;}
"deferred" {return TECH_LIGHT_DEFERRED;}

"disabledshadoweffects"	{return TECH_DISABLED_SHADOW_EFFECTS;}
"shadingeffect"	{return TECH_SHADING_EFFECT;}
"shadowvertexprogram"	{return TECH_SHADOW_VERTEX_PROGRAM;}
"vertexprogram" {return TECH_VERTEX_PROGRAM;}
"pixelprogram" {return TECH_PIXEL_PROGRAM;}
"geometryprogram" {return TECH_GEOMETRY_PROGRAM;}
"lodlevel" {return TECH_LOD_LEVEL;}
"group" {return TECH_GROUP;}

"defineall" {return TECH_DEFINE_ALL;}
"definevertex" {return TECH_DEFINE_VERTEX;}
"definegeometry" {return TECH_DEFINE_GEOMETRY;}
"definepixel" {return TECH_DEFINE_PIXEL;}
"defineshadowvertex"	{return TECH_DEFINE_SHADOW_VERTEX;}

"minmagfilter" {return SAMPLER_MIN_MAG_FILTER;}
"mipfilter" {return SAMPLER_MIP_FILTER;}
"addressu" {return SAMPLER_ADDRESS_U;}
"addressv" {return SAMPLER_ADDRESS_V;}
"linear" {return SAMPLER_FILTER_LINEAR;}
"point" {return SAMPLER_FILTER_POINT;}
"clamp" {return SAMPLER_ADDRESSMODE_CLAMP;}
"wrap" {return SAMPLER_ADDRESSMODE_WRAP;}
"texture" {return SAMPLER_TEXTURE_FILE;}
"index" {return SAMPLER_INDEX;}

"position"	{return VERTEX_DECL_USAGE_POSITION;}
"blendweight"	{return VERTEX_DECL_USAGE_BLENDWEIGHT;}
"blendindex"	{return VERTEX_DECL_USAGE_BLENDINDICES;}
"normal"	{return VERTEX_DECL_USAGE_NORMAL;}
"texcoord"	{return VERTEX_DECL_USAGE_TEXCOORD;}
"tangent"	{return VERTEX_DECL_USAGE_TANGENT;}
"colour"	{return VERTEX_DECL_USAGE_COLOUR;}
"custom"	{return VERTEX_DECL_USAGE_CUSTOM;}

"r32float"	{return VERTEX_DECL_TYPE_R32_FLOAT;}
"rg32float"	{return VERTEX_DECL_TYPE_R32G32_FLOAT;}
"rgb32float"	{return VERTEX_DECL_TYPE_R32G32B32_FLOAT;}
"rgba32float"	{return VERTEX_DECL_TYPE_R32G32B32A32_FLOAT;}
"rgba8unorm"	{return VERTEX_DECL_TYPE_R8G8B8A8_UNORM;}
"rgba8uint"	{return VERTEX_DECL_TYPE_R8G8B8A8_UINT;}
"rg16sint"	{return VERTEX_DECL_TYPE_R16G16_SINT;}
"rgba16sint"	{return VERTEX_DECL_TYPE_R16G16B16A16_SINT;}

"blendingenabled"	{return BLEND_ENABLED;}
"srcblend"	{return BLEND_SOURCE;}
"destblend"	{return BLEND_DEST;}
"blendop"	{return BLEND_OP;}
"srcblendalpha"	{return BLEND_SOURCE_ALPHA;}
"destblendalpha"	{return BLEND_DEST_ALPHA;}
"blendopalpha"	{return BLEND_BLEND_OP_ALPHA;}
"writemask"	{return BLEND_MASK;}

"srcalpha"	{return BLEND_OPT_SRC_ALPHA;}
"invsrcalpha"	{return BLEND_OPT_INV_SRC_ALPHA;}
"destalpha"	{return BLEND_OPT_DEST_ALPHA;}
"destcolor"	{return BLEND_OPT_DEST_COLOR;}
"invdestalpha"	{return BLEND_OPT_INV_DEST_ALPHA;}
"invsrccolor"	{return BLEND_OPT_INV_SRC_COLOR;}
"one"	{return BLEND_OPT_ONE;}
"srcalphasat"	{return BLEND_OPT_SRC_ALPHA_SAT;}
"srccolor"	{return BLEND_OPT_SRC_COLOR;}
"zero"	{return BLEND_OPT_ZERO;}
"invdestcolor"	{return BLEND_OPT_INV_DEST_COLOR;}

"add"	{return BLEND_OP_ADD;}
"max"	{return BLEND_OP_MAX;}
"min"	{return BLEND_OP_MIN;}
"revsubtract"	{return BLEND_OP_REV_SUBTRACT;}
"subtract"	{return BLEND_OP_SUBTRACT;}

"red"	{return MASK_RED;}
"green"	{return MASK_GREEN;}
"blue"	{return MASK_BLUE;}
"alpha"	{return MASK_ALPHA;}
"all"	{return MASK_ALL;}

"fill"	{return RAST_FILL_MODE;}
"cull"	{return RAST_CULL_MODE;}
"depthtestenabled"	{return RAST_DEPTH_TEST_ENABLED;}
"depthwriteenabled"	{return RAST_DEPTH_WRITE_ENABLED;}
"depthcmp"	{return RAST_DEPTH_CMP;}
"depthbias"	{return RAST_DEPTH_BIAS;}
"depthbiasclamp"	{return RAST_DEPTH_BIAS_CLAMP;}
"slopescaledbias"	{return RAST_SLOPE_SCALED_DEPTH_BIAS;}

"wireframe"	{return FILL_MODE_WIRE_FRAME;}
"solid"	{return FILL_MODE_SOLID;}

"clockwise"	{return CULL_MODE_CW;}
"counterclockwise"	{return CULL_MODE_CCW;}

"never"	{return DEPTH_CMP_NEVER;}
"less"	{return DEPTH_CMP_LESS;}
"equal"	{return DEPTH_CMP_EQUAL;}
"lessequal"	{return DEPTH_CMP_LESS_EQUAL;}
"greater"	{return DEPTH_CMP_GREATER;}
"notequal"	{return DEPTH_CMP_NOT_EQUAL;}
"greaterequal"	{return DEPTH_CMP_GREATER_EQUAL;}
"always"	{return DEPTH_CMP_ALWAYS;}

{IDENT}  {
		mtrl_lval.str = (char*)mash::g_shaderCompilerData->g_materialLexerMemoryPool->GetMemory(strlen(yytext)+1);
		strcpy (mtrl_lval.str, yytext);
		
		return IDENTIFIER;}
		
{STR}    {
		mtrl_lval.str = mash::StringConstant(yytext);
		return STRING;}
		
{SIGN}?{DIGIT}+	{
		mtrl_lval.fNumber = mash::StringToNumber(yytext);
		return NUMBER_LITERAL;}
		
{SIGN}?{FLOAT_LITERAL}	{
		mtrl_lval.fNumber = mash::StringToNumber(yytext);
		return NUMBER_LITERAL;}
		
"//"     {mash::EatComment(yyinput);}
"/*"	{mash::EatBlockComment(yyinput);}
":"		{return COLON;}
\n       {mash::g_lineno++;}
\r\n       {mash::g_lineno++;}
{WSPACE} {}
.        {return ERROR_TOKEN;} 

%%

void SetLexerString(const char *sString)
{
	yy_scan_string(sString);
}

void DeleteLexerString()
{
	yy_delete_buffer(YY_CURRENT_BUFFER);
}
