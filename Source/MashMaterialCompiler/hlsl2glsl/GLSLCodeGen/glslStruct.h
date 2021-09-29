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
// Definition of GlslStruct
//=================================================================================================================================

#ifndef GLSL_STRUCT_H
#define GLSL_STRUCT_H

#include "glslCommon.h"

//=================================================================================================================================
/// GlslStruct
/// 
/// \brief This class provides the support for struct types.
//=================================================================================================================================
class GlslStruct 
{
public:
   // Struct member description, presently does not handle structs of structs
   struct member    
   {
      std::string name;
      std::string semantic;
	  std::string typeName;//Added to support structs within structs
      EGlslSymbolType type;
      int arraySize;
   };

   //=========================================================================================================   
   /// Constructor
   //=========================================================================================================   
   GlslStruct( const std::string &n) : name(n), isUniformBuffer(false) {}

   //=========================================================================================================   
   /// Destructor
   //=========================================================================================================   
   virtual ~GlslStruct() {}

   bool getIsUniformBuffer()const{return isUniformBuffer;}
   void setIsUniformBuffer(bool val){isUniformBuffer = val;}

   //=========================================================================================================   
   /// Get the structure name
   //=========================================================================================================   
   const std::string& getName();

   //=========================================================================================================   
   /// Add a new member to the structure
   //=========================================================================================================      
   void addMember( const member& m ) { memberList.push_back(m); }

   //=========================================================================================================   
   /// Get the structure member specified by which
   //=========================================================================================================      
   const member& getMember( int which )const { return memberList[which]; }

   //=========================================================================================================   
   /// Get the number of structure members
   //=========================================================================================================         
   int memberCount()const { return int(memberList.size()); }

   //=========================================================================================================   
   /// Get the declaration for the structure in GLSL as a string
   //=========================================================================================================            
   std::string getDecl();
private:

   // Members in structure
   std::vector<member> memberList;

   // Name of structure
   std::string name;

   //mangled name if this is used as a uniform buffer
   std::string mangledBufferName;

   bool isUniformBuffer;
};

#endif //GLSL_STRUCT_H
