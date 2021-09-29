//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIReader.h"
#include "MashDevice.h"
#include "MashFileManager.h"
#include "MashVideo.h"
#include "MashGUIManager.h"
#include "MashGUIButton.h"
#include "MashGUICheckBox.h"
#include "MashGUIStaticText.h"
#include "MashGUITextBox.h"
#include "MashGUIMenuBar.h"
#include "MashGUISprite.h"
#include "MashGUIView.h"
#include "MashGUIWindow.h"
#include "MashGUIPopupMenu.h"

namespace mash
{
	void CMashGUIReader::LoadTreeItems(MashXMLReader *xmlReader, MashGUITree *tree, int32 parentId)
	{
		do
		{
			int32 itemValue = 0;
			xmlReader->GetAttributeInt("value", itemValue);
			int32 thisId = tree->AddItem(xmlReader->GetAttributeRaw("text"), parentId, itemValue);
			if (xmlReader->MoveToFirstChild("item"))
			{
				LoadTreeItems(xmlReader, tree, thisId);
				xmlReader->PopChild();
			}
		}while(xmlReader->MoveToNextSibling("item"));
	}

	void CMashGUIReader::LoadPopupItems(MashDevice *device, MashXMLReader *xmlReader, MashGUIPopupMenu *popup)
	{
		do
		{
			int32 val = 0;
			xmlReader->GetAttributeInt("value", val);
			const int8 *textVal = xmlReader->GetAttributeRaw("text");
			MashGUIPopupMenu *subMenu = 0;

			if (xmlReader->MoveToFirstChild("item"))
			{
				subMenu = device->GetGUIManager()->CreatePopupMenu();
				LoadPopupItems(device, xmlReader, subMenu);
				xmlReader->PopChild();
			}

			popup->AddItem(textVal, val, subMenu);

		}while(xmlReader->MoveToNextSibling("item"));
	}

