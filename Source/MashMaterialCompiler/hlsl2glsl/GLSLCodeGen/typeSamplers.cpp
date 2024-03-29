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
// Implementation of TSamplerTraverser
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================
#include "typeSamplers.h"



//=========================================================================================================
/// Traverse symbol node
/// \param node
///   Symbol node to to traverse
/// \param it
///   Pointer to traverser
//=========================================================================================================
void TSamplerTraverser::traverseSymbol( TIntermSymbol *node, TIntermTraverser *it )
{
   TSamplerTraverser* sit = static_cast<TSamplerTraverser*>(it);

   if (sit->abort)
      return;

   if (sit->typing && sit->id == node->getId())
   {
      TType* type = node->getTypePointer();

      // Technically most of these should never happen
      if ( type->isArray())
         type->setType( sit->sampType, type->getNominalSize(), type->isMatrix(), true, type->getArraySize());
      else
         type->setType( sit->sampType, type->getNominalSize(), type->isMatrix(), false);
   }
}

//=========================================================================================================
/// Traverse binary node
/// \param preVisit
///   Used only by base class, unused here.
/// \param node
///   Binary node to to traverse
/// \param it
///   Pointer to traverser
//=========================================================================================================
bool TSamplerTraverser::traverseBinary( bool preVisit, TIntermBinary *node, TIntermTraverser *it )
{
   TSamplerTraverser* sit = static_cast<TSamplerTraverser*>(it);

   if (sit->abort)
      return false;

   switch (node->getOp())
   {
   case EOpAssign:
      //believed to be disallowed on samplers...
      break;


   case EOpIndexDirect:
   case EOpIndexIndirect:
      //not planning to support arrays of samplers with different types 
      break;

   case EOpIndexDirectStruct:
      //Structs in samplers not supported
      break;

   default:
      break;
   }


   return !(sit->abort);
}

//=========================================================================================================
/// Traverse unary node
/// \param preVisit
///   Used only by base class, unused here.
/// \param node
///   Unary node to to traverse
/// \param it
///   Pointer to traverser
//=========================================================================================================
bool TSamplerTraverser::traverseUnary( bool preVisit, TIntermUnary *node, TIntermTraverser *it)
{
   TSamplerTraverser* sit = static_cast<TSamplerTraverser*>(it);


   return !(sit->abort);
}

//=========================================================================================================
/// Traverse selection node
/// \param preVisit
///   Used only by base class, unused here.
/// \param node
///   Selection node to to traverse
/// \param it
///   Pointer to traverser
//=========================================================================================================
bool TSamplerTraverser::traverseSelection( bool preVisit, TIntermSelection *node, TIntermTraverser *it)
{
   //TODO: might need to run down this rat hole for ?: operator
   TSamplerTraverser* sit = static_cast<TSamplerTraverser*>(it);

   return !(sit->abort);
}

