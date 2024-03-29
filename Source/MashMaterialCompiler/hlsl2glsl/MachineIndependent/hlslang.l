/*-*-C++-*-
//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
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
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
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

*/
/* Based on
ANSI C grammar, Lex specification

In 1985, Jeff Lee published this Lex specification together with a Yacc 
grammar for the April 30, 1985 ANSI C draft.  Tom Stockfisch reposted 
both to net.sources in 1987; that original, as mentioned in the answer 
to question 17.25 of the comp.lang.c FAQ, can be ftp'ed from ftp.uu.net, 
file usenet/net.sources/ansi.c.grammar.Z. 

I intend to keep this version as close to the current C Standard grammar 
as possible; please let me know if you discover discrepancies. 

Jutta Degener, 1995 
*/

D           [0-9]
L           [a-zA-Z_]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
O           [0-7]
F           [hHfF]
I           [uUlL]


%option nounput 
%{
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "ParseHelper.h"
#include "hlslang_tab.h"

/* windows only pragma */
#ifdef _MSC_VER
#pragma warning(disable : 4102)
#endif

int yy_input(char* buf, int max_size);
//TSourceLoc yylineno;

#ifdef _WIN32
	TSourceLoc yylineno;
    extern int yyparse(TParseContext&);
    #define YY_DECL int yylex(YYSTYPE* pyylval, TParseContext& parseContext)    
#else
    extern int yyparse(void*);
    #define YY_DECL int yylex(YYSTYPE* pyylval, void* parseContextLocal)
    #define parseContext (*((TParseContext*)(parseContextLocal)))		
#endif
 
#define YY_INPUT(buf,result,max_size) (result = yy_input(buf, max_size))

%}

%option noyywrap
%option never-interactive
%option outfile="Gen_hlslang.cpp"
%x FIELDS


%%
<*>"//"[^\n]*"\n"     { /* ?? carriage and/or line-feed? */ };

"const"        {  pyylval->lex.line = yylineno; return(CONST_QUAL); }
"uniform"      {  pyylval->lex.line = yylineno; return(UNIFORM); }

"break"        {  pyylval->lex.line = yylineno; return(BREAK); }
"continue"     {  pyylval->lex.line = yylineno; return(CONTINUE); }
"do"           {  pyylval->lex.line = yylineno; return(DO); }
"for"          {  pyylval->lex.line = yylineno; return(FOR); }
"while"        {  pyylval->lex.line = yylineno; return(WHILE); }

"if"           {  pyylval->lex.line = yylineno; return(IF); }
"else"         {  pyylval->lex.line = yylineno; return(ELSE); }

"in"           {  pyylval->lex.line = yylineno; return(IN_QUAL); }
"out"          {  pyylval->lex.line = yylineno; return(OUT_QUAL); }
"inout"        {  pyylval->lex.line = yylineno; return(INOUT_QUAL); }

"float"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(FLOAT_TYPE); }
"float1"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(FLOAT_TYPE); }
"int"          {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(INT_TYPE); }
"int1"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(INT_TYPE); }
"void"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(VOID_TYPE); }
"bool"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(BOOL_TYPE); }
"bool1"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(BOOL_TYPE); }
"string"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(STRING_TYPE); }
"true"         {  pyylval->lex.line = yylineno; pyylval->lex.b = true;  return(BOOLCONSTANT); }
"false"        {  pyylval->lex.line = yylineno; pyylval->lex.b = false; return(BOOLCONSTANT); }

"discard"      {  pyylval->lex.line = yylineno; return(DISCARD); }
"return"       {  pyylval->lex.line = yylineno; return(RETURN); }

"float2x2"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(MATRIX2); }
"float3x3"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(MATRIX3); }
"float4x4"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(MATRIX4); }
"half2x2"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(MATRIX2); }
"half3x3"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(MATRIX3); }
"half4x4"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(MATRIX4); }

