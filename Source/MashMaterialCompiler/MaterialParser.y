
%{

/* ------------------------------------------------------------------
   Initial code (copied verbatim to the output file)
   ------------------------------------------------------------------ */

// Includes
//#include <malloc.h>
#include <string.h>  // strcpy

#include "Material.h"
#include "LexerHelper.h"

// Some yacc (bison) defines
#define YYDEBUG 1	      // Generate debug code; needed for YYERROR_VERBOSE
#define YYERROR_VERBOSE // Give a more specific parse error message

// Forward references
#if defined (MASH_LINUX) || defined(MASH_APPLE)
extern void mtrl_error (const char *msg);
#endif
//The lexer
#define YY_DECL int mtrl_lex (void)

namespace mash
{
extern sShaderCompilerData *g_shaderCompilerData;
}

%}

/* ------------------------------------------------------------------
   Yacc declarations
   ------------------------------------------------------------------ */

/* The structure for passing value between lexer and parser */
%union {
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

%{
    extern int yylex(void);
%}

%token ERROR_TOKEN 
%token <str> IDENTIFIER 
%token <fNumber> NUMBER_LITERAL
%token <str> STRING
%token MATERIAL LOD_DISTANCE TECHNIQUE MESH_LOD_ENABLE MATERIAL_USER_STRING
%token BEGIN_CS END_CS COLON
%token TECH_LIGHT_MAPS TOK_VERTEX TECH_LIGHT_PIXEL TECH_LIGHTING_MODE TECH_LIGHT_DEFERRED TECH_LIGHT_AUTO TECH_SHADING_EFFECT
%token TECH_DEFINE_ALL TECH_DEFINE_VERTEX TECH_DEFINE_PIXEL TECH_DEFINE_GEOMETRY TECH_DEFINE_SHADOW_VERTEX
%token TOK_NONE  
%token TOK_TRUE TOK_FALSE
%token TECH_SHADOW_VERTEX_PROGRAM TECH_VERTEX_PROGRAM TECH_PIXEL_PROGRAM TECH_GEOMETRY_PROGRAM TECH_LOD_LEVEL TECH_GROUP TECH_DISABLED_SHADOW_EFFECTS
%token SAMPLER_MIN_MAG_FILTER SAMPLER_MIP_FILTER SAMPLER_ADDRESS_U SAMPLER_ADDRESS_V 
%token SAMPLER_FILTER_LINEAR SAMPLER_FILTER_POINT SAMPLER_ADDRESSMODE_CLAMP SAMPLER_ADDRESSMODE_WRAP
%token SAMPLER2D SAMPLERCUBE SAMPLER_TEXTURE_FILE SAMPLER_INDEX

%token VERTEX_DECL_USAGE_POSITION VERTEX_DECL_USAGE_BLENDWEIGHT VERTEX_DECL_USAGE_BLENDINDICES VERTEX_DECL_USAGE_COLOUR
%token VERTEX_DECL_USAGE_NORMAL VERTEX_DECL_USAGE_TEXCOORD VERTEX_DECL_USAGE_CUSTOM VERTEX_DECL_USAGE_TANGENT
%token VERTEX_DECL_TYPE_R32_FLOAT VERTEX_DECL_TYPE_R32G32_FLOAT VERTEX_DECL_TYPE_R32G32B32_FLOAT VERTEX_DECL_TYPE_R32G32B32A32_FLOAT
%token VERTEX_DECL_TYPE_R8G8B8A8_UNORM VERTEX_DECL_TYPE_R8G8B8A8_UINT VERTEX_DECL_TYPE_R16G16_SINT VERTEX_DECL_TYPE_R16G16B16A16_SINT

%token BLEND_ENABLED BLEND_STATE_DECLARATION BLEND_SOURCE BLEND_DEST BLEND_OP BLEND_SOURCE_ALPHA BLEND_DEST_ALPHA BLEND_BLEND_OP_ALPHA BLEND_MASK

%token BLEND_OPT_SRC_ALPHA BLEND_OPT_INV_SRC_ALPHA BLEND_OPT_DEST_ALPHA BLEND_OPT_DEST_COLOR BLEND_OPT_INV_DEST_COLOR
%token BLEND_OPT_INV_DEST_ALPHA BLEND_OPT_INV_SRC_COLOR BLEND_OPT_ONE BLEND_OPT_SRC_ALPHA_SAT BLEND_OPT_SRC_COLOR BLEND_OPT_ZERO 
%token BLEND_OP_ADD BLEND_OP_MAX BLEND_OP_MIN BLEND_OP_REV_SUBTRACT BLEND_OP_SUBTRACT MASK_RED MASK_GREEN MASK_BLUE MASK_ALPHA MASK_ALL

%token FILL_MODE_WIRE_FRAME FILL_MODE_SOLID CULL_MODE_CW CULL_MODE_CCW DEPTH_CMP_NEVER DEPTH_CMP_LESS DEPTH_CMP_EQUAL DEPTH_CMP_LESS_EQUAL
%token DEPTH_CMP_GREATER DEPTH_CMP_NOT_EQUAL DEPTH_CMP_GREATER_EQUAL DEPTH_CMP_ALWAYS

%token RASTERIZER_DECLARATION RAST_FILL_MODE RAST_CULL_MODE RAST_DEPTH_TEST_ENABLED RAST_DEPTH_WRITE_ENABLED RAST_DEPTH_CMP RAST_DEPTH_BIAS
%token RAST_DEPTH_BIAS_CLAMP RAST_SLOPE_SCALED_DEPTH_BIAS

%type<bBool>  bool_types
%type<samplerType> sampler_type
%type<texAddressValue> texture_address_method_type
%type<texFilter> sampler_filter_method_types
%type<texFilter> sampler_filter_method_mip_types
%type<lightingType> lighting_types
%type<str> optional_identifier
%type<fNumber> optional_digit
%type<str> optional_string
%type<vertexUsage> vertex_decl_usage
%type<vertexType> vertex_decl_type
%type<fillMode> fill_modes
%type<cullMode> cull_modes
%type<depthCmp> depth_cmps
%type<blendState> blend_states_opts
%type<blendOp> blend_operations
%type<colourWriteMask> write_mask
%type<iNumber> technique_defines

%expect 3 /*stops conflicts due to optional params found in vertex_element and technique_define_string_list*/

%start program_entry
%%

/* ------------------------------------------------------------------
   Yacc grammar rules
   ------------------------------------------------------------------ */
program_entry
	:	{
			/*ResetBlendState(mash::g_shaderCompilerData->g_blendState);
			ResetRasterizer(mash::g_shaderCompilerData->g_currentRasteriser);
			mash::g_shaderCompilerData->g_currentMaterial.Reset();
			mash::g_shaderCompilerData->g_currentTechnique.Reset();
			mash::g_shaderCompilerData->g_currentSampler.Reset();
			
			if (!mash::g_shaderCompilerData->g_tempDefineList.Empty())
				mash::g_shaderCompilerData->g_tempDefineList.Clear();
			if (!mash::g_shaderCompilerData->g_vertexDeclaration.Empty())
				mash::g_shaderCompilerData->g_vertexDeclaration.Clear();
			if (!mash::g_shaderCompilerData->g_materials.Empty())
				mash::g_shaderCompilerData->g_materials.Clear();*/
		}
		program_list
	|	/*empty*/
	;

program_list
	:	program
	|	program_list program
	;

program
	:	MATERIAL IDENTIFIER BEGIN_CS material_statement_list END_CS	
		{
			mash::g_shaderCompilerData->g_currentMaterial.sMaterialName = $2;
			mash::g_shaderCompilerData->g_materials.PushBack(mash::g_shaderCompilerData->g_currentMaterial);
			mash::g_shaderCompilerData->g_currentMaterial.Reset();
		}
		
	|	MATERIAL IDENTIFIER COLON IDENTIFIER BEGIN_CS material_ref_statement_list END_CS	
		{
			mash::g_shaderCompilerData->g_currentMaterial.CreateRef($2, $4);
			mash::g_shaderCompilerData->g_materials.PushBack(mash::g_shaderCompilerData->g_currentMaterial);
			mash::g_shaderCompilerData->g_currentMaterial.Reset();
		}
	;
	
technique_declaration
	:	TECHNIQUE IDENTIFIER BEGIN_CS technique_statement_list END_CS
		{
			mash::g_shaderCompilerData->g_currentTechnique.sName = $2;
		}
	;
	
material_ref_statement_list
	:	material_ref_statement
	|	material_ref_statement_list material_ref_statement
	;

material_ref_statement
	:	MATERIAL_USER_STRING user_string_list
	|	TECHNIQUE IDENTIFIER BEGIN_CS technique_ref_statement_list END_CS
		{
			mash::g_shaderCompilerData->g_currentTechnique.sName = $2;
			mash::g_shaderCompilerData->g_currentMaterial.techniques.PushBack(mash::g_shaderCompilerData->g_currentTechnique);
			mash::g_shaderCompilerData->g_currentTechnique.Reset();
		}
	|	sampler_declaration
		{
			mash::g_shaderCompilerData->g_currentMaterial.samplers.PushBack(mash::g_shaderCompilerData->g_currentSampler);
			mash::g_shaderCompilerData->g_currentSampler.Reset();
		}
	;
	
technique_ref_statement_list
	:	technique_ref_statement
	|	technique_ref_statement_list technique_ref_statement
	;
	
technique_ref_statement
	:	sampler_declaration
		{
			mash::g_shaderCompilerData->g_currentTechnique.samplers.PushBack(mash::g_shaderCompilerData->g_currentSampler);
			mash::g_shaderCompilerData->g_currentSampler.Reset();
		}
	;

material_statement_list
	:	material_statement
	|	material_statement_list material_statement
	;
	
material_statement
	:	LOD_DISTANCE lod_distace_list
	|	MESH_LOD_ENABLE bool_types
		{
			mash::g_shaderCompilerData->g_currentMaterial.meshLodEnabled = $2;
		}
	|	MATERIAL_USER_STRING user_string_list
	|	vertex_declaration	
		{
			mash::g_shaderCompilerData->g_currentMaterial.vertexDeclaration = mash::g_shaderCompilerData->g_vertexDeclaration;
			mash::g_shaderCompilerData->g_vertexDeclaration.Clear();
		}
	|	technique_declaration
		{
			mash::g_shaderCompilerData->g_currentMaterial.techniques.PushBack(mash::g_shaderCompilerData->g_currentTechnique);
			mash::g_shaderCompilerData->g_currentTechnique.Reset();
		}
	|	blend_state_declaration
		{
			if (mash::g_shaderCompilerData->g_currentMaterial.blendState)
			{
				delete mash::g_shaderCompilerData->g_currentMaterial.blendState;
				mash::g_shaderCompilerData->g_currentMaterial.blendState = 0;
			}
			
			mash::g_shaderCompilerData->g_currentMaterial.blendState = new mash::sBlendStates(mash::g_shaderCompilerData->g_blendState);
			ResetBlendState(mash::g_shaderCompilerData->g_blendState);
		}
	|	rasterizer_declaration
		{
			if (mash::g_shaderCompilerData->g_currentMaterial.rasteriserState)
			{
				delete mash::g_shaderCompilerData->g_currentMaterial.rasteriserState;
				mash::g_shaderCompilerData->g_currentMaterial.rasteriserState = 0;
			}
			
			mash::g_shaderCompilerData->g_currentMaterial.rasteriserState = new mash::sRasteriserStates(mash::g_shaderCompilerData->g_currentRasteriser);
			ResetRasterizer(mash::g_shaderCompilerData->g_currentRasteriser);
		}
	|	sampler_declaration
		{
			mash::g_shaderCompilerData->g_currentMaterial.samplers.PushBack(mash::g_shaderCompilerData->g_currentSampler);
			mash::g_shaderCompilerData->g_currentSampler.Reset();
		}
	;
	
user_string_list
	:	user_string
	|	user_string_list user_string
	;
	
user_string
	:	STRING 
		{
			if ($1)
			{
				mash::g_shaderCompilerData->g_currentMaterial.userString.PushBack($1);
				//delete []$1;
			}
		}
	;
	
lod_distace_list
	:	lod_distace
	|	lod_distace_list lod_distace
	;
	
lod_distace
	:	NUMBER_LITERAL	{mash::g_shaderCompilerData->g_currentMaterial.lodDistances.PushBack($1)}
	;
	
vertex_declaration
	:	TOK_VERTEX BEGIN_CS vertex_element_list END_CS
	;
	
vertex_element_list
	:	vertex_element
	|	vertex_element_list vertex_element
	;
	
vertex_element
	:	vertex_decl_usage vertex_decl_type optional_digit optional_digit
		{
			mash::sVertexElement vertexElement;
			vertexElement.declUsage = $1;
			vertexElement.declType = $2;
			vertexElement.stream = (int)$3;
			vertexElement.stepRate = (int)$4;
			mash::g_shaderCompilerData->g_vertexDeclaration.PushBack(vertexElement);
		}
	;

technique_statement_list
	:	technique_statement
	|	technique_statement_list technique_statement
	;
	
technique_statement
	:	TECH_VERTEX_PROGRAM STRING STRING optional_string
		{
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_VERTEX].profileString = $2;
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_VERTEX].fileName = $3;
			
			if ($4)
				mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_VERTEX].entry = $4;
				
			//these strings are stored in std::string so we can delete the old data
			//if ($2) delete []$2;
			//if ($3) delete []$3;
			//if ($4) delete []$4;
				
		}
	|	TECH_PIXEL_PROGRAM STRING STRING optional_string
		{
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_PIXEL].profileString = $2;
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_PIXEL].fileName = $3;
			
			if ($4)
				mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_PIXEL].entry = $4;
			
			//if ($2) delete []$2;
			//if ($3) delete []$3;
			//if ($4) delete []$4;
		}
	|	TECH_GEOMETRY_PROGRAM STRING STRING optional_string
		{
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_GEOMETRY].profileString = $2;
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_GEOMETRY].fileName = $3;
			
			if ($4)
				mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_GEOMETRY].entry = $4;
			
			//if ($2) delete []$2;
			//if ($3) delete []$3;
			//if ($4) delete []$4;
		}
	|	TECH_SHADOW_VERTEX_PROGRAM STRING STRING optional_string
		{
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_SHADOW_VERTEX].profileString = $2;
			mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_SHADOW_VERTEX].fileName = $3;
			
			if ($4)
				mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_SHADOW_VERTEX].entry = $4;
			
			//if ($2) delete []$2;
			//if ($3) delete []$3;
			//if ($4) delete []$4;
		}
	|	TECH_DISABLED_SHADOW_EFFECTS technique_shadow_effect_string_list
	|	TECH_SHADING_EFFECT STRING {mash::g_shaderCompilerData->g_currentTechnique.sShadingEffect = $2}
	|	TECH_LIGHTING_MODE lighting_types {mash::g_shaderCompilerData->g_currentTechnique.lightingType = $2}
	|	TECH_LOD_LEVEL lod_level_list
	|	TECH_GROUP STRING	{mash::g_shaderCompilerData->g_currentTechnique.sGroupName = $2}
	|	technique_defines technique_define_string_list
		{
			switch($1)
			{
				case TECH_DEFINE_VERTEX:
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_VERTEX].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_VERTEX].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					break;
				case TECH_DEFINE_PIXEL:
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_PIXEL].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_PIXEL].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					break;
				case TECH_DEFINE_GEOMETRY:
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_GEOMETRY].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_GEOMETRY].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					break;
				case TECH_DEFINE_SHADOW_VERTEX:
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_SHADOW_VERTEX].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_SHADOW_VERTEX].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					break;
				case TECH_DEFINE_ALL:
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_VERTEX].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_VERTEX].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_PIXEL].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_PIXEL].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_GEOMETRY].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_GEOMETRY].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_SHADOW_VERTEX].macros.Insert(mash::g_shaderCompilerData->g_currentTechnique.programs[mash::aTECH_PROG_SHADOW_VERTEX].macros.Begin(), 
					mash::g_shaderCompilerData->g_tempDefineList.Begin(), mash::g_shaderCompilerData->g_tempDefineList.End());
					break;
			}
			
			mash::g_shaderCompilerData->g_tempDefineList.Clear();
		}
	|	sampler_declaration
		{
			mash::g_shaderCompilerData->g_currentTechnique.samplers.PushBack(mash::g_shaderCompilerData->g_currentSampler);
			mash::g_shaderCompilerData->g_currentSampler.Reset();
		}
	|	vertex_declaration
		{
			mash::g_shaderCompilerData->g_currentTechnique.vertexDeclaration = mash::g_shaderCompilerData->g_vertexDeclaration;
			mash::g_shaderCompilerData->g_vertexDeclaration.Clear();
		}
	|	blend_state_declaration
		{
			mash::g_shaderCompilerData->g_currentTechnique.blendState = mash::g_shaderCompilerData->g_blendState;
			ResetBlendState(mash::g_shaderCompilerData->g_blendState);
		}
	|	rasterizer_declaration
		{
			mash::g_shaderCompilerData->g_currentTechnique.rasteriserState = mash::g_shaderCompilerData->g_currentRasteriser;
			ResetRasterizer(mash::g_shaderCompilerData->g_currentRasteriser);
		}
	;
	
