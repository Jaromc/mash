//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_DEBUG_LOG_H_
#define _C_MASH_GUI_DEBUG_LOG_H_

#include "CMashGUIStaticText.h"

namespace mash
{
	class CMashGUIDebugLog : public CMashGUIStaticText
	{
	private:
		void OnMessage(const sLogEvent &e);
		uint32 m_logFlags;
		uint32 m_logReceiverID;
		uint32 m_maxMessageCount;
		MashList<uint32> m_messageLengthList;
	public:
		CMashGUIDebugLog(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement,
			uint32 logFlags,
			uint32 maxMessageCount);

		~CMashGUIDebugLog();
	};
}

#endif