"half"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(FLOAT_TYPE); }
"half1"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return(FLOAT_TYPE); }
"half2"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (VEC2); }
"half3"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (VEC3); }
"half4"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (VEC4); }
"float2"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (VEC2); }
"float3"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (VEC3); }
"float4"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (VEC4); }
"int2"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (IVEC2); }
"int3"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (IVEC3); }
"int4"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (IVEC4); }
"bool2"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (BVEC2); }
"bool3"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (BVEC3); }
"bool4"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (BVEC4); }

"vector"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (VECTOR); }
"matrix"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (MATRIX); }
"register"        {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return (REGISTER); }

"sampler1D"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return SAMPLER1D; }
"sampler2D"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return SAMPLER2D; }
"sampler3D"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return SAMPLER3D; }

"sampler"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return SAMPLERGENERIC; }
"samplerCUBE"     {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return SAMPLERCUBE; }

"texture"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return TEXTURE; }
"texture2D"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return TEXTURE; }
"texture3D"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return TEXTURE; }
"textureCUBE"       {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return TEXTURE; }
"sampler_state"     {  pyylval->lex.line = yylineno; parseContext.lexAfterType = true; return SAMPLERSTATE; }

"struct"       {  pyylval->lex.line = yylineno; return(STRUCT); }

"asm"          {  PaReservedWord(); return 0; }

"class"        {  PaReservedWord(); return 0; }
"union"        {  PaReservedWord(); return 0; }
"enum"         {  PaReservedWord(); return 0; }
"typedef"      {  PaReservedWord(); return 0; }
"template"     {  PaReservedWord(); return 0; }
"this"         {  PaReservedWord(); return 0; }
"packed"       {  PaReservedWord(); return 0; }

"goto"         {  PaReservedWord(); return 0; }
"switch"       {  PaReservedWord(); return 0; }
"default"      {  PaReservedWord(); return 0; }

"inline"       {  PaReservedWord(); return 0; }
"noinline"     {  PaReservedWord(); return 0; }
"volatile"     {  PaReservedWord(); return 0; }
"public"       {  PaReservedWord(); return 0; }
"static"       {  /*pyylval->lex.line = yylineno; return(UNIFORM);*//*Treat as uniform, till we write out the shader code*//*PaReservedWord(); return 0;*/ }
"extern"       {  PaReservedWord(); return 0; }
"external"     {  PaReservedWord(); return 0; }
"interface"    {  PaReservedWord(); return 0; }

"long"         {  PaReservedWord(); return 0; }
"short"        {  PaReservedWord(); return 0; }
"double"       {  PaReservedWord(); return 0; }
"fixed"        {  PaReservedWord(); return 0; }
"unsigned"     {  PaReservedWord(); return 0; }

"hvec2"        {  PaReservedWord(); return 0; }
"hvec3"        {  PaReservedWord(); return 0; }
"hvec4"        {  PaReservedWord(); return 0; }
"fvec2"        {  PaReservedWord(); return 0; }
"fvec3"        {  PaReservedWord(); return 0; }
"fvec4"        {  PaReservedWord(); return 0; }
"dvec2"        {  PaReservedWord(); return 0; }
"dvec3"        {  PaReservedWord(); return 0; }
"dvec4"        {  PaReservedWord(); return 0; }

"sampler3DRect"        {  PaReservedWord(); return 0; }

"sizeof"       {  PaReservedWord(); return 0; }
"cast"         {  PaReservedWord(); return 0; }

"namespace"    {  PaReservedWord(); return 0; }
"using"        {  PaReservedWord(); return 0; }

{L}({L}|{D})*       {  
   pyylval->lex.line = yylineno; 
   pyylval->lex.string = NewPoolTString(yytext); 
   return PaIdentOrType(*pyylval->lex.string, parseContext, pyylval->lex.symbol); 
}

{D}+{E}{F}?           { pyylval->lex.line = yylineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }
{D}+"."{D}*({E})?{F}? { pyylval->lex.line = yylineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }
"."{D}+({E})?{F}?     { pyylval->lex.line = yylineno; pyylval->lex.f = static_cast<float>(atof(yytext)); return(FLOATCONSTANT); }