technique_shadow_effect_string_list
	:	/*empty*/	
	|	technique_shadow_effect_string
	|	technique_shadow_effect_string_list technique_shadow_effect_string
	;
	
technique_shadow_effect_string
	:	STRING	
		{
			mash::eLIGHTTYPE lightType = mash::GetLightTypeFromString($1);
			if (lightType != mash::aLIGHT_TYPE_COUNT)
				mash::g_shaderCompilerData->g_currentTechnique.disabledShadowCasters[lightType] = false;
		}
	;
	
technique_define_string_list
	:	/*empty*/	
	|	technique_define_string
	|	technique_define_string_list technique_define_string
	;
	
technique_define_string
	:	STRING	
		{
			mash::sEffectMacro m;
			m.name = $1;
			mash::g_shaderCompilerData->g_tempDefineList.PushBack(m);
		}
	;

rasterizer_declaration
	:	RASTERIZER_DECLARATION BEGIN_CS rasterizer_state_list END_CS
	;
	
rasterizer_state_list
	:	rasterizer_states
	|	rasterizer_state_list rasterizer_states
	;
	
rasterizer_states
	:	RAST_FILL_MODE fill_modes	{mash::g_shaderCompilerData->g_currentRasteriser.fillMode = $2}
	|	RAST_CULL_MODE cull_modes	{mash::g_shaderCompilerData->g_currentRasteriser.cullMode = $2}
	|	RAST_DEPTH_TEST_ENABLED	bool_types	{mash::g_shaderCompilerData->g_currentRasteriser.depthTestingEnable = $2}
	|	RAST_DEPTH_WRITE_ENABLED bool_types	{mash::g_shaderCompilerData->g_currentRasteriser.depthWritingEnabled = $2}
	|	RAST_DEPTH_CMP depth_cmps	{mash::g_shaderCompilerData->g_currentRasteriser.depthComparison = $2}
	|	RAST_DEPTH_BIAS NUMBER_LITERAL	{mash::g_shaderCompilerData->g_currentRasteriser.depthBias = $2}
	|	RAST_DEPTH_BIAS_CLAMP NUMBER_LITERAL	{mash::g_shaderCompilerData->g_currentRasteriser.depthBiasClamp = $2}
	|	RAST_SLOPE_SCALED_DEPTH_BIAS NUMBER_LITERAL	{mash::g_shaderCompilerData->g_currentRasteriser.slopeScaledDepthBias = $2}
	;
	
