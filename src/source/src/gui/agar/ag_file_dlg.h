/** @file ag_file_dlg.h

	Skelton for retropc emulator
	SDL + Agar edition

	@author Sasaji
	@date   2012.03.01

	@brief [ ag_file_dlg ]
*/

#ifndef AG_FILE_DLG_H
#define AG_FILE_DLG_H

#include "ag_dlg.h"
#include "../../emumsg.h"

namespace GUI_AGAR
{

/**
	@brief file dialog
*/
class AG_FILE_DLG : public AG_DLG {
	friend class AG_GUI_BASE;

private:
	Uint32 flags;
	int ft_all;

	// file dialog event handler
	static void OnCloseLoad(AG_Event *);
	static void OnCloseSave(AG_Event *);
	static void OnKeyUpLoad(AG_Event *);
	static void OnKeyUpSave(AG_Event *);
	static void OnClickOkLoad(AG_Event *);
	static void OnClickOkSave(AG_Event *);
	static void OnClickOkOverwrite(AG_Event *);
	static void OnSelectFileType(AG_Event *);

public:
	AG_FILE_DLG(EMU *, AG_GUI_BASE *);
	~AG_FILE_DLG();

	enum FileDlgType {
		DATAREC,
		FLOPPY,
		FLOPPY_NEW,
		PRINTER,
		STATUS,
		AUTOKEY,
		RECKEY,
		RECKEY_AFTER_STATUS,
		STATUS_AND_RECKEY
	};
	void CreateLoad(FileDlgType, const char *, const char *, const char *, const char *, void *);
	void CreateLoad(FileDlgType, const char *, const char *, const char *, const char *);
	void CreateSave(FileDlgType, const char *, const char *, const char *, const char *, void *);
	void CreateSave(FileDlgType, const char *, const char *, const char *, const char *);
    void CreateOverwriteDlg(AG_FileDlg *, FileDlgType, void *);

	void CloseLoad(AG_FileDlg *, FileDlgType);
	void CloseSave(AG_FileDlg *, FileDlgType);
	void LoadFile(AG_FileDlg *, FileDlgType, void *);
	void SaveFile(AG_FileDlg *, FileDlgType, void *);
};

}; /* namespace GUI_AGAR */

#endif /* AG_FILE_DLG_H */
