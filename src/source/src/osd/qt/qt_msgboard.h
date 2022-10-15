﻿/** @file qt_msgboard.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2012.06.26 -

	@brief [ message board ]
*/

#ifndef QT_MSGBOARD_H
#define QT_MSGBOARD_H

#include <list>
//#include <SDL.h>
//#include <SDL_ttf.h>
#include "../../common.h"
//#include <QPixelFormat>
//#include <QImage>
#include <QFont>
#include "qt_ccolor.h"
#include "../../cmutex.h"
#include "../../msgs.h"

class EMU;
class CSurface;
class CPixelFormat;

#define MSGBOARD_STR_SIZE	512

/**
	@brief MsgBoard is the class that display various messages on the screen.
*/
class MsgBoard
{
private:
	EMU *emu;
	CSurface *sMainSuf;

	CColor fg;
	CColor bg;

	struct {
		int cx;
		int cy;
	} szWin;

	typedef struct msg_item_st {
		char cmsg[MSGBOARD_STR_SIZE];
		int cnt;
	} item_t;

	typedef std::list<item_t> list_t;

	typedef struct msg_data_st {
		VmRectWH re;	// 描画枠
		struct {
			int cx;
			int cy;
		} sz;			// 描画した文字の幅、高さ
		struct {
			int x;
			int y;
		} pt;			// 描画先の位置
		int place;		// ptの基準 0:左上 1:右上 2:左下 3:右下
		QFont *font;
		CMutex *mux;
		list_t lists;
	} msg_data_t;

	msg_data_t msg;
	msg_data_t info;

	bool visible;
	bool enable;

	bool inited;

	typedef struct sbuf_st {
		char buf[1024];
	} sbuf_t;

	// 文字列出力
	void draw(CSurface *screen, msg_data_t &data);
	// 文字列をバックバッファに描画
	void draw_text(msg_data_t &data);
	void draw_text(CSurface *suf, msg_data_t &data, int left, int top);
	// カウントダウン
	void count_down(msg_data_t &data);

	// 画面表示用システムフォントの設定
	bool set_sys_font(CMsg::Id title, const _TCHAR *name, int pt, QFont **font);
public:
	MsgBoard(EMU *pEmu);
	~MsgBoard();

	// 初期化
	void CreateSurface(CPixelFormat *format, int width, int height);
	void InitScreen(CPixelFormat *format, int width, int height);

	// 表示
	void SetVisible(bool val) {
		visible = val;
	}
	// カウントダウン
	void CountDown(void);

	// メッセージ設定
	void Set(msg_data_t &data, const _TCHAR *str, int sec);
	void Set(msg_data_t &data, CMsg::Id id, int sec);
	// メッセージ削除
	void Delete(msg_data_t &data, const _TCHAR *str);
	void Delete(msg_data_t &data, CMsg::Id id);
	// ウィンドウサイズ設定
	void SetSize(int width, int height);

	// メッセージ位置設定
	void SetMessagePos(int cx, int cy, int place);
	// メッセージ設定
	void SetMessage(const _TCHAR *str, int sec = 5);
	void SetMessage(CMsg::Id id, int sec = 5);
	void SetMessageF(const _TCHAR *format, ...);
	// メッセージ削除
	void DeleteMessage(const _TCHAR *str);
	void DeleteMessage(CMsg::Id id);

	// 情報位置設定
	void SetInfoPos(int cx, int cy, int place);
	// 情報設定
	void SetInfo(const _TCHAR *str, int sec = 2);
	void SetInfo(CMsg::Id id, int sec = 2);
	void SetInfoF(const _TCHAR *format, ...);
	// 情報削除
	void DeleteInfo(const _TCHAR *str);
	void DeleteInfo(CMsg::Id id);

	// 文字列出力
	void Draw(CSurface *screen);

	// フォント設定
	bool SetFont();

	// 有効？
	bool IsEnable() { return enable; }
};

#endif /* QT_MSGBOARD_H */