//=========================================================================================================
/// Traverse aggregate node
/// \param preVisit
///   Used only by base class, unused here.
/// \param node
///   Aggregate node to to traverse
/// \param it
///   Pointer to traverser
//=========================================================================================================
bool TSamplerTraverser::traverseAggregate( bool preVisit, TIntermAggregate *node, TIntermTraverser *it)
{
   TSamplerTraverser* sit = static_cast<TSamplerTraverser*>(it);
   TInfoSink &infoSink = sit->infoSink;

   if (sit->abort)
      return false;

   if (! (sit->typing) )
   {
      switch (node->getOp())
      {
      
      case EOpFunction:
         // Store the current function name to use to setup the parameters
         sit->currentFunction = node->getName().c_str(); 
         break;

      case EOpParameters:
         // Store the parameters to the function in the map
         sit->functionMap[sit->currentFunction.c_str()] = &(node->getSequence());
         break;

      case EOpFunctionCall:
         {
            // This is a bit tricky.  Find the function in the map.  Loop over the parameters
            // and see if the parameters have been marked as a typed sampler.  If so, propagate
            // the sampler type to the caller
            if ( sit->functionMap.find ( node->getName().c_str() ) != sit->functionMap.end() )
            {
               // Get the sequence of function parameters
               TIntermSequence *funcSequence = sit->functionMap[node->getName().c_str()];
               
               // Get the sequence of parameters being passed to function
               TIntermSequence &sequence = node->getSequence();

               // Grab iterators to both sequences
               TIntermSequence::iterator it = sequence.begin();
               TIntermSequence::iterator funcIt = funcSequence->begin();

               assert ( sequence.size() == funcSequence->size() );
               if ( sequence.size() == funcSequence->size() )
               {
                  while ( it != sequence.end() )
                  {
                     TIntermSymbol *sym = (*it)->getAsSymbolNode();
                     TIntermSymbol *funcSym = (*funcIt)->getAsSymbolNode();
                     
                     if ( sym != NULL && funcSym != NULL)
                     {
                        // If the parameter is generic, and the sampler to which
                        // it is being passed has been marked, propogate its sampler
                        // type to the caller.
                        if ( sym->getBasicType() == EbtSamplerGeneric &&
                             funcSym->getBasicType() != EbtSamplerGeneric )
                        {
                           sit->typeSampler ( sym, funcSym->getBasicType() );
                        }
                     }
                     it++;
                     funcIt++;
                  }
               }
            }
         }
         break;

         //HLSL texture functions
      case EOpTex1D:
      case EOpTex1DProj:
      case EOpTex1DLod:
      case EOpTex1DBias:
      case EOpTex1DGrad:
         {
            TIntermSequence &sequence = node->getSequence();
            assert( sequence.size());
            TIntermTyped *sampArg = sequence[0]->getAsTyped();
            if ( sampArg)
            {
               if (sampArg->getBasicType() == EbtSamplerGeneric)
               {
                  //type the sampler
                  sit->typeSampler( sampArg, EbtSampler1D);
               }
               else if (sampArg->getBasicType() != EbtSampler1D)
               {
                  //We have a sampler mismatch error
                  infoSink.info << "Error: " << node->getLine() << ": Sampler type mismatch, likely using a generic sampler as two types\n";
               }
            }
            else
            {
               assert(0);
            }

         }
         // We need to continue the traverse here, because the calls could be nested 
         break;


      case EOpTex2D:
      case EOpTex2DProj:
      case EOpTex2DLod:
      case EOpTex2DBias:
      case EOpTex2DGrad:
         {
            TIntermSequence &sequence = node->getSequence();
            assert( sequence.size());
            TIntermTyped *sampArg = sequence[0]->getAsTyped();
            if ( sampArg)
            {
               if (sampArg->getBasicType() == EbtSamplerGeneric)
               {
                  //type the sampler
                  sit->typeSampler( sampArg, EbtSampler2D);
               }
               else if (sampArg->getBasicType() != EbtSampler2D)
               {
                  //We have a sampler mismatch error
                  infoSink.info << "Error: " << node->getLine() << ": Sampler type mismatch, likely using a generic sampler as two types\n";
               }
            }
            else
            {
               assert(0);
            }

         }
         // We need to continue the traverse here, because the calls could be nested 
         break;

      case EOpTex3D:
      case EOpTex3DProj:
      case EOpTex3DLod:
      case EOpTex3DBias:
      case EOpTex3DGrad:
         {
            TIntermSequence &sequence = node->getSequence();
            assert( sequence.size());
            TIntermTyped *sampArg = sequence[0]->getAsTyped();
            if ( sampArg)
            {
               if (sampArg->getBasicType() == EbtSamplerGeneric)
               {
                  //type the sampler
                  sit->typeSampler( sampArg, EbtSampler3D);
               }
               else if (sampArg->getBasicType() != EbtSampler3D)
               {
                  //We have a sampler mismatch error
                  infoSink.info << "Error: " << node->getLine() << ": Sampler type mismatch, likely using a generic sampler as two types\n";
               }
            }
            else
            {
               assert(0);
            }

         }
         // We need to continue the traverse here, because the calls could be nested 
         break;

      case EOpTexCube:
      case EOpTexCubeProj:
      case EOpTexCubeLod:
      case EOpTexCubeBias:
      case EOpTexCubeGrad:
         {
            TIntermSequence &sequence = node->getSequence();
            assert( sequence.size());
            TIntermTyped *sampArg = sequence[0]->getAsTyped();
            if ( sampArg)
            {
               if (sampArg->getBasicType() == EbtSamplerGeneric)
               {
                  //type the sampler
                  sit->typeSampler( sampArg, EbtSamplerCube);
               }
               else if (sampArg->getBasicType() != EbtSamplerCube)
               {
                  //We have a sampler mismatch error
                  infoSink.info << "Error: " << node->getLine() << ": Sampler type mismatch, likely using a generic sampler as two types\n";
               }
            }
            else
            {
               assert(0);
            }

         }
         // We need to continue the traverse here, because the calls could be nested 
         break;

      default: 
         break;
      }
   }


   return !(sit->abort);
}