0[xX]{H}+{I}?         { pyylval->lex.line = yylineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }
0{O}+{I}?             { pyylval->lex.line = yylineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }
0{D}+{I}?             { pyylval->lex.line = yylineno; parseContext.error(yylineno, "Invalid Octal number.", yytext, "", ""); parseContext.recover(); return 0;}
{D}+{I}?              { pyylval->lex.line = yylineno; pyylval->lex.i = strtol(yytext, 0, 0); return(INTCONSTANT); }





"/*"            {  int ret = PaParseComment(pyylval->lex.line, parseContext); if (!ret) return ret; }   

"+="            {  pyylval->lex.line = yylineno; return(ADD_ASSIGN); }
"-="            {  pyylval->lex.line = yylineno; return(SUB_ASSIGN); }
"*="            {  pyylval->lex.line = yylineno; return(MUL_ASSIGN); }
"/="            {  pyylval->lex.line = yylineno; return(DIV_ASSIGN); }
"%="            {  pyylval->lex.line = yylineno; return(MOD_ASSIGN); }
"<<="           {  pyylval->lex.line = yylineno; return(LEFT_ASSIGN); }
">>="           {  pyylval->lex.line = yylineno; return(RIGHT_ASSIGN); }
"&="            {  pyylval->lex.line = yylineno; return(AND_ASSIGN); }
"^="            {  pyylval->lex.line = yylineno; return(XOR_ASSIGN); }
"|="            {  pyylval->lex.line = yylineno; return(OR_ASSIGN); }

"++"            {  pyylval->lex.line = yylineno; return(INC_OP); }
"--"            {  pyylval->lex.line = yylineno; return(DEC_OP); }
"&&"            {  pyylval->lex.line = yylineno; return(AND_OP); }
"||"            {  pyylval->lex.line = yylineno; return(OR_OP); }
"^^"            {  pyylval->lex.line = yylineno; return(XOR_OP); }
"<="            {  pyylval->lex.line = yylineno; return(LE_OP); }
">="            {  pyylval->lex.line = yylineno; return(GE_OP); }
"=="            {  pyylval->lex.line = yylineno; return(EQ_OP); }
"!="            {  pyylval->lex.line = yylineno; return(NE_OP); }
"<<"            {  pyylval->lex.line = yylineno; return(LEFT_OP); }
">>"            {  pyylval->lex.line = yylineno; return(RIGHT_OP); }
";"         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = false; return(SEMICOLON); }
("{"|"<%")      {  pyylval->lex.line = yylineno; parseContext.lexAfterType = false; return(LEFT_BRACE); }
("}"|"%>")      {  pyylval->lex.line = yylineno; return(RIGHT_BRACE); }
","         {  pyylval->lex.line = yylineno; if (parseContext.inTypeParen) parseContext.lexAfterType = false; return(COMMA); }
":"         {  pyylval->lex.line = yylineno; return(COLON); }
"="         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = false; return(EQUAL); }
"("         {  pyylval->lex.line = yylineno; parseContext.lexAfterType = false; parseContext.inTypeParen = true; return(LEFT_PAREN); }
")"         {  pyylval->lex.line = yylineno; parseContext.inTypeParen = false; return(RIGHT_PAREN); }
("["|"<:")      {  pyylval->lex.line = yylineno; return(LEFT_BRACKET); }
("]"|":>")      {  pyylval->lex.line = yylineno; return(RIGHT_BRACKET); }
"."         { BEGIN(FIELDS);  return(DOT); }
"!"         {  pyylval->lex.line = yylineno; return(BANG); }
"-"         {  pyylval->lex.line = yylineno; return(DASH); }
"~"         {  pyylval->lex.line = yylineno; return(TILDE); }
"+"         {  pyylval->lex.line = yylineno; return(PLUS); }
"*"         {  pyylval->lex.line = yylineno; return(STAR); }
"/"         {  pyylval->lex.line = yylineno; return(SLASH); }
"%"         {  pyylval->lex.line = yylineno; return(PERCENT); }
"<"         {  pyylval->lex.line = yylineno; return(LEFT_ANGLE); }
">"         {  pyylval->lex.line = yylineno; return(RIGHT_ANGLE); }
"|"         {  pyylval->lex.line = yylineno; return(VERTICAL_BAR); }
"^"         {  pyylval->lex.line = yylineno; return(CARET); }
"&"         {  pyylval->lex.line = yylineno; return(AMPERSAND); }
"?"         {  pyylval->lex.line = yylineno; return(QUESTION); }

