/** @file wxw_msgboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2019.03.01 -

	@brief [ message board ]
*/

#include <wx/wx.h>
#include "wxw_msgboard.h"
#include "../../cpixfmt.h"
#include "wxw_csurface.h"
#include "wxw_emu.h"
#include "../../config.h"
#include "../../utility.h"

#undef USE_BG_TRANSPARENT

MsgBoard::MsgBoard(EMU *pEmu)
{
	visible = true;
	enable  = false;

	sMainSuf = NULL;
	emu = pEmu;

	msg.font = NULL;
	info.font = NULL;

	msg.mux = new CMutex();
	info.mux = new CMutex();

	inited = false;
}

MsgBoard::~MsgBoard()
{
	if (inited) {
		delete msg.font;
		delete info.font;
	}
	delete sMainSuf;

	delete msg.mux;
	delete info.mux;
}

void MsgBoard::CreateSurface(CPixelFormat *format, int width, int height)
{
	if (sMainSuf != NULL) {
		delete sMainSuf;
	}
	sMainSuf = new CSurface(width, height, *format);
	if (sMainSuf == NULL) {
		enable = false;
	} else {
		enable = true;
	}
}

/// 画面初期化
void MsgBoard::InitScreen(CPixelFormat *format, int width, int height)
{
	// 色
	fg.Set(*format, 0x00, 0xc0, 0x80, 0xff);
	bg.Set(*format, 0x00, 0x40, 0x00, 0xff);

	// ウィンドウサイズ
	szWin.cx = width;
	szWin.cy = height;

	CreateSurface(format, width, 128);

	if (enable) {
		// SDL_ttf 初期化
//		if (!TTF_WasInit()) {
//			if (TTF_Init() != -1) {
				inited = true;
//			} else {
//				enable = false;
//			}
//		}
	}
	// メッセージ用フォントの設定
	// 枠設定
	RECT_IN(msg.re, 0, 0, width, 63);
	// 表示位置
	msg.pt.x = 0; msg.pt.y = 0; info.place = 2;
	// 情報用フォントの設定
	// 枠設定
	RECT_IN(info.re, 0, 64, width, 127);
	// 表示位置
	info.pt.x = 0; info.pt.y = 0; info.place = 1;

	if (enable) {
		SetFont();
	}
	if (enable) {
		logging->out_log_x(LOG_INFO , CMsg::MsgBoard_OK);
	} else {
		logging->out_log_x(LOG_ERROR, CMsg::MsgBoard_Failed);
	}
}

/// フォント設定
bool MsgBoard::SetFont()
{
	if (!inited) return false;

	// メッセージ用フォントの設定
	enable = set_sys_font(CMsg::message, config.msgboard_msg_fontname, config.msgboard_msg_fontsize, &msg.font, msg.font_name);
	if (enable) {
//		TTF_SetFontStyle(msg.font, TTF_STYLE_NORMAL);
		config.msgboard_msg_fontname.Set(msg.font_name);
		// 情報用フォントの設定
		enable = set_sys_font(CMsg::info, config.msgboard_info_fontname, config.msgboard_info_fontsize, &info.font, info.font_name);
		if (enable) {
//			TTF_SetFontStyle(info.font, TTF_STYLE_NORMAL);
			config.msgboard_info_fontname.Set(info.font_name);
		}
	}
	return enable;
}

/// 文字列出力
void MsgBoard::draw(CSurface *screen, msg_data_t &data)
{
	VmRectWH reDst;

	data.mux->lock();

	if (!data.lists.empty()) {
//		list_t::iterator it = data.lists.begin();

		// 基準位置の計算
		if (data.place & 1) {
			reDst.x = szWin.cx + data.pt.x - data.sz.cx;
			reDst.w = data.sz.cx;
		} else {
			reDst.x = data.pt.x;
			reDst.w = data.sz.cx;
		}
		if (data.place & 2) {
			reDst.y = szWin.cy + data.pt.y - data.sz.cy;
			reDst.h = data.sz.cy;
		} else {
			reDst.y = data.pt.y;
			reDst.h = data.sz.cy;
		}

		// メインコンテキストにメッセージをコピー
#ifdef USE_BG_TRANSPARENT
		draw_text(screen, data, reDst.x, reDst.y);
#else
		sMainSuf->Blit(data.re, *screen, reDst);
#endif
	}

	data.mux->unlock();
}

void MsgBoard::Draw(CSurface *screen)
{
	if (!enable || !visible) return;

	draw(screen, msg);
	draw(screen, info);
}

