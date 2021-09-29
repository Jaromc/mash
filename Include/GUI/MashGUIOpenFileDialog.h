//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_FILE_EXPLORER_H_
#define _MASH_GUI_FILE_EXPLORER_H_

#include "MashReferenceCounter.h"
#include "MashGUIEventDispatch.h"

namespace mash
{
	/*!
		This is made up of several gui components.
	*/
	class MashGUIOpenFileDialog : public MashReferenceCounter, public MashGUIEventDispatch
	{
	public:
		MashGUIOpenFileDialog():MashReferenceCounter(){}

		virtual ~MashGUIOpenFileDialog(){}

		//! Clears the last file selected.
		/*!
			And next time the dialog opens the directory will start from the
			application directory.
		*/
		virtual void ClearLastFilepath() = 0;

		//! Closes this dialog.
		virtual void CloseDialog() = 0;

		//! Opens this dialog.
		virtual void OpenDialog() = 0;

		//! Gets the selected file name.
		/*!
			This is only a file name, not the file path.
			\return File name.
		*/
		virtual const int8* GetSelectedFileName()const = 0;

		//! Gets the selected file path.
		/*!
			This is a path only, not a file name.
			/return File path.
		*/
		virtual const int8* GetSelectedFileDirectory()const = 0;

		//! Is this dialog open.
		/*!
			\return True if this dialog is open, false otherwise.
		*/
		virtual bool IsOpen()const = 0;
	};
}

#endif