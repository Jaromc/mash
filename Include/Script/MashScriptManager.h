//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SCRIPT_MANAGER_H_
#define _MASH_SCRIPT_MANAGER_H_

#include "MashEnum.h"
#include "MashReferenceCounter.h"
#include "MashLuaTypes.h"
#include "MashLuaTypesC.h"

namespace mash
{
	class MashLuaScript;
	class MashLuaScriptWrapper;
	class MashSceneNodeScriptHandler;
	class MashDevice;

	/*!
        Creates lua scripts and enables some interaction between the engine and scripts.
     
        Here is an example of handling input and custom functions.
     
        \code
     
        //action mapping used in the input manager
        const uint32 moveXAxis = aINPUTMAP_USER_REGION;
        const uint32 moveYAxis = moveXAxis + 1;
     
        //This will be the keymapping used in our script.
        //The string is what we will use in the script to access the key value.
        //The int32 is the input action we created above.
        sMashLuaKeyValue luaInputVals[] = {
            {"moveXAxis", moveXAxis},
            {"moveYAxis", moveYAxis},
            {0, 0}
        };
        scriptManager->SetLibInputValues(luaInputVals);
    
        //This creates a custom function for our script.
        //The string is what we will use in the script to call the function.
        //And the function pointer is the C callback function. 
		//RotateVectorToNodeLuaCallback must return an int that states the number of items
		//pushed onto the stack using a function such as MashLuaScript::Pushxxx().
        sMashLuaUserFunction luaExtraFunctions[] = {
            {"rotateVectorToNode", RotateVectorToNodeLuaCallback},
            {0, 0}
        };
        scriptManager->SetLibUserFunctions(luaExtraFunctions);
     
        \endcode
     
		Calling SetLibInputValues() will also call MashInputManager::EnabledInputHelpers(). 
        See that class for more information.
     
        For scene node handling, the following function are automatically called from
        scene nodes:
     
        "onUpdate" - Called at the start of each update.
        "onAttach" - Called when the script is first attached to a node.
        "onMouseEnter" - Called when the cursor hovers over the node in screen space.
        "onMouseExit" - Called when the cursor leaves the object in screen space.
     
        If these functions are found in a scene node script they will be called as the
        events happen. A script can contain any number of the callback functions.
	*/
	class MashScriptManager : public MashReferenceCounter
	{
	public:
		MashScriptManager():MashReferenceCounter(){}
		virtual ~MashScriptManager(){}

        //! Loads a new lua script from files.
        /*!
            The returned wrapper must be dropped when you are done with it.
         
            \param scriptName Script location.
            \return New script.
        */
		virtual MashLuaScriptWrapper* CreateLuaScript(const int8 *scriptName) = 0;

        //! Sets the values used for input handling from lua.
        /*!
            Last element must contain a null string. This MUST be called before any scripts are
            loaded if input is used.
         
            See class description for an example.
         
            \param value Input values.
        */
		virtual void SetLibInputValues(sMashLuaKeyValue *values) = 0;
        
        //! Adds custom functions to lua.
        /*!
            Last element must contain a null string. See class description for an example.
         
            \param functPtrList Custom function list.
        */
		virtual void SetLibUserFunctions(sMashLuaUserFunction *functPtrList) = 0;

        //! Creates a scene node script handler.
        /*!
            See class description for an example. The returned pointer must be dropped.
         
            \param script Script to be used for the node.
            \return New script handler. This can then be added to MashSceneNode::AddCallback().
        */
		virtual MashSceneNodeScriptHandler* CreateSceneNodeScriptHandler(MashLuaScriptWrapper *script) = 0;

        //! Called internally to remove a script.
        /*!
            \param script Script to remove.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS _DestroyLuaScript(MashLuaScript *script) = 0;
        
        //! Called internally to initialise this manager.
        /*!
            \param device Device.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS _Initialise(MashDevice *device) = 0;

		virtual int32 _CallUserFunction(void *state, uint32 functionId) = 0;
	};

    //! Called internally to create the script manager.
	_MASH_EXPORT MashScriptManager* CreateMashScriptManager();
}

#endif