fill_modes
	:	FILL_MODE_WIRE_FRAME	{$$ = mash::aFILL_WIRE_FRAME}
	|	FILL_MODE_SOLID	{$$ = mash::aFILL_SOLID}
	;
	
cull_modes
	:	TOK_NONE	{$$ = mash::aCULL_NONE}
	|	CULL_MODE_CW	{$$ = mash::aCULL_CW}
	|	CULL_MODE_CCW	{$$ = mash::aCULL_CCW}
	;
	
depth_cmps
	:	DEPTH_CMP_NEVER	{$$ = mash::aZCMP_NEVER}
	|	DEPTH_CMP_LESS	{$$ = mash::aZCMP_LESS}
	|	DEPTH_CMP_EQUAL	{$$ = mash::aZCMP_EQUAL}
	|	DEPTH_CMP_LESS_EQUAL	{$$ = mash::aZCMP_LESS_EQUAL}
	|	DEPTH_CMP_GREATER	{$$ = mash::aZCMP_GREATER}
	|	DEPTH_CMP_NOT_EQUAL	{$$ = mash::aZCMP_NOT_EQUAL}
	|	DEPTH_CMP_GREATER_EQUAL	{$$ = mash::aZCMP_GREATER_EQUAL}
	|	DEPTH_CMP_ALWAYS	{$$ = mash::aZCMP_ALWAYS}
	;
	
