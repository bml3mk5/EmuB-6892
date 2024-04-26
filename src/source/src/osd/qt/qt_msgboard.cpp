/** @file qt_msgboard.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.26 -

	@brief [ message board ]
*/
#include "qt_msgboard.h"
#include "../../cpixfmt.h"
#include "qt_csurface.h"
#include "qt_emu.h"
#include "../../config.h"
#include "../../utility.h"
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>

#undef USE_BG_TRANSPARENT

MsgBoard::MsgBoard(EMU *pEmu)
{
	visible = true;
	enable  = false;

	sMainSuf = nullptr;
	emu = pEmu;

	msg.font = nullptr;
	info.font = nullptr;

	msg.mux = new CMutex();
	info.mux = new CMutex();

	inited = false;
}

MsgBoard::~MsgBoard()
{
	if (inited) {
		if (msg.font != nullptr) delete msg.font;
		if (info.font != nullptr) delete info.font;
	}
	delete sMainSuf;

	delete msg.mux;
	delete info.mux;
}

void MsgBoard::CreateSurface(CPixelFormat *format, int width, int height)
{
	delete sMainSuf;
	sMainSuf = new CSurface(width, height, *format);
	if (sMainSuf == nullptr) {
		enable = false;
	} else {
		enable = true;
	}
}

/// 画面初期化
void MsgBoard::InitScreen(CPixelFormat *format, int width, int height)
{
	// 色
#ifndef USE_BG_TRANSPARENT
	fg.Set(*format, 0x00, 0xc0, 0x80, 0xff);
	bg.Set(*format, 0x00, 0x40, 0x00, 0xff);
#else
	fg.Set(*format, 0x00, 0xc0, 0x20, 0xff);
	bg.Set(*format, 0x00, 0x20, 0x00, 0xff);
#endif

	// ウィンドウサイズ
	szWin.cx = width;
	szWin.cy = height;

	CreateSurface(format, width, 128);

	if (enable) {
		inited = true;
	}
	// メッセージ用フォントの設定
	// 枠設定
	RECT_IN(msg.re, 0, 0, width, 63)
	// 表示位置
	msg.pt.x = 0; msg.pt.y = 0; info.place = 2;
	// 情報用フォントの設定
	// 枠設定
	RECT_IN(info.re, 0, 64, width, 127)
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
	enable = set_sys_font(CMsg::message, pConfig->msgboard_msg_fontname.Get(), pConfig->msgboard_msg_fontsize, &msg.font);
	if (enable) {
		// 情報用フォントの設定
		enable = set_sys_font(CMsg::info, pConfig->msgboard_info_fontname.Get(), pConfig->msgboard_info_fontsize, &info.font);
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
	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 文字列の幅高さの枠を計算
		QFontMetrics m(*data.font);
		QSize sz = m.size(Qt::TextSingleLine, QString::fromUtf8(it->cmsg));
		data.sz.cx = sz.width() + 4;
		data.sz.cy = sz.height() + 4;

#ifndef USE_BG_TRANSPARENT
		// 枠を描画
		QPainter qp(sMainSuf->Get());

		qp.fillRect(data.re.x, data.re.y, data.re.w, data.re.h, bg);

		// 文字を描画
		qp.setPen(fg);
		qp.setFont(*data.font);
		qp.drawText(data.re.x + 2, data.re.y + 2, data.sz.cx - 4, data.sz.cy - 4, Qt::AlignLeft, QString::fromUtf8(it->cmsg));
		qp.end();
#endif
	}
}

/// 文字列をバックバッファに描画
///
/// @note ロックは呼び出し元で行うこと should be locked in caller function
void MsgBoard::draw_text(CSurface *suf, msg_data_t &data, int left, int top)
{
	VmRectWH re;

	if (enable && !data.lists.empty()) {
		list_t::iterator it = data.lists.begin();

		// 文字を描画
		QPainter qp(suf->Get());
		re.x = data.re.x + left; re.y = top;
		qp.setPen(fg);
		qp.setFont(*data.font);
		qp.drawText(re.x, re.y, data.sz.cx - 4, data.sz.cy - 4, Qt::AlignLeft, QString::fromUtf8(it->cmsg));
		qp.end();

	}
}

/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::Set(msg_data_t &data, const _TCHAR *str, int sec)
{
	item_t itm;

	UTILITY::tcs_to_mbs(itm.cmsg, str, MSGBOARD_STR_SIZE);
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
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::Set(msg_data_t &data, CMsg::Id id, int sec)
{
	Set(data, gMessages.Get(id), sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetMessage(const _TCHAR *str, int sec)
{
	Set(msg, str, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetMessage(CMsg::Id id, int sec)
{
	Set(msg, id, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetInfo(const _TCHAR *str, int sec)
{
	Set(info, str, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetInfo(CMsg::Id id, int sec)
{
	Set(info, id, sec);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetMessageF(const _TCHAR *format, ...)
{
	_TCHAR buf[MSGBOARD_STR_SIZE];

	va_list ap;

	va_start(ap, format);
	UTILITY::vstprintf(buf, sizeof(buf) / sizeof(buf[0]), format, ap);
	va_end(ap);

	SetMessage(buf);
}
/// メッセージ設定
///
/// @note メインスレッドとエミュスレッドから呼ばれる
void MsgBoard::SetInfoF(const _TCHAR *format, ...)
{
	_TCHAR buf[MSGBOARD_STR_SIZE];

	va_list ap;

	va_start(ap, format);
	UTILITY::vstprintf(buf, sizeof(buf) / sizeof(buf[0]), format, ap);
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
	char cmsg[MSGBOARD_STR_SIZE];

	data.mux->lock();

	if (!data.lists.empty()) {
		UTILITY::tcs_to_mbs(cmsg, str, MSGBOARD_STR_SIZE);
		list_t::iterator it = data.lists.begin();
		list_t::iterator it_next;

		while(it != data.lists.end())	{
			it_next = it;
			it_next++;

			if (strcmp(cmsg, it->cmsg) == 0) {
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
bool MsgBoard::set_sys_font(CMsg::Id title, const _TCHAR *name, int pt, QFont **font)
{
	QFontDatabase db;
	QFont valid_font = db.font(name, QString(), pt);
	valid_font.setPointSize(pt);
	*font = new QFont(valid_font);
	QByteArray font_name = valid_font.family().toUtf8();
	const char *xtitle = gMessages.Get(title);
	logging->out_logf_x(LOG_DEBUG, CMsg::MsgBoard_Use_VSTR_for_VSTR, font_name.data(), xtitle);
	return true;
}
