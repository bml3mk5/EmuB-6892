// mkkanji.cpp

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#if (_MSC_VER >= 1400)
#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )
#endif

LPBYTE lpBuf = NULL;
LPWORD lpBmp = NULL;
HBITMAP hBmp = NULL;
HDC hdcDib = NULL;

HFONT hFont = NULL;

int font_size;
TCHAR font_name[32];
TCHAR input_path[MAX_PATH];
TCHAR output_path[MAX_PATH];

void create_dib()
{
	// create dib section (32x32, 16bpp)
	lpBuf = (LPBYTE)GlobalAlloc(GPTR, sizeof(BITMAPINFO));
	LPBITMAPINFO lpDib = (LPBITMAPINFO)lpBuf;
	lpDib->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpDib->bmiHeader.biWidth = 32;
	lpDib->bmiHeader.biHeight = 32;
	lpDib->bmiHeader.biPlanes = 1;
	lpDib->bmiHeader.biBitCount = 16;
	lpDib->bmiHeader.biCompression = BI_RGB;
	lpDib->bmiHeader.biSizeImage = 0;
	lpDib->bmiHeader.biXPelsPerMeter = 0;
	lpDib->bmiHeader.biYPelsPerMeter = 0;
	lpDib->bmiHeader.biClrUsed = 0;
	lpDib->bmiHeader.biClrImportant = 0;
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	hBmp = CreateDIBSection(hdc, lpDib, DIB_RGB_COLORS, (PVOID*)&lpBmp, NULL, 0);
	hdcDib = CreateCompatibleDC(hdc);
	SelectObject(hdcDib, hBmp);
}

void release_dib()
{
	DeleteDC(hdcDib);
	DeleteObject(hBmp);
	GlobalFree(lpBuf);
}

void create_font()
{
	// create font (ms gothic, 16pt)
	LOGFONT logfont;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
//	logfont.lfCharSet = SHIFTJIS_CHARSET;
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfOutPrecision = OUT_TT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	_tcscpy(logfont.lfFaceName, font_name);
	logfont.lfHeight = font_size;
	logfont.lfWidth = (font_size / 2);
	hFont = CreateFontIndirect(&logfont);
	SelectObject(hdcDib, hFont);
	SetBkMode(hdcDib, TRANSPARENT);
	SetTextColor(hdcDib, RGB(255, 255, 255));
}

void release_font()
{
	DeleteObject(hFont);
}

int length_utf8_char(int c)
{
	if ((c & 0x80) == 0) return 1;

	if ((c & 0xf0) == 0xe0) return 3;
	if ((c & 0xf8) == 0xf0) return 4;
	return 2;
}

int make_bitmap_font(const TCHAR *in_name, const TCHAR *out_name)
{
	TCHAR infile[MAX_PATH];
	TCHAR outfile[MAX_PATH];

	CHAR inchar[8];
	int inlen;

	WCHAR outchar[4];
	int outlen;

	memset(infile, 0, sizeof(infile));
	_tcsncpy(infile, input_path, MAX_PATH - 1);
	_tcsncat(infile, in_name, MAX_PATH - 1);
	memset(outfile, 0, sizeof(outfile));
	_tcsncpy(outfile, output_path, MAX_PATH - 1);
	_tcsncat(outfile, out_name, MAX_PATH - 1);


	FILE* fs = _tfopen(infile, _T("rb"));
	if(fs == NULL) {
		printf("%s not found.\n", infile);
		return 1;
	}

	FILE* fd = _tfopen(outfile, _T("wb"));
	int c1;
	while((c1 = fgetc(fs)) != EOF) {
		memset(lpBmp, 0, 32 * 32 * 2);

		if(c1 == 0x0d || c1 == 0x0a) {
			continue;
		}
		if(c1 == 0x20) {
			// read more one byte
			fgetc(fs);
			for(int y = 0; y < 16; y++) {
				fputc(0x00, fd);
				fputc(0x01, fd);
			}
			continue;
		}

		memset(inchar, 0, sizeof(inchar));
		inlen = length_utf8_char(c1);
		for(int i=0; i<inlen; i++) {
			if (i == 0) {
				inchar[i] = (CHAR)c1;
			} else {
				inchar[i] = (CHAR)fgetc(fs);
			}
		}
		outlen = MultiByteToWideChar(CP_UTF8, 0, inchar, inlen, outchar, sizeof(outchar) / sizeof(WCHAR));

		if (outlen == 1) {
			ExtTextOutW(hdcDib, 0, 0, NULL, NULL, outchar, outlen, NULL);
		}
		if (inlen == 1) {
			// ascii
			inchar[0] = (CHAR)fgetc(fs);
			inchar[1] = 0;
			inlen = 1;
			outlen = MultiByteToWideChar(CP_UTF8, 0, inchar, inlen, outchar, sizeof(outchar) / sizeof(WCHAR));

			if (outlen == 1) {
				ExtTextOutW(hdcDib, 8, 0, NULL, NULL, outchar, outlen, NULL);
			}
		}

		for(int y = 0; y < 16; y++) {
			LPWORD pat = &lpBmp[32 * (31 - y)];
			int d;
			d  = pat[ 0] ? 0x80 : 0;
			d |= pat[ 1] ? 0x40 : 0;
			d |= pat[ 2] ? 0x20 : 0;
			d |= pat[ 3] ? 0x10 : 0;
			d |= pat[ 4] ? 0x08 : 0;
			d |= pat[ 5] ? 0x04 : 0;
			d |= pat[ 6] ? 0x02 : 0;
			d |= pat[ 7] ? 0x01 : 0;
			fputc(d, fd);
			d  = pat[ 8] ? 0x80 : 0;
			d |= pat[ 9] ? 0x40 : 0;
			d |= pat[10] ? 0x20 : 0;
			d |= pat[11] ? 0x10 : 0;
			d |= pat[12] ? 0x08 : 0;
			d |= pat[13] ? 0x04 : 0;
			d |= pat[14] ? 0x02 : 0;
			d |= pat[15] ? 0x01 : 0;
			fputc(d, fd);
		}
	}
	fclose(fd);
	fclose(fs);

	return 0;
}

