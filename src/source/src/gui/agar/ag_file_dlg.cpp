/** @file ag_file_dlg.cpp

	Skelton for retropc emulator
	SDL + Ager edition

	@author Sasaji
	@date   2012.3.3

	@brief [ ag_file_dlg ]
*/

#include "ag_file_dlg.h"
#include "ag_gui_base.h"
#include "ag_gui.h"
#include "../../main.h"

namespace GUI_AGAR
{

extern void get_dir_and_basename(const char *, char *, char *);

AG_FILE_DLG::AG_FILE_DLG(EMU *parent_emu, AG_GUI_BASE *parent_gui) : AG_DLG(parent_emu, parent_gui)
{
	flags = AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND;
#ifdef AG_FILEDLG_MASK_EXT
	flags |= AG_FILEDLG_MASK_EXT;
#endif
}

AG_FILE_DLG::~AG_FILE_DLG()
{
}

/**
 * create load file dialog
 */
void AG_FILE_DLG::CreateLoad(FileDlgType type, const char *title, const char *ext, const char *dir, const char *file, void *attr)
{
	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL);

	AG_WindowSetCaptionS(win, title);

	AG_FileDlg *fd = AG_FileDlgNew(win, flags | AG_FILEDLG_LOAD);

	if (dir != NULL && dir[0] != '\0') {
		AG_FileDlgSetDirectoryS(fd, dir);
	} else {
		AG_FileDlgSetDirectoryS(fd, emu->application_path());
	}
	if (file != NULL && file[0] != '\0') {
		AG_FileDlgSetFilenameS(fd, file);
	}
	if (ext != NULL) {
		AG_FileDlgAddType(fd, "Supported Files", ext, NULL, NULL);
	}
//	AG_SetEvent(fd->comTypes, "combo-selected", OnSelectFileType, "%Cp", fd);

	AG_FileDlgOkAction(fd, OnClickOkLoad, "%Cp %i %p", this, type, attr);
	AG_FileDlgCancelAction(fd, OnCloseLoad, "%Cp %Cp %i", this, fd, type);
	AG_SetEvent(win, "window-close", OnCloseLoad, "%Cp %Cp %i", this, fd, type);
	AG_AddEvent(win, "key-up", OnKeyUpLoad, "%Cp %Cp %i", this, fd, type);

	AG_TextboxSizeHintPixels(fd->tbFile,fd->tbFile->ed->wPre,fd->tbFile->ed->hPre + 2);

	gui->PostEtSystemPause(true);
	AG_WindowShow(win);
}

void AG_FILE_DLG::CreateLoad(FileDlgType type, const char *title, const char *ext, const char *dir, const char *file)
{
	CreateLoad(type, title, ext, dir, file, NULL);
}

/**
 * create save file dialog
 */
void AG_FILE_DLG::CreateSave(FileDlgType type, const char *title, const char *ext, const char *dir, const char *file, void *attr)
{
	AG_Window *win = AG_WindowNew(AG_WINDOW_MODAL);

	AG_WindowSetCaptionS(win, title);

	AG_FileDlg *fd = AG_FileDlgNew(win, flags | AG_FILEDLG_SAVE);

	if (dir != NULL && dir[0] != '\0') {
		AG_FileDlgSetDirectoryS(fd, dir);
	} else {
		AG_FileDlgSetDirectoryS(fd, emu->application_path());
	}
	if (file != NULL && file[0] != '\0') {
		AG_FileDlgSetFilenameS(fd, file);
	}
	if (ext != NULL) {
		AG_FileDlgAddType(fd, "Supported Files", ext, NULL, NULL);
	}
//	AG_SetEvent(fd->comTypes, "combo-selected", OnSelectFileType, "%Cp", fd);

	AG_FileDlgOkAction(fd, OnClickOkSave, "%Cp %i %p", this, type, attr);
	AG_FileDlgCancelAction(fd, OnCloseSave, "%Cp %Cp", this, fd, type);
	AG_SetEvent(win, "window-close", OnCloseSave, "%Cp %Cp %i", this, fd, type);
	AG_AddEvent(win, "key-up", OnKeyUpSave, "%Cp %Cp %i", this, fd, type);

	AG_TextboxSizeHintPixels(fd->tbFile,fd->tbFile->ed->wPre,fd->tbFile->ed->hPre + 2);

	gui->PostEtSystemPause(true);
	AG_WindowShow(win);
}

void AG_FILE_DLG::CreateSave(FileDlgType type, const char *title, const char *ext, const char *dir, const char *file)
{
	CreateSave(type, title, ext, dir, file, NULL);
}

/**
 * Create Overwrite dialog
 */
