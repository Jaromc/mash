//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIOpenFileDialog.h"
#include "MashGUIManager.h"
#include "MashFileStream.h"
#include "MashDevice.h"
#include "MashStringHelper.h"

namespace mash
{
	CMashGUIOpenFileDialog::CMashGUIOpenFileDialog(MashGUIManager *pGUIManager,
			const MashGUIRect &destination,
			int32 styleElement):MashGUIOpenFileDialog(), m_styleElement(styleElement), m_GUIManager(pGUIManager)
	{
		/*
			TODO : Move these into style/layout class
		*/
		MashGUIRect fileListRegion(MashGUIUnit(0.f, 10.0f), MashGUIUnit(0.f, 10.0f), 
			MashGUIUnit(1.0f, -10.0f), MashGUIUnit(0.6, 0.0f));

		MashGUIRect folderNameRegion(MashGUIUnit(0.0f, 10.0f), MashGUIUnit(0.7f, 0.0f), 
			MashGUIUnit(0.0f, 300.0f), MashGUIUnit(0.79f, 0.0f));

		MashGUIRect createFileButtonRegion(MashGUIUnit(0.0f, 310.0f), MashGUIUnit(0.7f, 0.0f), 
			MashGUIUnit(0.0f, 400.0f), MashGUIUnit(0.79f, 0.0f));

		MashGUIRect okButtonRegion(MashGUIUnit(0.0, 10.0f), MashGUIUnit(0.85f, 0.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.95f, 0.0f));

		MashGUIRect cancelButtonRegion(MashGUIUnit(0.0, 110.0f), MashGUIUnit(0.85f, 0.0f), 
			MashGUIUnit(0.0f, 200.0f), MashGUIUnit(0.95f, 0.0f));

		m_dialogWindow = pGUIManager->AddWindow(destination);
		m_dialogWindow->GetView()->SetHorizontalScrollState(false);
		m_dialogWindow->GetView()->SetVerticalScrollState(false);
		m_dialogWindow->SetLockFocusWhenActivated(true);

		m_pOkButton = pGUIManager->AddButton(okButtonRegion, m_dialogWindow->GetView());
		m_pOkButton->SetText("Ok");
		m_pCancelButton = pGUIManager->AddButton(cancelButtonRegion, m_dialogWindow->GetView());
		m_pCancelButton->SetText("Cancel");
		m_pCreateFileButton = pGUIManager->AddButton(createFileButtonRegion, m_dialogWindow->GetView());
		m_pCreateFileButton->SetText("Create File");

		m_pFileList = pGUIManager->AddListBox(fileListRegion, m_dialogWindow->GetView());
		m_pFileList->EnableIcons(true);

		m_pCreateFileTextBox = pGUIManager->AddTextBox(folderNameRegion, m_dialogWindow->GetView());
		m_pCreateFileTextBox->SetCanHaveFocus(true);

		m_pFileList->RegisterReceiver(aGUIEVENT_LB_SELECTION_CONFIRMED, MashGUIEventFunctor(&CMashGUIOpenFileDialog::OnLBSelection, this));
		m_pFileList->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&CMashGUIOpenFileDialog::OnLBSelectionChange, this));
		m_pOkButton->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&CMashGUIOpenFileDialog::OnSelectFile, this));
		m_pCancelButton->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&CMashGUIOpenFileDialog::OnCancelFile, this));
		m_pCreateFileButton->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&CMashGUIOpenFileDialog::OnCreateFile, this));

		MashGUIStyle *activeStyle = pGUIManager->GetActiveGUIStyle();
		const f32 maxCharacterHeight = activeStyle->GetFont()->GetMaxCharacterHeight();
		const f32 textBuffer = 5.0f;
		const f32 listBoxItemHeight = maxCharacterHeight + (textBuffer * 2);
		m_pFileList->SetItemHeight(listBoxItemHeight);
		MashGUIRect iconRegion(MashGUIUnit(0.0f, textBuffer), MashGUIUnit(0.0f, textBuffer), 
			MashGUIUnit(0.0f, textBuffer + maxCharacterHeight), MashGUIUnit(0.0f, textBuffer + maxCharacterHeight));

		MashGUIRect textRegion = iconRegion;
		textRegion.left.offset = iconRegion.right.offset + textBuffer;
		textRegion.right.offset = -textBuffer;
		textRegion.right.scale = 1.0f;

		m_pFileList->SetItemIconRegion(iconRegion);
		m_pFileList->SetItemTextRegion(textRegion);

		CloseDialog();
	}

	CMashGUIOpenFileDialog::~CMashGUIOpenFileDialog()
	{
		if (m_dialogWindow)
		{
			m_dialogWindow->Destroy();
			m_dialogWindow = 0;
		}
	}

	bool CMashGUIOpenFileDialog::IsOpen()const
	{
		return m_dialogWindow->GetRenderEnabled();
	}

	void CMashGUIOpenFileDialog::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		m_pOkButton->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		m_pCancelButton->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		m_pFileList->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		m_pCreateFileTextBox->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		m_dialogWindow->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		m_pCreateFileButton->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
	}

	void CMashGUIOpenFileDialog::ClearLastFilepath()
	{
		m_sFileName = "";
		m_sDirectory = "";
	}

	void CMashGUIOpenFileDialog::DisplayFolderContentsHelper(const int8 *sLocalFolderName)
	{
		MashStringc completeDir = m_sDirectory;
		if (completeDir[completeDir.Size() - 1] != '/')
		{
			completeDir += '/';
		}

		completeDir += sLocalFolderName;

		SetActiveFolder(completeDir.GetCString());
	}

	void CMashGUIOpenFileDialog::SetActiveFolder(const int8 *sAbsoluteFolderDir)
	{
		if ((m_sDirectory != sAbsoluteFolderDir) && MashDevice::StaticDevice->GetFileManager()->APISetWorkingDirectory(sAbsoluteFolderDir))
		{
			MashDevice::StaticDevice->GetFileManager()->APIGetCurrentDirectory(m_sDirectory);
			m_fileAttribs.Clear();
			m_pFileList->ClearAllItems();

			MashDevice::StaticDevice->GetFileManager()->APIGetDirectoryStructure("./", m_fileAttribs);

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *fileIcon = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_FILE_ICON);
			MashGUISkin *folderIcon = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_FOLDER_ICON);

			for(uint32 i = 0; i < m_fileAttribs.Size(); ++i)
			{
				if (m_fileAttribs[i].flags & MashFileManager::aFILE_ATTRIB_DIR ||
					m_fileAttribs[i].flags & MashFileManager::aFILE_ATTRIB_PARENT_DIR)
				{
					m_pFileList->AddItem(m_fileAttribs[i].relativeFilePath.GetCString(), i, folderIcon->GetTexture(), &folderIcon->baseSource);
				}
				else
				{
					m_pFileList->AddItem(m_fileAttribs[i].relativeFilePath.GetCString(), i, fileIcon->GetTexture(), &fileIcon->baseSource);
				}
			}
		}
	}

	void CMashGUIOpenFileDialog::OpenDialog()
	{
		m_dialogWindow->SetRenderEnabled(true);
		m_GUIManager->SetFocusedElement(m_dialogWindow);

		if (!m_originalFileDirectory.Empty() && !m_sDirectory.Empty())
		{
			MashDevice::StaticDevice->GetFileManager()->APISetWorkingDirectory(m_sDirectory.GetCString());
		}
		else
		{
			MashStringc curDir = "";
			MashDevice::StaticDevice->GetFileManager()->APIGetCurrentDirectory(curDir);
			m_originalFileDirectory = curDir;
			SetActiveFolder(curDir.GetCString());
		}

	}

	void CMashGUIOpenFileDialog::CloseDialog()
	{
		m_dialogWindow->SetRenderEnabled(false);

		if (!m_originalFileDirectory.Empty())
		{
			MashDevice::StaticDevice->GetFileManager()->APISetWorkingDirectory(m_originalFileDirectory.GetCString());
		}
	}

	void CMashGUIOpenFileDialog::OnLBSelection(const sGUIEvent &eventData)
	{
		int32 iIndex = m_pFileList->GetItemUserValue(m_pFileList->GetSelectedItemId());

		if (m_fileAttribs[iIndex].flags & MashFileManager::aFILE_ATTRIB_DIR)
		{
			DisplayFolderContentsHelper(m_fileAttribs[iIndex].relativeFilePath.GetCString());
		}
		else
		{
			m_sFileName = m_fileAttribs[iIndex].relativeFilePath;
			CloseDialog();

			sGUIEvent newGUIMsg;
			
			newGUIMsg.GUIEvent = aGUIEVENT_OPENFILE_SELECTED;
			newGUIMsg.component = this;
			ImmediateBroadcast(newGUIMsg);
		}
	}

	void CMashGUIOpenFileDialog::OnLBSelectionChange(const sGUIEvent &eventData)
	{
		int32 iIndex = m_pFileList->GetItemUserValue(m_pFileList->GetSelectedItemId());
		if (!(m_fileAttribs[iIndex].flags & MashFileManager::aFILE_ATTRIB_DIR))
		{
			m_sFileName = m_fileAttribs[iIndex].relativeFilePath;
		}
		else
		{
			m_sFileName = "";
		}

		MashStringc completeDir;
        ConcatenatePaths(m_sDirectory.GetCString(), m_sFileName.GetCString(), completeDir);

		m_pCreateFileTextBox->SetText(completeDir.GetCString());
	}

	void CMashGUIOpenFileDialog::OnSelectFile(const sGUIEvent &eventData)
	{
		int32 iIndex = m_pFileList->GetItemUserValue(m_pFileList->GetSelectedItemId());

		if (iIndex < m_fileAttribs.Size())
		{
			if (m_fileAttribs[iIndex].flags & MashFileManager::aFILE_ATTRIB_DIR)
			{
				DisplayFolderContentsHelper(m_fileAttribs[iIndex].relativeFilePath.GetCString());
			}
			else
			{
				m_sFileName = m_fileAttribs[iIndex].relativeFilePath;
				CloseDialog();

				sGUIEvent newGUIMsg;
				
				newGUIMsg.GUIEvent = aGUIEVENT_OPENFILE_SELECTED;
				newGUIMsg.component = this;
				ImmediateBroadcast(newGUIMsg);
			}
		}
		else
		{
			CloseDialog();
		}
	}

	void CMashGUIOpenFileDialog::OnCreateFile(const sGUIEvent &eventData)
	{
		if (m_pCreateFileTextBox->GetText().Empty())
			return;

		 MashFileStream *fileStream = MashDevice::StaticDevice->GetFileManager()->CreateFileStream();
        
		MashStringc fileName;
		GetFileNameAndExtention(m_pCreateFileTextBox->GetText().GetCString(), fileName);
        MashStringc completeDir;
        ConcatenatePaths(m_sDirectory.GetCString(), fileName.GetCString(), completeDir);

		//save an empty file
		fileStream->SaveFile(completeDir.GetCString(), aFILE_IO_TEXT);
        fileStream->Destroy();

		//update displayed data
		m_sDirectory.Clear();//force clear the dir so it updates
		completeDir.Clear();
		MashDevice::StaticDevice->GetFileManager()->APIGetCurrentDirectory(completeDir);
		SetActiveFolder(completeDir.GetCString());
	}

	void CMashGUIOpenFileDialog::OnCancelFile(const sGUIEvent &eventData)
	{
		CloseDialog();
	}
}