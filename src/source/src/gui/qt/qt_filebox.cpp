/** @file qt_filebox.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.11.01

	@brief [ qt file box ]
*/

#include "qt_filebox.h"
#include "../../cchar.h"
#include "../../utility.h"
#include "../../utils.h"

MyFileBox::MyFileBox(QWidget *parent, const QString &caption, bool save, const _TCHAR *directory, const char *filter)
 : QFileDialog(parent, caption)
{
	setDirectory(QTChar::fromTChar(directory));
	setAcceptMode(save ? AcceptSave : AcceptOpen);
	setFileMode(ExistingFile);	// select only one file
	setFilterTypes(filter, save);
}

MyFileBox::MyFileBox(QWidget *parent, CMsg::Id caption, bool save, const _TCHAR *directory, const char *filter)
 : QFileDialog(parent)
{
	setWindowTitle(CMSGV(caption));
	setDirectory(QTChar::fromTChar(directory));
	setAcceptMode(save ? AcceptSave : AcceptOpen);
	setFileMode(ExistingFile);	// select only one file
	setFilterTypes(filter, save);
}

MyFileBox::~MyFileBox()
{
}

void MyFileBox::setFilterTypes(const char *filter_str, bool save)
{
	QStringList filters;

	char subext[8];
	char label[_MAX_PATH];

	int fil_pos = 0;
	int fil_len = (int)strlen(filter_str);
	int sub_len = 0;
	int ext_nums = 0;
	strcpy(subext, "*.");
	if (!save) {
		// for load dialog
		// "Supported Files (*.foo *.bar)"
		UTILITY::strcpy(label, sizeof(label), CMSG(Supported_Files));
		UTILITY::strcat(label, sizeof(label), " (");
		do {
			sub_len = 0;
			fil_pos = UTILITY::get_token(filter_str, fil_pos, fil_len, &subext[2], 6, ';', &sub_len);
			if (fil_len > 0) {
				for(char *p=&subext[2]; *p != '\0'; p++) *p = tolower(*p);
				if (ext_nums > 0) {
					UTILITY::strcat(label, sizeof(label), " ");
				}
				UTILITY::strcat(label, sizeof(label), subext);
#ifndef Q_OS_WIN
				for(char *p=&subext[2]; *p != '\0'; p++) *p = toupper(*p);
				UTILITY::strcat(label, sizeof(label), " ");
				UTILITY::strcat(label, sizeof(label), subext);
#endif
				ext_nums++;
			}
		} while(fil_pos >= 0);
		UTILITY::strcat(label, sizeof(label), ")");
		if (ext_nums > 0) {
			filters.append(label);
		}
		// "All Files (*.*)"
		UTILITY::strcpy(label, sizeof(label), CMSG(All_Files));
		UTILITY::strcat(label, sizeof(label), " (*.*)");
		filters.append(label);

	} else {
		// for save dialog
		// "Foo File (*.foo)"
		// "Bar File (*.bar)"
		do {
			sub_len = 0;
			fil_pos = UTILITY::get_token(filter_str, fil_pos, fil_len, &subext[2], 6, ';', &sub_len);
			if (fil_len > 0) {
#ifndef Q_OS_WIN
				for(int i=0; i<2; i++) {
#else
				for(int i=0; i<1; i++) {
#endif
					if (i == 0) {
						for(char *p=&subext[2]; *p != '\0'; p++) *p = tolower(*p);
					} else {
						for(char *p=&subext[2]; *p != '\0'; p++) *p = toupper(*p);
					}
					UTILITY::strcpy(label, sizeof(label), &subext[2]);
					UTILITY::strcat(label, sizeof(label), " ");
					UTILITY::strcat(label, sizeof(label), CMSG(File));
					UTILITY::strcat(label, sizeof(label), " (");
					UTILITY::strcat(label, sizeof(label), subext);
					UTILITY::strcat(label, sizeof(label), ")");
					filters.append(label);
				}
				ext_nums++;
			}
		} while(fil_pos >= 0);
	}

	setNameFilters(filters);
}