sampler_declaration
	:	sampler_type optional_identifier BEGIN_CS sampler_statement_list END_CS
		{
			mash::g_shaderCompilerData->g_currentSampler.sSamplerName = $2;
			mash::g_shaderCompilerData->g_currentSampler.type = $1;
		}
	;
	
blend_state_declaration
	:	BLEND_STATE_DECLARATION BEGIN_CS blend_state_list END_CS
	;
	
blend_state_list
	:	blend_state
	|	blend_state_list blend_state
	;
	
blend_state
	:	BLEND_ENABLED bool_types	{mash::g_shaderCompilerData->g_blendState.blendingEnabled = $2}
	|	BLEND_SOURCE blend_states_opts	{mash::g_shaderCompilerData->g_blendState.srcBlend = $2}
	|	BLEND_DEST blend_states_opts	{mash::g_shaderCompilerData->g_blendState.destBlend = $2}
	|	BLEND_OP blend_operations	{mash::g_shaderCompilerData->g_blendState.blendOp = $2}
	|	BLEND_SOURCE_ALPHA blend_states_opts	{mash::g_shaderCompilerData->g_blendState.srcBlendAlpha = $2}
	|	BLEND_DEST_ALPHA blend_states_opts	{mash::g_shaderCompilerData->g_blendState.destBlendAlpha = $2}
	|	BLEND_BLEND_OP_ALPHA blend_operations	{mash::g_shaderCompilerData->g_blendState.blendOpAlpha = $2}
	|	BLEND_MASK write_mask	{mash::g_shaderCompilerData->g_blendState.colourWriteMask = $2}
	;
	