\"[^\"]*\"  {  pyylval->lex.line = yylineno; return(STRINGCONSTANT); }

<FIELDS>{L}({L}|{D})* { 
BEGIN(INITIAL);      
    pyylval->lex.line = yylineno;     
    pyylval->lex.string = NewPoolTString(yytext); 
    return FIELD_SELECTION; }
<FIELDS>[ \t\v\f\r] {}


[ \t\v\n\f\r]   {  }
<*><<EOF>> { (&parseContext)->AfterEOF = true; yy_delete_buffer(YY_CURRENT_BUFFER); yyterminate();}
<*>.    { parseContext.infoSink.info << "FLEX: Unknown char " << yytext << "\n";
          return 0; }

%%



//Including Pre-processor.
extern "C" {
  #include "./preprocessor/preprocess.h"
} 

//
// The YY_INPUT macro just calls this.  Maybe this could be just put into
// the macro directly.
//

int yy_input(char* buf, int max_size)
{
     char *char_token =NULL;
     int len;

    if ((len = yylex_CPP(buf, max_size)) == 0)
        return 0;
    if (len >= max_size) 
        YY_FATAL_ERROR( "input buffer overflow, can't enlarge buffer because scanner uses REJECT" );

    //debug code to dump the token stream to stdout
    //buf[len] = '\0';
    //printf( ":: %s\n", buf);

    buf[len] = ' ';
	return len+1;
}


//
// Parse an array of strings using yyparse.  We set up globals used by
// yywrap.
//
// Returns 0 for success, as per yyparse().
//
int PaParseStrings(char* argv[], int strLen[], int argc, TParseContext& parseContextLocal)
{
    int argv0len;
    
    ScanFromString(argv[0]); 
    
    //Storing the Current Compiler Parse context into the cpp structure.
	cpp->pC = (void*)&parseContextLocal;
	
	if (!argv || argc == 0)
        return 1;
    
    for (int i = 0; i < argc; ++i) {
        if (!argv[i]) {
            parseContextLocal.error(0, "Null shader source string", "", "");
            parseContextLocal.recover();
            return 1;
        }
    }
    
    if (!strLen) {
        argv0len = (int) strlen(argv[0]);
        strLen   = &argv0len;
    }
    yyrestart(0);
    (&parseContextLocal)->AfterEOF = false;
    cpp->PaWhichStr = 0;
    cpp->PaArgv     = argv;
    cpp->PaArgc     = argc;
    cpp->PaStrLen   = strLen;
    cpp->notAVersionToken = 0;
    yylineno   = 1;
   
    if (*cpp->PaStrLen >= 0) {    
        int ret;
        #ifdef _WIN32
            ret = yyparse(parseContextLocal);            
        #else
            ret = yyparse((void*)(&parseContextLocal));
        #endif
        if (cpp->CompileError == 1 || parseContextLocal.recoveredFromError || parseContextLocal.numErrors > 0)
             return 1;
        else
             return 0;
    }
    else
        return 0;
}

void yyerror(char *s) 
{
    if (((TParseContext *)cpp->pC)->AfterEOF) {
        if (cpp->tokensBeforeEOF == 1) {
            GlobalParseContext->error(yylineno, "syntax error", "pre-mature EOF", s, "");
            GlobalParseContext->recover();
        }
    } else {
        GlobalParseContext->error(yylineno, "syntax error", yytext, s, "");
        GlobalParseContext->recover();
    }            
}

void PaReservedWord()
{
    GlobalParseContext->error(yylineno, "Reserved word.", yytext, "", "");
    GlobalParseContext->recover();
}

int PaIdentOrType(TString& id, TParseContext& parseContextLocal, TSymbol*& symbol)
{
    symbol = parseContextLocal.symbolTable.find(id);
    if (parseContextLocal.lexAfterType == false && symbol && symbol->isVariable()) {
        TVariable* variable = static_cast<TVariable*>(symbol);
        if (variable->isUserType()) {
            parseContextLocal.lexAfterType = true;
            return TYPE_NAME;
        }
    }
    
    return IDENTIFIER;
}