/// 文字列をバックバッファに描画
///
/// @note ロックは呼び出し元で行うこと should be locked in caller function
void MsgBoard::draw_text(msg_data_t &data)
{
//	int len = 0;

	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		wxDC *dc = sMainSuf->GetDC();
#ifndef USE_BG_TRANSPARENT
		dc->SetPen(wxPen());
		dc->SetBrush(wxBrush(bg));

		// 枠を描画
		dc->DrawRectangle(data.re.x, data.re.y, data.re.w, data.re.h);
//		SDL_FillRect(sMainSuf, &data.re, SDL_MapRGB(sMainSuf->format, bg.r, bg.g, bg.b));

		// 文字を描画
//		len = (int)strlen(it->cmsg);
		dc->SetTextForeground(fg);
//		dc->SetTextBackground(bg);
		dc->SetFont(*data.font);

		wxSize sz = dc->GetTextExtent(it->msg);
		data.sz.cx = sz.x + 4;
		data.sz.cy = sz.y + 4;

		dc->DrawText(it->msg, data.re.x + 2, data.re.y + 2);
#else
		// 文字列の幅高さの枠を計算
		dc->SetFont(*data.font);

		wxSize sz = dc->GetTextExtent(it->msg);
		data.sz.cx = sz.x + 4;
		data.sz.cy = sz.y + 4;
#endif
	}
}

/// 文字列をバックバッファに描画
///
/// @note ロックは呼び出し元で行うこと should be locked in caller function
void MsgBoard::draw_text(CSurface *suf, msg_data_t &data, int left, int top)
{
//	int len = 0;

	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 文字を描画
//		len = (int)strlen(it->cmsg);
		wxDC *dc = suf->GetDC();

		dc->SetTextForeground(fg);
		dc->SetTextBackground(bg);
		dc->SetFont(*data.font);

		dc->DrawText(it->msg, data.re.x + left, top);
	}
}

/// メッセージ設定
void MsgBoard::Set(msg_data_t &data, const _TCHAR *str, int sec)
{
	item_t itm;

	UTILITY::tcscpy(itm.msg, MSGBOARD_STR_SIZE, str);
	itm.cnt = (60 * sec);

	data.mux->lock();

	data.lists.push_front(itm);
	if (data.lists.size() > 10) {
		data.lists.pop_back();
	}

	if (enable) {
		draw_text(data);
	}

	data.mux->unlock();
}
void MsgBoard::Set(msg_data_t &data, CMsg::Id id, int sec)
{
	Set(data, gMessages.Get(id), sec);
}
void MsgBoard::SetMessage(const _TCHAR *str, int sec)
{
	Set(msg, str, sec);
}
void MsgBoard::SetMessage(CMsg::Id id, int sec)
{
	Set(msg, id, sec);
}
void MsgBoard::SetInfo(const _TCHAR *str, int sec)
{
	Set(info, str, sec);
}
void MsgBoard::SetInfo(CMsg::Id id, int sec)
{
	Set(info, id, sec);
}
void MsgBoard::SetMessageF(const _TCHAR *format, ...)
{
	_TCHAR buf[MSGBOARD_STR_SIZE];

	va_list ap;

	va_start(ap, format);
	UTILITY::vstprintf(buf, MSGBOARD_STR_SIZE, format, ap);
	va_end(ap);

	SetMessage(buf);
}
void MsgBoard::SetInfoF(const _TCHAR *format, ...)
{
	_TCHAR buf[MSGBOARD_STR_SIZE];

	va_list ap;

	va_start(ap, format);
	UTILITY::vstprintf(buf, MSGBOARD_STR_SIZE, format, ap);
	va_end(ap);

	SetInfo(buf);
}

/// ウィンドウサイズ設定
void MsgBoard::SetSize(int width, int height)
{
	szWin.cx = width;
	szWin.cy = height;
}

/// メッセージ描画位置設定
/// @param[in] cx X
/// @param[in] cy Y
/// @param[in] place 1:画面右基準 2:画面下基準
void MsgBoard::SetMessagePos(int cx, int cy, int place)
{
	msg.pt.x = cx;
	msg.pt.y = cy;
	msg.place = place;
}
/// 情報描画位置設定
/// @param[in] cx X
/// @param[in] cy Y
/// @param[in] place 1:画面右基準 2:画面下基準
void MsgBoard::SetInfoPos(int cx, int cy, int place)
{
	info.pt.x = cx;
	info.pt.y = cy;
	info.place = place;
}

/// メッセージ削除
void MsgBoard::Delete(msg_data_t &data, const _TCHAR *str)
{
	bool redraw = false;

	data.mux->lock();

	if (!data.lists.empty()) {
		list_t::iterator it = data.lists.begin();
		list_t::iterator it_next;

		while(it != data.lists.end())	{
			it_next = it;
			it_next++;

			if (_tcscmp(str, it->msg) == 0) {
				data.lists.erase(it);
				redraw = true;
			}
			it = it_next;
		}
	}
	if (redraw) {
		draw_text(data);
	}

	data.mux->unlock();
}
void MsgBoard::Delete(msg_data_t &data, CMsg::Id id)
{
	Delete(data, gMessages.Get(id));
}
void MsgBoard::DeleteMessage(const _TCHAR *str)
{
	Delete(msg, str);
}
void MsgBoard::DeleteMessage(CMsg::Id id)
{
	Delete(msg, id);
}
void MsgBoard::DeleteInfo(const _TCHAR *str)
{
	Delete(info, str);
}
void MsgBoard::DeleteInfo(CMsg::Id id)
{
	Delete(info, id);
}