blend_states_opts
	:	BLEND_OPT_SRC_ALPHA	{$$ = mash::aBLEND_SRC_ALPHA}
	|	BLEND_OPT_INV_SRC_ALPHA	{$$ = mash::aBLEND_INV_SRC_ALPHA}
	|	BLEND_OPT_DEST_ALPHA	{$$ = mash::aBLEND_DEST_ALPHA}
	|	BLEND_OPT_DEST_COLOR	{$$ = mash::aBLEND_DEST_COLOR}
	|	BLEND_OPT_INV_DEST_ALPHA	{$$ = mash::aBLEND_INV_DEST_ALPHA}
	|	BLEND_OPT_INV_SRC_COLOR	{$$ = mash::aBLEND_INV_SRC_COLOR}
	|	BLEND_OPT_ONE	{$$ = mash::aBLEND_ONE}
	|	BLEND_OPT_SRC_ALPHA_SAT	{$$ = mash::aBLEND_SRC_ALPHA_SAT}
	|	BLEND_OPT_SRC_COLOR	{$$ = mash::aBLEND_SRC_COLOR}
	|	BLEND_OPT_ZERO	{$$ = mash::aBLEND_ZERO}
	|	BLEND_OPT_INV_DEST_COLOR	{$$ = mash::aBLEND_INV_DEST_COLOR}
	;
	