int PaParseComment(int &lineno, TParseContext& parseContextLocal)
{
    int transitionFlag = 0;
    int nextChar;
    
    while (transitionFlag != 2) {
        nextChar = yyinput();
        if (nextChar == '\n')
             lineno++;
        switch (nextChar) {
        case '*' :
            transitionFlag = 1;
            break;
        case '/' :  /* if star is the previous character, then it is the end of comment */
            if (transitionFlag == 1) {
                return 1 ;
            }
            break;
        case EOF :
            /* Raise error message here */
            parseContextLocal.error(yylineno, "End of shader found before end of comment.", "", "", "");
            GlobalParseContext->recover();
            return YY_NULL; 
        default :  /* Any other character will be a part of the comment */
            transitionFlag = 0;
        }
    }
    return 1;
}

extern "C" {

void CPPDebugLogMsg(const char *msg)
{
    ((TParseContext *)cpp->pC)->infoSink.debug.message(EPrefixNone, msg);
}

void CPPWarningToInfoLog(const char *msg)
{
    ((TParseContext *)cpp->pC)->infoSink.info.message(EPrefixWarning, msg, yylineno); 
}

void CPPShInfoLogMsg(const char *msg)
{
    ((TParseContext *)cpp->pC)->error(yylineno,"", "",msg,"");
    GlobalParseContext->recover();
}

void CPPErrorToInfoLog(char *msg)
{
    ((TParseContext *)cpp->pC)->error(yylineno,"syntax error", "",msg,"");
    GlobalParseContext->recover();
}

void SetLineNumber(int line)
{
    yylineno &= ~SourceLocLineMask;
    yylineno |= line;
}

void SetStringNumber(int string)
{
    yylineno = (string << SourceLocStringShift) | (yylineno & SourceLocLineMask);
}

int GetStringNumber(void)
{
    return yylineno >> 16;
}

int GetLineNumber(void)
{
    return yylineno & SourceLocLineMask;
}

void IncLineNumber(void)
{
    if ((yylineno & SourceLocLineMask) <= SourceLocLineMask)
        ++yylineno;
}

void DecLineNumber(void)
{
    if ((yylineno & SourceLocLineMask) > 0)
        --yylineno;
}

void HandlePragma(const char **tokens, int numTokens)
{    
    if (!strcmp(tokens[0], "optimize")) {
        if (numTokens != 4) {
            CPPShInfoLogMsg("optimize pragma syntax is incorrect");
            return;
        }
        
        if (strcmp(tokens[1], "(")) {
            CPPShInfoLogMsg("\"(\" expected after 'optimize' keyword");
            return;
        }
            
        if (!strcmp(tokens[2], "on"))
            ((TParseContext *)cpp->pC)->contextPragma.optimize = true;
        else if (!strcmp(tokens[2], "off"))
            ((TParseContext *)cpp->pC)->contextPragma.optimize = false;
        else {
            CPPShInfoLogMsg("\"on\" or \"off\" expected after '(' for 'optimize' pragma");
            return;
        }
        
        if (strcmp(tokens[3], ")")) {
            CPPShInfoLogMsg("\")\" expected to end 'optimize' pragma");
            return;
        }
    } else if (!strcmp(tokens[0], "debug")) {
        if (numTokens != 4) {
            CPPShInfoLogMsg("debug pragma syntax is incorrect");
            return;
        }
        
        if (strcmp(tokens[1], "(")) {
            CPPShInfoLogMsg("\"(\" expected after 'debug' keyword");
            return;
        }
            
        if (!strcmp(tokens[2], "on"))
            ((TParseContext *)cpp->pC)->contextPragma.debug = true;
        else if (!strcmp(tokens[2], "off"))
            ((TParseContext *)cpp->pC)->contextPragma.debug = false;
        else {
            CPPShInfoLogMsg("\"on\" or \"off\" expected after '(' for 'debug' pragma");
            return;
        }
        
        if (strcmp(tokens[3], ")")) {
            CPPShInfoLogMsg("\")\" expected to end 'debug' pragma");
            return;
        }
    } else {

#ifdef PRAGMA_TABLE
        //
        // implementation specific pragma
        // use ((TParseContext *)cpp->pC)->contextPragma.pragmaTable to store the information about pragma
        // For now, just ignore the pragma that the implementation cannot recognize
        // An Example of one such implementation for a pragma that has a syntax like
        // #pragma pragmaname(pragmavalue)
        // This implementation stores the current pragmavalue against the pragma name in pragmaTable.
        //        
        if (numTokens == 4 && !strcmp(tokens[1], "(") && !strcmp(tokens[3], ")")) {              
            TPragmaTable& pragmaTable = ((TParseContext *)cpp->pC)->contextPragma.pragmaTable;
            TPragmaTable::iterator iter;
            iter = pragmaTable.find(TString(tokens[0]));
            if (iter != pragmaTable.end()) {
                iter->second = tokens[2];
            } else {
                pragmaTable[tokens[0]] = tokens[2];
            }        
        } else if (numTokens >= 2) {
            TPragmaTable& pragmaTable = ((TParseContext *)cpp->pC)->contextPragma.pragmaTable;
            TPragmaTable::iterator iter;
            iter = pragmaTable.find(TString(tokens[0]));
            if (iter != pragmaTable.end()) {
                iter->second = tokens[1];
            } else {
                pragmaTable[tokens[0]] = tokens[1];
            }
        }
#endif // PRAGMA_TABLE
    }
}

void StoreStr(char *string)
{
    TString strSrc;
    strSrc = TString(string);

    ((TParseContext *)cpp->pC)->HashErrMsg = ((TParseContext *)cpp->pC)->HashErrMsg + " " + strSrc;
}

const char* GetStrfromTStr(void)
{
    cpp->ErrMsg = (((TParseContext *)cpp->pC)->HashErrMsg).c_str();
    return cpp->ErrMsg;
}

void ResetTString(void)
{
    ((TParseContext *)cpp->pC)->HashErrMsg = "";
}

TBehavior GetBehavior(const char* behavior)
{
    if (!strcmp("require", behavior))
        return EBhRequire;
    else if (!strcmp("enable", behavior))
        return EBhEnable;
    else if (!strcmp("disable", behavior))
        return EBhDisable;
    else if (!strcmp("warn", behavior))
        return EBhWarn;
    else {
        CPPShInfoLogMsg((TString("behavior '") + behavior + "' is not supported").c_str());
        return EBhDisable;
    }        
}

void  updateExtensionBehavior(const char* extName, const char* behavior)
{
    TBehavior behaviorVal = GetBehavior(behavior);
    TMap<TString, TBehavior>:: iterator iter;
    TString msg;
    
    // special cased for all extension
    if (!strcmp(extName, "all")) {
        if (behaviorVal == EBhRequire || behaviorVal == EBhEnable) {
            CPPShInfoLogMsg("extension 'all' cannot have 'require' or 'enable' behavior");  
            return;
        } else {
            for (iter =  ((TParseContext *)cpp->pC)->extensionBehavior.begin(); iter != ((TParseContext *)cpp->pC)->extensionBehavior.end(); ++iter)
                iter->second = behaviorVal;
        }        
    } else {
        iter = ((TParseContext *)cpp->pC)->extensionBehavior.find(TString(extName));
        if (iter == ((TParseContext *)cpp->pC)->extensionBehavior.end()) {
            switch (behaviorVal) {
            case EBhRequire:
                CPPShInfoLogMsg((TString("extension '") + extName + "' is not supported").c_str());  
                break;
            case EBhEnable:
            case EBhWarn:
            case EBhDisable:
                msg = TString("extension '") + extName + "' is not supported";
                ((TParseContext *)cpp->pC)->infoSink.info.message(EPrefixWarning, msg.c_str(), yylineno); 
                break;
            }
            return;
        } else
            iter->second = behaviorVal;
    }
}
        
}  // extern "C"

void setInitialState()
{
    yy_start = 1;
}
