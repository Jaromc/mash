//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_READER_H_
#define _C_MASH_GUI_READER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashString.h"
#include "MashList.h"
#include "MashXMLReader.h"
#include "MashGUILoadCallback.h"

namespace mash
{
	class MashDevice;
	class MashGUIView;
	class MashGUITree;
	class MashGUIPopupMenu;

	class CMashGUIReader : public MashReferenceCounter
	{
	private:
		void LoadTreeItems(MashXMLReader *xmlReader, MashGUITree *tree, int32 parentId = -1);
		void LoadComponent(MashDevice *device, MashXMLReader *xmlReader, MashGUIView *parent, MashGUILoadCallback *callback);
		void LoadPopupItems(MashDevice *device, MashXMLReader *xmlReader, MashGUIPopupMenu *popup);
	public:
		CMashGUIReader(){}
		~CMashGUIReader(){}

		eMASH_STATUS Load(MashDevice *device, const MashStringc &filename, MashGUIView *root = 0, MashGUILoadCallback *callback = 0);
	};
}

#endif