blend_operations
	:	BLEND_OP_ADD	{$$ = mash::aBLENDOP_ADD}
	|	BLEND_OP_MAX	{$$ = mash::aBLENDOP_MAX}
	|	BLEND_OP_MIN	{$$ = mash::aBLENDOP_MIN}
	|	BLEND_OP_REV_SUBTRACT	{$$ = mash::aBLENDOP_REV_SUBTRACT}
	|	BLEND_OP_SUBTRACT	{$$ = mash::aBLENDOP_SUBTRACT}
	;
	
write_mask
	:	MASK_RED	{$$ = mash::aCOLOUR_WRITE_RED}
	|	MASK_GREEN	{$$ = mash::aCOLOUR_WRITE_GREEN}
	|	MASK_BLUE	{$$ = mash::aCOLOUR_WRITE_BLUE}
	|	MASK_ALPHA	{$$ = mash::aCOLOUR_WRITE_ALPHA}
	|	MASK_ALL	{$$ = mash::aCOLOUR_WRITE_ALL}
    |	TOK_NONE	{$$ = mash::aCOLOUR_WRITE_NONE}
	;
	
sampler_statement_list
	:	sampler_statement
	|	sampler_statement_list sampler_statement
	;
	
sampler_statement
	:	SAMPLER_MIN_MAG_FILTER sampler_filter_method_types	{mash::g_shaderCompilerData->g_currentSampler.minMagFilter = $2}
	|	SAMPLER_MIP_FILTER sampler_filter_method_mip_types	{mash::g_shaderCompilerData->g_currentSampler.mipFilter = $2}
	|	SAMPLER_ADDRESS_U texture_address_method_type	{mash::g_shaderCompilerData->g_currentSampler.addressU = $2}
	|	SAMPLER_ADDRESS_V texture_address_method_type	{mash::g_shaderCompilerData->g_currentSampler.addressV = $2}
	|	SAMPLER_TEXTURE_FILE STRING	{mash::g_shaderCompilerData->g_currentSampler.sTextureFile = $2}
	|	SAMPLER_INDEX NUMBER_LITERAL	{mash::g_shaderCompilerData->g_currentSampler.index = $2}
	;
	
