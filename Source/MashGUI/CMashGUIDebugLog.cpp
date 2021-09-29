//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIDebugLog.h"
#include "MashLog.h"

namespace mash
{
	CMashGUIDebugLog::CMashGUIDebugLog(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement,
			uint32 logFlags,
			uint32 maxMessageCount):CMashGUIStaticText(pGUIManager,
				pInputManager, pParent, destination, styleElement), m_logFlags(logFlags), m_maxMessageCount(maxMessageCount),
				m_logReceiverID(0)
	{
		#ifdef MASH_LOG_ENABLED
			m_logReceiverID = MashLog::Instance()->AddReceiver(MashLogEventFunctor(&CMashGUIDebugLog::OnMessage, this));
		#endif

		SetRenderBackground(true);
		SetWordWrap(true);
	}

	CMashGUIDebugLog::~CMashGUIDebugLog()
	{
		#ifdef MASH_LOG_ENABLED
			MashLog::Instance()->RemoveReceiver(m_logReceiverID);
		#endif
	}

	void CMashGUIDebugLog::OnMessage(const sLogEvent &e)
	{
		if (e.msg && (m_logFlags & e.level))
		{
			MashStringc msg;

			if (e.level & MashLog::aERROR_LEVEL_WARNING)
			{
				msg = MashStringc("Warning : ") + e.msg;
			}
			else if (e.level & MashLog::aERROR_LEVEL_ERROR)
			{
				msg = MashStringc("Error : ") + e.msg;
			}
			else if (e.level & MashLog::aERROR_LEVEL_INFORMATION)
			{
				msg = MashStringc("Information : ") + e.msg;
			}
			else
			{
				msg = MashStringc("Misc : ") + e.msg;
			}
			
			msg += "\n";
			m_messageLengthList.PushBack(msg.Size());

			if (m_messageLengthList.Size() > m_maxMessageCount)
			{
				MashStringc newText = GetText();
				newText.Erase(newText.Begin(), newText.Begin() + m_messageLengthList.Front());

				m_messageLengthList.PopFront();

				newText += msg;
				SetText(newText);
			}
			else
				AddText(msg);

			
		}
	}
}