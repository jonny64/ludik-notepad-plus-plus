//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
enum FOLDER_TYPE {MODEL, CONTENT, PRESENTATION};  

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	static ShortcutKey F6 = {false, false, false, VK_F6};
	
	static ShortcutKey ALT_G = {false, true, false, 0x47};
	static ShortcutKey ALT_M = {false, true, false, 0x4D};
	
	static ShortcutKey ALT_S = {false, true, false, 0x53};
	static ShortcutKey ALT_W = {false, true, false, 0x57};

	static ShortcutKey ALT_U = {false, true, false, 0x55};
	static ShortcutKey ALT_D = {false, true, false, 0x44};
	static ShortcutKey ALT_C = {false, true, false, 0x43};


	//--------------------------------------------//
	//-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
	//--------------------------------------------//
	// with function :
	// setCommand(int index,                      // zero based number to indicate the order of command
	//            TCHAR *commandName,             // the command name that you want to see in plugin menu
	//            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
	//            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
	//            bool check0nInit                // optional. Make this menu item be checked visually
	//            );
	setCommand(0, TEXT("Model"), toggleModel, &F6, false);
	setCommand(2, TEXT("Content/get_item"), getItem, &ALT_G, false);
	setCommand(3, TEXT("Presentation/draw_item"), drawItem, &ALT_M, false);
	setCommand(5, TEXT("Presentation/select"), select, &ALT_S, false);
	setCommand(6, TEXT("Presentation/draw"), draw, &ALT_W, false);
	setCommand(8, TEXT("Content/do_update"), doUpdate, &ALT_U, false);
	setCommand(9, TEXT("Content/do_delete"), doDelete, &ALT_D, false);
	setCommand(10, TEXT("Content/do_create"), doCreate, &ALT_C, false);
	
	const int LINES_FROM_TOP = 9;
	scintillaMsg(SCI_SETYCARETPOLICY, CARET_SLOP | CARET_STRICT, LINES_FROM_TOP);
}

//
// clean up
//
void commandMenuCleanUp()
{
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
	if (index >= nbFunc)
	return false;

	if (!pFunc)
	return false;

	lstrcpy(funcItem[index]._itemName, cmdName);
	funcItem[index]._pFunc = pFunc;
	funcItem[index]._init2Check = check0nInit;
	funcItem[index]._pShKey = sk;

	return true;
}

HWND getCurrentScintilla()
{
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
	return NULL;
	if (which == 0)
	{
		return nppData._scintillaMainHandle;
	} else
	{
		return nppData._scintillaSecondHandle;
	}
	return nppData._scintillaMainHandle;
}

int searchScintilla(const wchar_t* substr)
{
	// Scintilla still requires multibyte strings
	char short_substr[MAX_PATH] = {0};
	wcstombs(short_substr, substr, MAX_PATH);

	int searchFlags = SCFIND_REGEXP | SCFIND_POSIX;
	int documentEnd = scintillaMsg(SCI_GETLENGTH);
	
	scintillaMsg(SCI_SETTARGETSTART, 0);
	scintillaMsg(SCI_SETTARGETEND, documentEnd);
	scintillaMsg(SCI_SETSEARCHFLAGS, searchFlags);
	int posFind = scintillaMsg(SCI_SEARCHINTARGET, strlen(short_substr), (LPARAM)&short_substr);
	
	return posFind;
}

UINT scintillaMsg(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND curScintilla = getCurrentScintilla();
	return ::SendMessage(curScintilla, message, wParam, lParam);
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//

FOLDER_TYPE getCurrentFolder()
{
	
	wchar_t currentDir[MAX_PATH] = {0};
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM)&currentDir);
	
	if (NULL != wcsstr(currentDir, L"Presentation"))
	{
		return PRESENTATION;
	}

	if (NULL != wcsstr(currentDir, L"Model"))
	{
		return MODEL;
	}

	if (NULL != wcsstr(currentDir, L"Content"))
	{
		return CONTENT;
	}


	return CONTENT;
}


void switchTo(const wchar_t *libFolder)
{
	// get active filename and its directory
	wchar_t currentDir[MAX_PATH] = {0};
	wchar_t filename[MAX_PATH] = {0};
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDIRECTORY, MAX_PATH, (LPARAM)&currentDir);
	::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, MAX_PATH, (LPARAM)&filename);
	
	if (0 == wcslen(currentDir)) 
	{
		return;
	}

	// switch to directory
	int position = wcslen(currentDir);
	while (position > 0 && (currentDir[position] != L'\\'))
	{
		position--;
	}

	wchar_t fileToOpen[MAX_PATH] = {0};
	wcscpy(fileToOpen, currentDir);
	wcscpy(fileToOpen + position + 1, libFolder);
	wcscat(fileToOpen, L"\\");
	wcscat(fileToOpen, filename);
	
	::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)&fileToOpen);
}

void switchTo(FOLDER_TYPE libFolder)
{
	switch(libFolder)
	{
	case MODEL:
		switchTo(L"Model");
		break;
	case CONTENT:
		switchTo(L"Content");
		break;
	case PRESENTATION:
		switchTo(L"Presentation");
		break;
	}
}

void toggleModel()
{
	static FOLDER_TYPE prevFolder;
	if (MODEL == getCurrentFolder ())
	{
		switchTo(prevFolder);
		return;
	}

	prevFolder = getCurrentFolder ();
	switchTo(MODEL);
}

void getItem()
{	
	switchTo(CONTENT);
	
	int posFind = searchScintilla(L"sub get_item_of");
	int subLine = scintillaMsg(SCI_LINEFROMPOSITION, posFind);
	scintillaMsg(SCI_GOTOLINE, subLine + 2);
}

void drawItem()
{	
	switchTo(PRESENTATION);

	int posFind = searchScintilla(L"sub draw_item_of_");
	int subLine = scintillaMsg(SCI_LINEFROMPOSITION, posFind);
	scintillaMsg(SCI_GOTOLINE, subLine + 2);
}

void select()
{
	switchTo(CONTENT);

	int posFind = searchScintilla(L"sub select_");
	int subLine = scintillaMsg(SCI_LINEFROMPOSITION, posFind);
	scintillaMsg(SCI_GOTOLINE, subLine + 2);
}

void draw()
{
	switchTo(PRESENTATION);

	int posFind = searchScintilla(L"sub draw_[^i]");
	int subLine = scintillaMsg(SCI_LINEFROMPOSITION, posFind);
	scintillaMsg(SCI_GOTOLINE, subLine + 2);
}

void doUpdate()
{
	switchTo(CONTENT);

	int posFind = searchScintilla(L"sub do_update_");
	int subLine = scintillaMsg(SCI_LINEFROMPOSITION, posFind);
	scintillaMsg(SCI_GOTOLINE, subLine + 2);
}

void doDelete()
{
	switchTo(CONTENT);

	int posFind = searchScintilla(L"sub do_delete_");
	int subLine = scintillaMsg(SCI_LINEFROMPOSITION, posFind);
	scintillaMsg(SCI_GOTOLINE, subLine + 2);

}

void doCreate()
{
	switchTo(CONTENT);

	int posFind = searchScintilla(L"sub do_create_");
	int subLine = scintillaMsg(SCI_LINEFROMPOSITION, posFind);
	scintillaMsg(SCI_GOTOLINE, subLine + 2);
}