sampler_filter_method_types
	:	SAMPLER_FILTER_LINEAR	{$$ = mash::TEX_FILTER_LINEAR}
	|	SAMPLER_FILTER_POINT	{$$ = mash::TEX_FILTER_POINT}
	;
	
sampler_filter_method_mip_types
	:	SAMPLER_FILTER_LINEAR	{$$ = mash::TEX_FILTER_LINEAR}
	|	SAMPLER_FILTER_POINT	{$$ = mash::TEX_FILTER_POINT}
	|	TOK_NONE	{$$ = mash::TEX_FILTER_MIPNONE}
	;
	
texture_address_method_type
	:	SAMPLER_ADDRESSMODE_CLAMP	{$$ = mash::aTEXTURE_ADDRESS_CLAMP}
	|	SAMPLER_ADDRESSMODE_WRAP	{$$ = mash::aTEXTURE_ADDRESS_WRAP}
	;
	
sampler_type
	:	SAMPLER2D	{$$ = mash::aSAMPLER2D}
	|	SAMPLERCUBE	{$$ = mash::aSAMPLERCUBE}
	;

lod_level_list
	:	lod_level
	|	lod_level_list lod_level
	;
	
technique_defines
	:	TECH_DEFINE_ALL	{$$ = TECH_DEFINE_ALL}
	|	TECH_DEFINE_VERTEX	{$$ = TECH_DEFINE_VERTEX}
	|	TECH_DEFINE_PIXEL	{$$ = TECH_DEFINE_PIXEL}
	|	TECH_DEFINE_GEOMETRY	{$$ = TECH_DEFINE_GEOMETRY}
	;

