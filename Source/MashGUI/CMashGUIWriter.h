//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_WRITER_H_
#define _C_MASH_GUI_WRITER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashString.h"
#include "MashList.h"
#include "MashXMLWriter.h"
#include "MashGUITree.h"
namespace mash
{
	class MashDevice;
	class MashGUIComponent;
	class MashGUIPopupMenu;

	class CMashGUIWriter : public MashReferenceCounter
	{
	private:
		void SavePopupMenu(MashXMLWriter *xmlWriter, MashGUIPopupMenu *popup);
		void SaveTreeItems(MashXMLWriter *xmlWriter, const MashArray<MashGUITree::sItemData> &items);
		void SaveComponent(MashXMLWriter *xmlWriter, const MashGUIComponent *component);
	public:
		CMashGUIWriter(){}
		~CMashGUIWriter(){}

		eMASH_STATUS Save(MashDevice *device, const MashStringc &filename, MashGUIComponent *root, bool saveRoot);
	};
}

#endif