/// カウントダウン
void MsgBoard::count_down(msg_data_t &data)
{
	bool redraw = false;

	data.mux->lock();

	if (!data.lists.empty()) {
		list_t::iterator it = data.lists.begin();
		list_t::iterator it_next;

		while(it != data.lists.end())	{
			it_next = it;
			it_next++;

			if (it->cnt > 0) it->cnt--;
			if (it->cnt == 0) {
				data.lists.erase(it);
				redraw = true;
			}
			it = it_next;
		}
	}
	if (redraw) {
		draw_text(data);
	}

	data.mux->unlock();
}
void MsgBoard::CountDown(void)
{
	count_down(msg);
	count_down(info);
}

/// 画面表示用システムフォントの設定
bool MsgBoard::set_sys_font(CMsg::Id title, const _TCHAR *name, int pt, wxFont **font, _TCHAR *font_name)
{
	sbuf_t sbuf;
//	std::list<sbuf_t> fpath;
	std::list<sbuf_t> fname;
	wxFont f;

	if (_tcslen(name) > 0) {
		UTILITY::tcscpy(sbuf.buf, 1024, name);
		fname.push_back(sbuf);
	}

	f = wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT);
	if (f.IsOk()) {
		UTILITY::tcscpy(sbuf.buf, 1024, f.GetFaceName().t_str());
		fname.push_back(sbuf);
	}

#if 0
#if defined(_WIN32)
//	config.font_path.GetN(sbuf.buf, _MAX_PATH);
//	fpath.push_back(sbuf);
//	emu->resource_path(sbuf.buf, _MAX_PATH);
//	fpath.push_back(sbuf);
//	emu->application_path(sbuf.buf, _MAX_PATH);
//	fpath.push_back(sbuf);
//	sprintf(sbuf.buf, "%s\\fonts\\", getenv("SystemRoot"));
//	fpath.push_back(sbuf);



#elif defined(linux)
	config.font_path.GetN(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	emu->resource_path(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	emu->application_path(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "%s/.fonts/", getenv("HOME"));
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/usr/share/fonts/truetype/");
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/usr/share/fonts/opentype/");
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/usr/share/fonts/opentype/ipafont/");
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/usr/share/fonts/truetype/freefont/");
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/usr/X11R6/lib/X11/fonts/TTF/");
	fpath.push_back(sbuf);

	UTILITY::tcs_to_mbs(sbuf.buf, name, _MAX_PATH);
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "ttf-japanese-gothic.ttf");
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "fonts-japanese-gothic.ttf");
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "FreeSans.ttf");
	fname.push_back(sbuf);

#elif defined(__APPLE__) && defined(__MACH__)
	config.font_path.GetN(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	emu->resource_path(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	emu->application_path(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "%s/Library/Fonts/", getenv("HOME"));
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/System/Library/Fonts/");
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/Library/Fonts/");
	fpath.push_back(sbuf);

	UTILITY::tcs_to_mbs(sbuf.buf, name, _MAX_PATH);
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "Hiragino Sans GB W3.ttc");
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "Osaka.ttf");
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "AppleGothic.ttf");
	fname.push_back(sbuf);

#elif defined(__FreeBSD__)
	config.font_path.GetN(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	emu->resource_path(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	emu->application_path(sbuf.buf, _MAX_PATH);
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "%s/.fonts/", getenv("HOME"));
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/usr/share/fonts/TTF/");
	fpath.push_back(sbuf);
	sprintf(sbuf.buf, "/usr/local/lib/X11/fonts/TTF/");
	fpath.push_back(sbuf);

	UTILITY::tcs_to_mbs(sbuf.buf, name, _MAX_PATH);
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "ipagp-mona.ttf");
	fname.push_back(sbuf);
	strcpy(sbuf.buf, "luxisr.ttf");
	fname.push_back(sbuf);

#endif
#endif
	CTchar xtitle(gMessages.Get(title));

	std::list<sbuf_t>::iterator itn = fname.begin();
	while (itn != fname.end()) {
		f.Create(pt, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, itn->buf);
		if (f.IsOk()) {
			delete *font;
			*font = new wxFont(f);
			if (font_name) {
				UTILITY::tcscpy(font_name, 128, f.GetFaceName().t_str());
			}
			logging->out_logf_x(LOG_INFO, CMsg::MsgBoard_Use_VSTR_for_VSTR, f.GetFaceName().t_str(), xtitle.Get());
			return true;
		}
		itn++;
	}
	logging->out_logf_x(LOG_WARN, CMsg::MsgBoard_Couldn_t_find_fonts_for_VSTR, xtitle.Get());
	return false;
}