lod_level
	:	NUMBER_LITERAL	{mash::g_shaderCompilerData->g_currentTechnique.supportedLodLevels.PushBack($1)}
	;
	
lighting_types
	:	TOK_NONE	{$$ = mash::aLIGHT_TYPE_NONE}
	|	TECH_LIGHT_MAPS	{$$ = mash::aLIGHT_TYPE_LIGHT_MAP}
	|	TOK_VERTEX	{$$ = mash::aLIGHT_TYPE_VERTEX}
	|	TECH_LIGHT_AUTO	{$$ = mash::aLIGHT_TYPE_AUTO}
	|	TECH_LIGHT_PIXEL	{$$ = mash::aLIGHT_TYPE_PIXEL}
	|	TECH_LIGHT_DEFERRED	{$$ = mash::aLIGHT_TYPE_DEFERRED}
	;
	
vertex_decl_usage
	:	VERTEX_DECL_USAGE_POSITION	{$$ = mash::aDECLUSAGE_POSITION}
	|	VERTEX_DECL_USAGE_BLENDWEIGHT	{$$ = mash::aDECLUSAGE_BLENDWEIGHT}
	|	VERTEX_DECL_USAGE_BLENDINDICES	{$$ = mash::aDECLUSAGE_BLENDINDICES}
	|	VERTEX_DECL_USAGE_NORMAL	{$$ = mash::aDECLUSAGE_NORMAL}
	|	VERTEX_DECL_USAGE_TEXCOORD	{$$ = mash::aDECLUSAGE_TEXCOORD}
	|	VERTEX_DECL_USAGE_CUSTOM	{$$ = mash::aDECLUSAGE_CUSTOM}
	|	VERTEX_DECL_USAGE_TANGENT	{$$ = mash::aDECLUSAGE_TANGENT}
	|	VERTEX_DECL_USAGE_COLOUR	{$$ = mash::aDECLUSAGE_COLOUR}
	;
	
vertex_decl_type
	:	VERTEX_DECL_TYPE_R32_FLOAT	{$$ = mash::aDECLTYPE_R32_FLOAT}
	|	VERTEX_DECL_TYPE_R32G32_FLOAT	{$$ = mash::aDECLTYPE_R32G32_FLOAT}
	|	VERTEX_DECL_TYPE_R32G32B32_FLOAT	{$$ = mash::aDECLTYPE_R32G32B32_FLOAT}
	|	VERTEX_DECL_TYPE_R32G32B32A32_FLOAT	{$$ = mash::aDECLTYPE_R32G32B32A32_FLOAT}
	|	VERTEX_DECL_TYPE_R8G8B8A8_UNORM	{$$ = mash::aDECLTYPE_R8G8B8A8_UNORM}
	|	VERTEX_DECL_TYPE_R8G8B8A8_UINT	{$$ = mash::aDECLTYPE_R8G8B8A8_UINT}
	|	VERTEX_DECL_TYPE_R16G16_SINT	{$$ = mash::aDECLTYPE_R16G16_SINT}
	|	VERTEX_DECL_TYPE_R16G16B16A16_SINT	{$$ = mash::aDECLTYPE_R16G16B16A16_SINT}
	;
	
optional_string
	:	STRING	{$$ = $1}
	|	/*empty*/	{$$ = 0}
	;
	
optional_digit
	:	NUMBER_LITERAL {$$ = $1}
	|	/*empty*/	{$$ = 0}
	;
	
optional_identifier
	:	IDENTIFIER {$$ = $1}
	|	/*empty*/ {$$ = 0}
	;
	
bool_types
	:	TOK_TRUE	{$$ = true}
	|	TOK_FALSE	{$$ = false}
	;

%%
/* ------------------------------------------------------------------
   Additional code (again copied verbatim to the output file)
   ------------------------------------------------------------------ */