void AG_FILE_DLG::CreateOverwriteDlg(AG_FileDlg *fd, FileDlgType type, void *attr)
{
	AG_Window *win;
	AG_Button *btn;
	AG_HBox *hb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_MODAL);
	AG_WindowSetCaptionS(win, "File already exists.");

	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_LabelNew(win, 0, "File %s exists. Overwrite?", fd->cfile);
	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, "Yes",
		    OnClickOkOverwrite, "%Cp %Cp %Cp %i %p", this, win, fd, type, attr);
		btn = AG_ButtonNewFn(hb, 0, "No", AGWINDETACH(win));
		AG_WidgetFocus(btn);
	}

	AG_WindowShow(win);
}

//////////

void AG_FILE_DLG::CloseLoad(AG_FileDlg *fd, FileDlgType type)
{
	flags = fd->flags;

	switch(type) {
	case STATUS_AND_RECKEY:
		{
			// send event to open reckey file dialog
			AG_Window *win = gui->GetWindow();	// top level
			if (win != NULL) {
				AG_PostEvent(fd, win, "user-process", "%i", EVT_OPEN_LOAD_RECKEY_DIALOG);
			}
		}
		break;
	default:
		break;
	}

	AG_Window *win = AG_ParentWindow((void *)fd);
	if (win != NULL) {
		// close window
		AG_ObjectDetach(win);
	}

	if (type != STATUS_AND_RECKEY) {
		gui->PostEtSystemPause(false);
	}
}

void AG_FILE_DLG::CloseSave(AG_FileDlg *fd, FileDlgType type)
{
	flags = fd->flags;

	switch(type) {
	case STATUS_AND_RECKEY:
		{
			// send event to open reckey file dialog
			AG_Window *win = gui->GetWindow();	// top level
			if (win != NULL) {
				AG_PostEvent(fd, win, "user-process", "%i", EVT_OPEN_SAVE_RECKEY_DIALOG);
			}
		}
		break;
	default:
		break;
	}

	AG_Window *win = AG_ParentWindow((void *)fd);
	if (win != NULL) {
		// close window
		AG_ObjectDetach(win);
	}

	if (type != STATUS_AND_RECKEY) {
		gui->PostEtSystemPause(false);
	}
}

void AG_FILE_DLG::LoadFile(AG_FileDlg *fd, FileDlgType type, void *attr)
{
	flags = fd->flags;


	switch(type) {
	case DATAREC:
		gui->PostEtLoadDataRecMessage(fd->cfile);
		break;
	case FLOPPY:
		gui->PostEtOpenFloppyMessage((int)(intptr_t)attr, fd->cfile, 0, 0, true);
		break;
	case FLOPPY_NEW:
		break;
	case PRINTER:
		break;
	case STATUS:
		gui->PostEtLoadStatusMessage(fd->cfile);
		break;
	case AUTOKEY:
		gui->PostEtLoadAutoKeyMessage(fd->cfile);
		break;
	case RECKEY:
		gui->PostEtLoadRecKeyMessage(fd->cfile);
		break;
	case RECKEY_AFTER_STATUS:
		gui->PostEtLoadRecKeyMessage(fd->cfile);
		break;
	case STATUS_AND_RECKEY:
		{
			gui->PostEtLoadStatusMessage(fd->cfile);
			// send event to open reckey file dialog
			AG_Window *win = gui->GetWindow();	// top level
			if (win != NULL) {
				AG_PostEvent(fd, win, "user-process", "%i", EVT_OPEN_LOAD_RECKEY_DIALOG);
			}
		}
		break;
	}

	AG_Window *win = AG_ParentWindow((void *)fd);
	if (win != NULL) {
		// close window
		AG_ObjectDetach((void *)win);
	}

//	if (type != STATUS_AND_RECKEY) {
//		gui->PostEtSystemPause(false);
//	}
}

void AG_FILE_DLG::SaveFile(AG_FileDlg *fd, FileDlgType type, void *attr)
{
	flags = fd->flags;

	switch(type) {
	case DATAREC:
		gui->PostEtSaveDataRecMessage(fd->cfile);
		break;
	case FLOPPY:
		break;
	case FLOPPY_NEW:
		if (emu->create_blank_floppy_disk(fd->cfile, (int)(intptr_t)attr >> 8)) {
			gui->PostEtOpenFloppyMessage((int)(intptr_t)attr & 0xff, fd->cfile, 0, 0, false);
		}
		break;
	case PRINTER:
		gui->PostEtSavePrinterMessage((int)(intptr_t)attr, fd->cfile);
		break;
	case STATUS:
		gui->PostEtSaveStatusMessage(fd->cfile, false);
		break;
	case AUTOKEY:
		break;
	case RECKEY:
		gui->PostEtSaveRecKeyMessage(fd->cfile, false);
		break;
	case RECKEY_AFTER_STATUS:
		gui->PostEtSaveRecKeyMessage(fd->cfile, false);
		break;
	case STATUS_AND_RECKEY:
		{
			gui->PostEtSaveStatusMessage(fd->cfile, true);
			// send event to open reckey file dialog
			AG_Window *win = gui->GetWindow();	// top level
			if (win != NULL) {
				AG_PostEvent(fd, win, "user-process", "%i", EVT_OPEN_SAVE_RECKEY_DIALOG);
			}
		}
		break;
	}

	AG_Window *win = AG_ParentWindow((void *)fd);
	if (win != NULL) {
		// close window
		AG_ObjectDetach((void *)win);
	}
//	if (type != STATUS && type != STATUS_AND_RECKEY && type != RECKEY_AFTER_STATUS) {
//		gui->PostEtSystemPause(false);
//	}
}