	void CMashGUIReader::LoadComponent(MashDevice *device, MashXMLReader *xmlReader, MashGUIView *parent, MashGUILoadCallback *callback)
	{
		do
		{
			int32 guiType;
			MashStringc guiTypeAsString;
			MashStringc guiElementName;
			xmlReader->GetAttributeString("type", guiTypeAsString);
			guiType = GetGUITypeFromString(guiTypeAsString);

			xmlReader->GetAttributeString("name", guiElementName);

			MashGUIRect destinationRegion;

			xmlReader->MoveToFirstChild("destinationregion");
			xmlReader->MoveToFirstChild("left");
			xmlReader->GetAttributeFloat("scale", destinationRegion.left.scale);
			xmlReader->GetAttributeFloat("offset", destinationRegion.left.offset);
			xmlReader->PopChild();
			xmlReader->MoveToFirstChild("right");
			xmlReader->GetAttributeFloat("scale", destinationRegion.right.scale);
			xmlReader->GetAttributeFloat("offset", destinationRegion.right.offset);
			xmlReader->PopChild();
			xmlReader->MoveToFirstChild("top");
			xmlReader->GetAttributeFloat("scale", destinationRegion.top.scale);
			xmlReader->GetAttributeFloat("offset", destinationRegion.top.offset);
			xmlReader->PopChild();
			xmlReader->MoveToFirstChild("bottom");
			xmlReader->GetAttributeFloat("scale", destinationRegion.bottom.scale);
			xmlReader->GetAttributeFloat("offset", destinationRegion.bottom.offset);
			xmlReader->PopChild();
			xmlReader->PopChild();

			MashGUIComponent *newComponent = 0;
			switch(guiType)
			{
			case aGUI_VIEWPORT:
				{
					newComponent = device->GetGUIManager()->AddViewport(destinationRegion, parent);
					break;
				}
			case aGUI_BUTTON:
				{
					newComponent = device->GetGUIManager()->AddButton(destinationRegion, parent);
					MashGUIButton *button = (MashGUIButton*)newComponent;
					MashStringc tempString;
					xmlReader->GetAttributeString("text", tempString);
					button->SetText(tempString.GetCString());
					int32 val = 0;
					if (xmlReader->GetAttributeInt("isswitch", val))
						button->SetIsSwitch(val);

					break;
				}
			case aGUI_CHECK_BOX:
				{
					newComponent = device->GetGUIManager()->AddCheckBox(destinationRegion, parent);
					MashGUICheckBox *checkBox = (MashGUICheckBox*)newComponent;
					int32 val = 0;
					if (xmlReader->GetAttributeInt("checked", val))
						checkBox->SetChecked(val);

					break;
				}
			case aGUI_STATIC_TEXT:
				{
					newComponent = device->GetGUIManager()->AddStaticText(destinationRegion, parent);
					MashGUIStaticText *staticText = (MashGUIStaticText*)newComponent;
					MashStringc tempString;
					if (xmlReader->GetAttributeString("text", tempString))
						staticText->SetText(tempString.GetCString());

					int32 tempInt = 0;
					if (xmlReader->GetAttributeInt("wordwrap", tempInt))
						staticText->SetWordWrap((bool)tempInt);

					break;
				}
			case aGUI_TEXT_BOX:
				{
					newComponent = device->GetGUIManager()->AddTextBox(destinationRegion, parent);
					MashGUITextBox *textBox = (MashGUITextBox*)newComponent;
					MashStringc tempString;
					if (xmlReader->GetAttributeString("text", tempString))
						textBox->SetText(tempString.GetCString());

					if (xmlReader->GetAttributeString("textformat", tempString))
						textBox->SetTextFormat(GetGUITextFormatFromString(tempString));

					int32 tempInt = 1;
					int32 minVal = mash::math::MinInt32();
					int32 maxVal = mash::math::MaxInt32();

					xmlReader->GetAttributeInt("minnumber", minVal);
					xmlReader->GetAttributeInt("maxnumber", maxVal);
					textBox->SetNumberMinMax(minVal, maxVal);

					if (xmlReader->GetAttributeInt("precision", tempInt))
						textBox->SetFloatPrecision(tempInt);

					if (xmlReader->GetAttributeInt("usenumberbuttons", tempInt))
						textBox->SetNumberButtonState((bool)tempInt);

					break;
				}
			case aGUI_VIEW:
				{
					newComponent = device->GetGUIManager()->AddView(destinationRegion, parent);
					MashGUIView *newView = (MashGUIView*)newComponent;
					int32 tempBool = 1;
					if (xmlReader->GetAttributeInt("hscrollenabled", tempBool))
						newView->SetHorizontalScrollState(tempBool);

					if (xmlReader->GetAttributeInt("vscrollenabled", tempBool))
						newView->SetVerticalScrollState(tempBool);

					if (xmlReader->GetAttributeInt("renderbackground", tempBool))
						newView->SetRenderBackgroundState(tempBool);

					break;
				}
			case aGUI_WINDOW:
				{
					newComponent = device->GetGUIManager()->AddWindow(destinationRegion, parent);
					MashGUIWindow *newWindow = (MashGUIWindow*)newComponent;
					if (xmlReader->GetAttributeRaw("text"))
						newWindow->SetTitleBarText(xmlReader->GetAttributeRaw("text"));

					int32 tempBool = 1;
					if (xmlReader->GetAttributeInt("closebuttonenabled", tempBool))
						newWindow->EnableCloseButton((bool)tempBool);
					
					if (xmlReader->GetAttributeInt("minimizebuttonenabled", tempBool))
						newWindow->EnableMinimizeButton((bool)tempBool);

					if (xmlReader->GetAttributeInt("mousedragenabled", tempBool))
						newWindow->EnableMouseDrag((bool)tempBool);

					if (xmlReader->GetAttributeInt("hscrollenabled", tempBool))
						newWindow->GetView()->SetHorizontalScrollState((bool)tempBool);

					if (xmlReader->GetAttributeInt("vscrollenabled", tempBool))
						newWindow->GetView()->SetVerticalScrollState((bool)tempBool);

					if (xmlReader->GetAttributeInt("renderbackground", tempBool))
						newWindow->GetView()->SetRenderBackgroundState((bool)tempBool);

					if (xmlReader->GetAttributeInt("alwaysontop", tempBool))
						newWindow->SetAlwaysOnTop((bool)tempBool);

					break;
				}
			case aGUI_TREE:
				{
					newComponent = device->GetGUIManager()->AddTree(destinationRegion, parent);
					MashGUITree *tree = (MashGUITree*)newComponent;
					if (xmlReader->MoveToFirstChild("item"))
					{
						LoadTreeItems(xmlReader, tree);
						xmlReader->PopChild();
					}
					
					break;
				}
			case aGUI_SPRITE:
				{
					newComponent = device->GetGUIManager()->AddSprite(destinationRegion, parent);
					MashGUISprite *sprite = (MashGUISprite*)newComponent;
					const int8 *texturePath = xmlReader->GetAttributeRaw("texture");
					if (texturePath)
						sprite->GetSkin()->SetTexture(device->GetRenderer()->GetTexture(texturePath));

					xmlReader->GetAttributeInt("texturesourceleft", (int32&)sprite->GetSkin()->baseSource.left);
					xmlReader->GetAttributeInt("texturesourceright", (int32&)sprite->GetSkin()->baseSource.right);
					xmlReader->GetAttributeInt("texturesourcetop", (int32&)sprite->GetSkin()->baseSource.top);
					xmlReader->GetAttributeInt("texturesourcebottom", (int32&)sprite->GetSkin()->baseSource.bottom);
					break;	
				}
			case aGUI_MENUBAR:
				{
					newComponent = device->GetGUIManager()->AddMenuBar(parent, destinationRegion.left, destinationRegion.right);
					MashGUIMenuBar *menuBar = (MashGUIMenuBar*)newComponent;
					if (xmlReader->MoveToFirstChild("menuitem"))
					{
						do
						{
							MashGUIPopupMenu *popup = device->GetGUIManager()->CreatePopupMenu();
							if (xmlReader->MoveToFirstChild("item"))
							{
								LoadPopupItems(device, xmlReader, popup);
								xmlReader->PopChild();
							}

							int32 userValue = 0;
							xmlReader->GetAttributeInt(("value"), userValue);

							menuBar->AddItem(xmlReader->GetAttributeRaw("text"), popup, userValue);

						}while(xmlReader->MoveToNextSibling("menuitem"));

						xmlReader->PopChild();
					}
					break;
				}
			case aGUI_LISTBOX:
				{
					newComponent = device->GetGUIManager()->AddListBox(destinationRegion, parent);
					MashGUIListBox *listBox = (MashGUIListBox*)newComponent;

					int32 tempInt = 0;
					if (xmlReader->GetAttributeInt("itemheight", tempInt))
						listBox->SetItemHeight(tempInt);

					tempInt = 0;
					if (xmlReader->GetAttributeInt("iconsenabled", tempInt))
						listBox->EnableIcons((bool)tempInt);

					MashGUIRect tempRect;
					if (xmlReader->MoveToFirstChild("itemiconregion"))
					{
						if (xmlReader->MoveToFirstChild("left"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.left.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.left.offset);
							xmlReader->PopChild();
						}
						if (xmlReader->MoveToFirstChild("right"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.right.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.right.offset);
							xmlReader->PopChild();
						}

						if (xmlReader->MoveToFirstChild("top"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.top.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.top.offset);
							xmlReader->PopChild();
						}

						if (xmlReader->MoveToFirstChild("bottom"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.bottom.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.bottom.offset);
							xmlReader->PopChild();
						}
						xmlReader->PopChild();
						
						listBox->SetItemIconRegion(tempRect);
					}
					
					tempRect.Zero();

					if (xmlReader->MoveToFirstChild("itemtextregion"))
					{
						if (xmlReader->MoveToFirstChild("left"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.left.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.left.offset);
							xmlReader->PopChild();
						}
						if (xmlReader->MoveToFirstChild("right"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.right.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.right.offset);
							xmlReader->PopChild();
						}

						if (xmlReader->MoveToFirstChild("top"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.top.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.top.offset);
							xmlReader->PopChild();
						}

						if (xmlReader->MoveToFirstChild("bottom"))
						{
							xmlReader->GetAttributeFloat("scale", tempRect.bottom.scale);
							xmlReader->GetAttributeFloat("offset", tempRect.bottom.offset);
							xmlReader->PopChild();
						}
						xmlReader->PopChild();
						
						listBox->SetItemTextRegion(tempRect);
					}
	
					if (xmlReader->MoveToFirstChild("item"))
					{
						do
						{
							int32 itemValue = 0;
							mash::MashRectangle2 itemIconSourceRegion;
							const int8 *itemText = xmlReader->GetAttributeRaw("text");
							xmlReader->GetAttributeInt("value", itemValue);

							const int8 *itemIconPath = xmlReader->GetAttributeRaw("iconlocation");
							xmlReader->GetAttributeInt("iconsourceleft", (int32&)itemIconSourceRegion.left);
							xmlReader->GetAttributeInt("iconsourceright", (int32&)itemIconSourceRegion.right);
							xmlReader->GetAttributeInt("iconsourcetop", (int32&)itemIconSourceRegion.top);
							xmlReader->GetAttributeInt("iconsourcebottom", (int32&)itemIconSourceRegion.bottom);

							MashTexture *texture = 0;
							if (itemIconPath)
								texture = device->GetRenderer()->GetTexture(itemIconPath);

							listBox->AddItem(itemText, itemValue, texture, &itemIconSourceRegion);
						}while(xmlReader->MoveToNextSibling("item"));

						xmlReader->PopChild();
					}
					break;
				}
			};	

			if (newComponent)
			{
				newComponent->SetName(guiElementName);

				int32 tempInt = 0;
				if (xmlReader->GetAttributeInt("clippingenabled", tempInt))
					newComponent->SetClippingEnabled((bool)tempInt);
			
				if (xmlReader->GetAttributeInt("canhavefocus", tempInt))
					newComponent->SetCanHaveFocus((bool)tempInt);

				if (xmlReader->GetAttributeInt("renderenabled", tempInt))
					newComponent->SetRenderEnabled((bool)tempInt);

				if (xmlReader->GetAttributeInt("focuslock", tempInt))
					newComponent->_SetLockFocusWhenActivated((bool)tempInt);

				if (callback)
					callback->OnCreateComponent(newComponent);

				if (guiType == aGUI_WINDOW || guiType == aGUI_VIEW)
				{
					MashGUIView *newParent = 0;
					if (guiType == aGUI_VIEW)
						newParent = (MashGUIView*)newComponent;
					else if (guiType == aGUI_WINDOW)
						newParent = (MashGUIView*)((MashGUIWindow*)newComponent)->GetView();

					if (xmlReader->MoveToFirstChild("guielement"))
					{
						LoadComponent(device, xmlReader, newParent, callback);

						xmlReader->PopChild();
					}
				}
			}

		}while(xmlReader->MoveToNextSibling("guielement"));
	}

	eMASH_STATUS CMashGUIReader::Load(MashDevice *device, const MashStringc &filename, MashGUIView *root, MashGUILoadCallback *callback)
	{
		MashXMLReader *xmlReader = device->GetFileManager()->CreateXMLReader(filename.GetCString());
		if (xmlReader && xmlReader->MoveToFirstChild("guielement"))
		{
			do
			{
				LoadComponent(device, xmlReader, root, callback);
			}while(xmlReader->MoveToNextSibling("guielement"));

			xmlReader->PopChild();
		}

		if (xmlReader)
			xmlReader->Destroy();

		return aMASH_OK;
	}
}