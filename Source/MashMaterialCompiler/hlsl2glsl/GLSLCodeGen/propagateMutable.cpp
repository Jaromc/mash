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
// Implementation of TPropagateMutable
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================
#include "propagateMutable.h"


//=================================================================================================================================
//
//          Private Members Block
//
//=================================================================================================================================

//=========================================================================================================
/// Propage the mutable uniform qualifier throughout the symbols as needed
/// \param Node
///   Symbol node to propogate mutables
/// \param it
///   Pointer to traverser
//=========================================================================================================
void TPropagateMutable::traverseSymbol( TIntermSymbol *node, TIntermTraverser *it )
{
   TPropagateMutable* sit = static_cast<TPropagateMutable*>(it);

   if (sit->abort)
      return;

   if (sit->propagating && sit->id == node->getId())
   {
      node->getTypePointer()->changeQualifier( EvqMutableUniform );

   }
   else if (!sit->propagating && sit->fixedIds.find(node->getId()) == sit->fixedIds.end() )
   {
      if (node->getQualifier() == EvqMutableUniform)
      {
         sit->abort = true;
         sit->id = node->getId();
         sit->fixedIds.insert(sit->id);
      }
   }
}

//=================================================================================================================================
//
//          Public Members Block
//
//=================================================================================================================================


//=========================================================================================================
/// The TProgpogateMutable class implements only the symbol travere function (this member).  It goes
/// through the symbol and propogates the mutubale qualifier through the symbol nodes.  This function
/// initiates the propogation.
/// \param node
///   The root node of the tree
/// \param info
///   Infosink to log to
//=========================================================================================================
void TPropagateMutable::PropagateMutable( TIntermNode *node, TInfoSink &info )
{
   TPropagateMutable st(info);

   do
   {
      st.abort = false;
      node->traverse( &st);

      // If we aborted, try to type the node we aborted for
      if (st.abort)
      {
         st.propagating = true;
         st.abort = false;
         node->traverse( &st);
         st.propagating = false;
         st.abort = true;
      }
   } while (st.abort);

}
