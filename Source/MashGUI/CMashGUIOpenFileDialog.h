//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_GUI_FILE_EXPLORER_H_
#define _CMASH_GUI_FILE_EXPLORER_H_

#include "MashGUIOpenFileDialog.h"
#include "MashGUIButton.h"
#include "MashGUIListBox.h"
#include "MashGUITextBox.h"
#include "MashFileManager.h"
#include "MashGUIStaticText.h"
#include "MashGUIWindow.h"
namespace mash
{
	class CMashGUIOpenFileDialog : public MashGUIOpenFileDialog
	{
	private:
		MashGUIButton *m_pOkButton;
		MashGUIButton *m_pCancelButton;
		MashGUIButton *m_pCreateFileButton;
		MashGUIListBox *m_pFileList;
		MashGUITextBox *m_pCreateFileTextBox;
		MashGUIWindow *m_dialogWindow;

		MashStringc m_sFileName;
		MashStringc m_sDirectory;

		MashGUIManager *m_GUIManager;

		MashArray<MashFileManager::sFileAttributes> m_fileAttribs;

		int32 m_styleElement;
		MashStringc m_originalFileDirectory;

		void DisplayFolderContentsHelper(const int8 *sLocalFolderName);

		void OnLBSelection(const sGUIEvent &eventData);
		void OnLBSelectionChange(const sGUIEvent &eventData);
		void OnSelectFile(const sGUIEvent &eventData);
		void OnCancelFile(const sGUIEvent &eventData);
		void OnCreateFile(const sGUIEvent &eventData);
	public:
		CMashGUIOpenFileDialog(MashGUIManager *pGUIManager,
			const MashGUIRect &destination,
			int32 styleElement);

		virtual ~CMashGUIOpenFileDialog();

		void ClearLastFilepath();
		void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);
		void SetActiveFolder(const int8 *sAbsoluteFolderDir);
		void CloseDialog();
		void OpenDialog();
		bool IsOpen()const;
		const int8* GetSelectedFileName()const;
		const int8* GetSelectedFileDirectory()const;
	};

	inline const int8* CMashGUIOpenFileDialog::GetSelectedFileName()const
	{
		return m_sFileName.GetCString();
	}

	inline const int8* CMashGUIOpenFileDialog::GetSelectedFileDirectory()const
	{
		return m_sDirectory.GetCString();
	}
}

#endif