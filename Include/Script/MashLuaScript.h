//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LUA_SCRIPT_H_
#define _MASH_LUA_SCRIPT_H_

#include "MashEnum.h"
#include "MashString.h"
#include "MashCompileSettings.h"

namespace mash
{
    /*!
        Lua script interface. These can be created from MashScriptManager.
     
        This should never be deleted directly. Instead call MashLuaScriptWrapper::Drop().
    */
	class _MASH_EXPORT MashLuaScript
	{
	public:
		MashLuaScript();

		virtual ~MashLuaScript();

        //! Returns true if the functon exists within the script.
        /*!
            \param functionName Function to check.
            \return True if the function exists, false otherwise.
        */
		bool GetFunctionExists(const int8 *functionName)const;

		//! Calls some function that is within this script.
		/*!
			\param functionName A valid function in the script.
			\param numParameters Number of parameters "functionName" excepts.
			\param numResults Number of results "functionName" returns.
			\return aMASH_OK if the operation was successful.
		*/
		eMASH_STATUS CallFunction(const int8 *functionName, int32 numParameters, int32 numResults);

		//! Returns the number of elements in the stack.
		/*!
			A scripted function can return a number of variables. A call to
            GetTop() can be used to determine the number of variables returned
            from such a function.
         
			\return Number of elements in the stack.
		*/
		int32 GetTop();
        
		//! Gets an int32 from the stack
		/*!
			This will convert a variable stored at a location in the stack
			to an int32. The result is then returned.

			\param index Valid index in the stack.
			\return Converted data.
		*/
		int32 GetInt(int32 index);
        
        //! Gets a f32 from the stack
		/*!
            This will convert a variable stored at a location in the stack
            to an f32. The result is then returned.
         
            \param index Valid index in the stack.
            \return Converted data.
         */
		f32 GetFloat(int32 index);
        
        //! Gets a bool from the stack
		/*!
            This will convert a variable stored at a location in the stack
            to an bool. The result is then returned.
         
            \param index Valid index in the stack.
            \return Converted data.
         */
		bool GetBool(int32 index);
        
        //! Gets a string from the stack
		/*!
            This will convert a variable stored at a location in the stack
            to a string. The result is then returned.
         
            \param index Valid index in the stack.
            \param out Converted data.
         */
		void GetString(int32 index, MashStringc &out);

		//! Pushes a f32 onto the stack.
		/*!
			\param value Data to push onto the stack.
		*/
		void PushFloat(f32 value);

		//! Pushes an int32 onto the stack.
		/*!
			\param value Data to push onto the stack.
		*/
		void PushInt(int32 value);

		//! Pushes a boolean onto the stack.
		/*!
			\param value Data to push onto the stack.
		*/
		void PushBool(bool value);

		//! Pushes a character array onto the stack.
		/*!
			\param value Data to push onto the stack.
		*/
		void PushString(const int8 *value);

		//! Pushes some user data onto the stack.
		/*!
			\param value Data to push onto the stack.
		*/
		void PushUserData(void* value);

		//! Returns a global variable from within the script.
		/*!
			Searches for a varibale of name and returns the value
			given to it.

			\param name Global variable name.
			\return Global variable data to return.
		*/
		int32 GetGlobalInt(const int8 *name);
		
		//! Returns a global variable from within the script.
		/*!
			Searches for a varibale of name and returns the value
			given to it.

            \param name Global variable name.
            \return Global variable data to return.
		*/
		f32 GetGlobalFloat(const int8 *name);
		
		//! Returns a global variable from within the script.
		/*!
			Searches for a varibale of name and returns the value
			given to it.

            \param name Global variable name.
            \return Global variable data to return.
		*/
		bool GetGlobalBool(const int8 *name);
		
		//! Returns a global variable from within the script.
		/*!
			Searches for a varibale of name and returns the value
			given to it.

            \param name Global variable name.
            \param out Global variable data to return.
		*/
		void GetGlobalString(const int8 *name, MashStringc &out);

        //! Sets a global int32.
        /*!
            \param name Global variable name.
            \param data Data to set.
        */
		void SetGlobalInt(const int8 *name, int32 data);
        
        //! Sets a global f32.
        /*!
            \param name Global variable name.
            \param data Data to set.
        */
		void SetGlobalFloat(const int8 *name, f32 data);
        
        //! Sets a global bool.
        /*!
            \param name Global variable name.
            \param data Data to set.
        */
		void SetGlobalBool(const int8 *name, bool data);
        
        //! Sets a global string.
        /*!
            \param name Global variable name.
            \param data Data to set.
        */
		void SetGlobalString(const int8 *name, const int8 *data);
	};
}

#endif