/*
 * Event Handler (static)
 */
void AG_FILE_DLG::OnCloseLoad(AG_Event *event)
{
	AG_FILE_DLG *dlg = (AG_FILE_DLG *)AG_PTR(1);
	AG_FileDlg *fd = (AG_FileDlg *)AG_PTR(2);
	FileDlgType type = (FileDlgType)AG_INT(3);

	dlg->CloseLoad(fd, type);
}

void AG_FILE_DLG::OnCloseSave(AG_Event *event)
{
	AG_FILE_DLG *dlg = (AG_FILE_DLG *)AG_PTR(1);
	AG_FileDlg *fd = (AG_FileDlg *)AG_PTR(2);
	FileDlgType type = (FileDlgType)AG_INT(3);

	dlg->CloseSave(fd, type);
}

void AG_FILE_DLG::OnKeyUpLoad(AG_Event *event)
{
	AG_FILE_DLG *dlg = (AG_FILE_DLG *)AG_PTR(1);
	AG_FileDlg *fd = (AG_FileDlg *)AG_PTR(2);
	FileDlgType type = (FileDlgType)AG_INT(3);
	int key = AG_INT(4);

	switch(key) {
	case AG_KEY_ESCAPE:
		dlg->CloseLoad(fd, type);
		break;
	}
}

void AG_FILE_DLG::OnKeyUpSave(AG_Event *event)
{
	AG_FILE_DLG *dlg = (AG_FILE_DLG *)AG_PTR(1);
	AG_FileDlg *fd = (AG_FileDlg *)AG_PTR(2);
	FileDlgType type = (FileDlgType)AG_INT(3);
	int key = AG_INT(4);

	switch(key) {
	case AG_KEY_ESCAPE:
		dlg->CloseSave(fd, type);
		break;
	}
}

void AG_FILE_DLG::OnClickOkLoad(AG_Event *event)
{
	AG_FileDlg *fd = (AG_FileDlg *)AG_SELF();
	AG_FILE_DLG *dlg = (AG_FILE_DLG *)AG_PTR(1);
	FileDlgType type = (FileDlgType)AG_INT(2);
	void *attr = AG_PTR(3);

	if (AG_FileDlgCheckReadAccess(fd) == -1) {
		AG_TextMsgFromError();
		return;
	}
	dlg->LoadFile(fd, type, attr);
}

void AG_FILE_DLG::OnClickOkSave(AG_Event *event)
{
	AG_FileDlg *fd = (AG_FileDlg *)AG_SELF();
	AG_FILE_DLG *dlg = (AG_FILE_DLG *)AG_PTR(1);
	FileDlgType type = (FileDlgType)AG_INT(2);
	void *attr = AG_PTR(3);
	AG_FileInfo info;
	const char *p;

	p = AG_ShortFilename(fd->cfile);
	if (p != NULL) {
		if (strchr(p, '.') == NULL) {
			AG_TextMsgS(AG_MSG_ERROR, "Need file extension.");
			return;
		}
	}

	if (AG_GetFileInfo(fd->cfile, &info) == 0) {
		if (info.perms & AG_FILE_WRITEABLE) {
			dlg->CreateOverwriteDlg(fd, type, attr);
			return;
		}
	}
	dlg->SaveFile(fd, type, attr);
}

void AG_FILE_DLG::OnClickOkOverwrite(AG_Event *event)
{
	AG_FILE_DLG *dlg = (AG_FILE_DLG *)AG_PTR(1);
	AG_Window *win = (AG_Window *)AG_PTR(2);
	AG_FileDlg *fd = (AG_FileDlg *)AG_PTR(3);
	FileDlgType type = (FileDlgType)AG_INT(4);
	void *attr = AG_PTR(5);

	AG_ObjectDetach(win);

	dlg->SaveFile(fd, type, attr);
}

void AG_FILE_DLG::OnSelectFileType(AG_Event *event)
{
	AG_FileDlg *fd = (AG_FileDlg *)AG_PTR(1);
	AG_TlistItem *it = (AG_TlistItem *)AG_PTR(2);

	if (it == NULL) return;

	AG_FileType *ft = (AG_FileType *)it->p1;

	if (ft == NULL) return;

	if (ft->action != NULL) {
		AG_PostEvent(NULL, fd, ft->action->name,
			"%p %p %p %p %p", fd, ft, ft->action->argv[0], ft->action->argv[1], ft->action->argv[2]);
	}
}

}; /* namespace GUI_AGAR */