enum en_subargs {
	SUBARG_NONE = 0,
	SUBARG_FONT,
	SUBARG_INPUT,
	SUBARG_OUTPUT,
	SUBARG_FSIZE,
};

static const struct st_arg_list {
	const TCHAR *key;
	char subarg;
	char usage;
} arg_list[] = {
	{ _T("-h"), SUBARG_NONE, 1 },
	{ _T("-f"), SUBARG_FONT, 0 },
	{ _T("-i"), SUBARG_INPUT, 0 },
	{ _T("-o"), SUBARG_OUTPUT, 0 },
	{ _T("-p"), SUBARG_FSIZE, 0 },
	{ NULL }
};

void usage()
{
	_tprintf(_T("Make Kanji ROM image. Win Version 0.1.1\n")); 
	_tprintf(_T("Usage: mkkanji.exe [-h] [-f \"<font name>\"] [-p <font size>] [-i <input path>] [-o <output path>]\n"));
	_tprintf(_T("Options: -h : Show this usage.\n"));
	_tprintf(_T("         -f : Set font name Windows installed.\n"));
	_tprintf(_T("         -p : Set font size. (default:16)(range:8-24)\n"));
	_tprintf(_T("         -o : Set path for output.\n"));
	_tprintf(_T("         -i : Set path where kanji.txt is stored in.\n"));
}

int parse_options(int argc, TCHAR *argv[])
{
	const TCHAR *arg;
	TCHAR *err;

	int subarg = 0;
	for(int n=1; n<argc; n++) {
		arg = argv[n];
		int match = -1;
		for(int i=0; arg_list[i].key; i++) {
			if (_tcsicmp(arg, arg_list[i].key) == 0) {
				match = i;
				break;
			}
		}
		if (match >= 0) {
			if (arg_list[match].usage || subarg) {
				usage();
				return 0;
				break;
			}
			subarg = arg_list[match].subarg;
		} else {
			switch(subarg) {
			case SUBARG_FONT:
				memset(font_name, 0, sizeof(font_name));
				_tcsncpy(font_name, arg, sizeof(font_name) - 1);
				break;
			case SUBARG_INPUT:
				memset(input_path, 0, sizeof(input_path));
				_tcsncpy(input_path, arg, MAX_PATH-1);
				break;
			case SUBARG_OUTPUT:
				memset(output_path, 0, sizeof(output_path));
				_tcsncpy(output_path, arg, MAX_PATH-1);
				break;
			case SUBARG_FSIZE:
				err = NULL;
				font_size = (int)_tcstol(arg, &err, 10);
				if ((err && *err) || font_size < 8 || 16 < font_size) {
					usage();
					return 0;
				}
				break;
			default:
				usage();
				return 0;
				break;
			}
			subarg = 0;
		}
	}
	return 1;
}


int _tmain(int argc, TCHAR* argv[])
{
//	_tcscpy(font_name, _T("‚l‚r ƒSƒVƒbƒN"));
	_tcscpy(font_name, _T("MS Gothic"));
	font_size = 16;
	memset(input_path, 0, sizeof(input_path));
	memset(output_path, 0, sizeof(output_path));

	// parse options
	if (!parse_options(argc, argv)) {
		return 1;
	}

	// create dib section (32x32, 16bpp)
	create_dib();

	// create font (ms gothic, 16pt)
	create_font();

	// convert font
	int rc = 0;
	rc |= make_bitmap_font(_T("kanji.txt"), _T("KANJI.ROM"));
//	rc |= make_bitmap_font(_T("kanji2.txt"), _T("KANJI2.ROM"));

	// release
	release_font();

	release_dib();

	return rc;
}