//=========================================================================================================
/// Traverse loop node
/// \param preVisit
///   Used only by base class, unused here.
/// \param node
///   Loop node to to traverse
/// \param it
///   Pointer to traverser
//=========================================================================================================
bool TSamplerTraverser::traverseLoop( bool preVisit, TIntermLoop *node, TIntermTraverser *it)
{
   TSamplerTraverser* sit = static_cast<TSamplerTraverser*>(it);

   return !(sit->abort);
}

//=========================================================================================================
/// Traverse branch node
/// \param preVisit
///   Used only by base class, unused here.
/// \param node
///   Branch node to to traverse
/// \param it
///   Pointer to traverser
//=========================================================================================================
bool TSamplerTraverser::traverseBranch( bool preVisit, TIntermBranch *node,  TIntermTraverser *it)
{
   TSamplerTraverser* sit = static_cast<TSamplerTraverser*>(it);

   return !(sit->abort);
}

//=========================================================================================================
/// Set the type for the sampler
/// \param node
///   Sampler node to set
/// \param samp
///   Sampler type to set node to
//=========================================================================================================
void TSamplerTraverser::typeSampler( TIntermTyped *node, TBasicType samp )
{
   TIntermSymbol *symNode = node->getAsSymbolNode();

   if ( !symNode)
   {
      //TODO: add logic to handle sampler arrays and samplers as struct members

      //Don't try typing this one, it is a complex expression
      TIntermBinary *biNode = node->getAsBinaryNode();

      if ( biNode )
      {
         switch (biNode->getOp())
         {
         case EOpIndexDirect:
         case EOpIndexIndirect:
            infoSink.info << "Warning: " << node->getLine() <<  ": typing of sampler arrays presently unsupported\n";
            break;

         case EOpIndexDirectStruct:
            infoSink.info << "Warning: " << node->getLine() <<  ": typing of samplers as struct members presently unsupported\n";
            break;
         }
      }
      else
      {
         infoSink.info << "Warning: " << node->getLine() <<  ": unexpected expression type for sampler, cannot type\n";
      }
      abort = false;
   }
   else
   {
      // We really have something to type, abort this traverse and activate typing
      abort = true;
      id = symNode->getId();
      sampType = samp;
   }
}

//=========================================================================================================
/// Traverse the entire tree, typing all samplers that need to be typed
/// \param node
///   Root node of the tree
/// \param info
///   Infosink to hold log
//=========================================================================================================
void TSamplerTraverser::TypeSamplers( TIntermNode *node, TInfoSink &info )
{
   TSamplerTraverser st(info);

   do
   {
      st.abort = false;
      node->traverse( &st);

      // If we aborted, try to type the node we aborted for
      if (st.abort)
      {
         st.typing = true;
         st.abort = false;
         node->traverse( &st);
         st.typing = false;
         st.abort = true;
      }
   } while (st.abort);

}
