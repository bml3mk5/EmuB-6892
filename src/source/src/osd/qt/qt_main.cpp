/** @file qt_main.cpp

	Skelton for retropc emulator
	Qt edition

	@author Sasaji
	@date	2016.11.15

	@brief [ qt_main ]

	@note
	This code is based on winmain.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "qt_main.h"
#include <QString>
#include <QPainter>
#include <QScreen>
#include <QThread>
#include <QTime>

#include "../../common.h"
#include "../../config.h"
#include "qt_emu.h"
#include "../../emumsg.h"
#include "../../gui/gui.h"
#include "../../utility.h"
#include "../../clocale.h"
#include "qt_parseopt.h"
#include "qt_utils.h"
//#include "qt_clocale.h"


#undef LOG_MEASURE

#define MIN_INTERVAL	4

#define CALC_NEXT_INTERVAL(now_ms, interval_ms, delta_ms) { \
	interval_ms = now_ms / FRAMES_PER_SEC; \
	now_ms = now_ms - (interval_ms * FRAMES_PER_SEC) + delta_ms; \
}

// variable

/// emulation core
EMU *emu = nullptr;
GUI *gui = nullptr;
CParseOptions *options = nullptr;

/// ALT key
//int *key_mod = nullptr;

bool need_update_title = false;
//SDL_mutex *mux_allow_update_screen = nullptr;
//SDL_cond *cond_allow_update_screen = nullptr;

t_frame_count frames_result;

/// timing control
#define MAX_SKIP_FRAMES 10

static QTime *timer = nullptr;
static EmuThread *emu_thread = nullptr;

//
//
//

int main(int argc, char *argv[])
{
	int rc;

	MyWidget *sc = nullptr;
	MyGLWidget *scgl = nullptr;

	QApplication app(argc, argv);

	timer = new QTime();
	timer->start();

	QString app_path = app.applicationDirPath();
#if 0
	QTranslator translator;
	QLocale locale = QLocale::system();
//	QLocale::Country cont = locale.country();
//	QLocale::Language lang = locale.language();
    QString res_path = app_path;
#ifdef Q_OS_MACX
    res_path = app_path.append("/../Resources/locale");
#else
	res_path = res_path.append("/locale");
#endif
    bool trans_ok = translator.load(locale, CONFIG_NAME, "_", res_path, ".qm");
	if (trans_ok) {
		app.installTranslator(&translator);
	}
#endif

    // get and parse options
	options = new CParseOptions(argc, argv);

	// logging
	logging = new Logging(options->get_ini_path());

	// i18n
#ifdef Q_OS_MACX
    clocale = new CLocale(options->get_res_path(), CONFIG_NAME, "");
#else
	clocale = new CLocale(app_path, CONFIG_NAME, "");
#endif

	// create a instance of emulation core
	emu = new EMU_OSD(options->get_app_path(), options->get_ini_path(), options->get_res_path());
	logging->set_receiver(emu);
//	if (!clocale->IsOk()) {
//		logging->out_log(LOG_WARN, _T("Locale initialize failed."));
//	}
//	EMU_OSD *emu_osd = dynamic_cast<EMU_OSD *>(emu);

	// load config
	pconfig = new Config;
	config.load(options->get_ini_file());

	// change locale if need
	clocale->ChangeLocaleIfNeed(config.language);
	logging->out_logc(LOG_INFO, _T("Locale:["), clocale->GetLocaleName(), _T("] Lang:["), clocale->GetLanguageName(), _T("]"), nullptr);

	gui = new GUI(argc, argv, emu);
	rc = gui->Init();
	if (rc == -1) {
		rc = 1;
	} else if (rc == 1) {
		// use english message.
//		clocale->UnsetLocale();
//		logging->out_log(LOG_WARN, _T("Use default locale message."));
		rc = 0;
	}
	while (!rc) {
		emu->set_gui(gui);

#ifdef USE_OPENGL
		emu->set_use_opengl(config.use_opengl);
#endif

		// create window
		emu->init_screen_mode();
		if (!emu->create_screen(0, config.window_position_x, config.window_position_y, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, 0)) {
			break;
		}

		// create global keys
		if (gui->CreateGlobalKeys() == -1) {
			break;
		}

		// initialize emulation core
		emu->initialize();
		// restore screen mode
		if(config.window_mode >= 0 && config.window_mode < 8) {
			emu->change_screen_mode(config.window_mode);
		}
		else if(config.window_mode >= 8) {
			int prev_mode = config.window_mode;
			config.window_mode = 0;	// initialize window mode
			emu->change_screen_mode(prev_mode);
		}
		// use offscreen surface
		if (!emu->create_offlinesurface()) {
			break;
		}
//		// set again
//		emu->set_window(config.window_mode, desktop_width, desktop_height);

//		key_mod = emu->get_key_mod_ptr();

		// vm start
		if (emu) {
			emu->reset();
		}

		// open files
		options->open_recent_file(gui);

		// init mutex
	//	mux_allow_update_screen = SDL_CreateMutex();
	//	cond_allow_update_screen =SDL_CreateCond();

		if (mainwindow->isValid()) {
			if (config.use_opengl) {
				scgl = new MyGLWidget(mainwindow);
				mainwindow->setCentralWidget(scgl);
				scgl->resize(config.screen_width, config.screen_height);
			} else {
				sc = new MyWidget(mainwindow);
				mainwindow->setCentralWidget(sc);
				sc->resize(config.screen_width, config.screen_height);
			}
			app.setFont(QApplication::font("QMenu"));

			mainwindow->show();

			// create emulator thread
			emu_thread = new EmuThread(mainwindow);
			if (!emu_thread) {
				logging->out_logf(LOG_ERROR, _T("Create EmuThread."));
				break;
			}
			// connect signal
			QObject::connect(emu_thread, SIGNAL(needUpdateScreen()), mainwindow, SLOT(updateScreenSlot()));
//			QObject::connect(emu_thread, SIGNAL(needUpdateScreen()), mainwindow, SLOT(repaint()));
			QObject::connect(emu_thread, SIGNAL(needUpdateTitle()), mainwindow, SLOT(updateTitleSlot()));

			emu_thread->start();

			// event loop
			rc = app.exec();
		}
		delete scgl;
		delete sc;

		// wait thread
		if (emu_thread) {
			emu_thread->stop();
			emu_thread->wait();
		}
		emu_thread = nullptr;

		// release emulation core
		if(emu) {
			emu->release();
		}

		delete mainwindow;
		mainwindow = nullptr;

		break;
	}

	// save config
	config.save();
	config.release();

	delete pconfig;
	delete gui;
	logging->set_receiver(nullptr);
	delete emu;
	delete clocale;
	delete logging;
	delete options;

	delete timer;

	return rc;
}

//
// Main screen
//
MyWidget::MyWidget(QWidget *parent) :
	QWidget(parent)
{
}

MyWidget::~MyWidget()
{
}

void MyWidget::paintEvent(QPaintEvent *)
{
	EMU_OSD *emu_osd = dynamic_cast<EMU_OSD *>(emu);
	QPainter painter(this);
	emu_osd->update_screen_pa(&painter);
	painter.end();
	gui->UpdatedScreen();
}

//
// OpenGL screen
//
MyGLWidget::MyGLWidget(QWidget *parent) :
	QOpenGLWidget(parent)
{
}

MyGLWidget::~MyGLWidget()
{
}

void MyGLWidget::initializeGL()
{
//	QOpenGLFunctions *gf = QOpenGLContext::currentContext()->functions();

	EMU_OSD *emu_osd = dynamic_cast<EMU_OSD *>(emu);
	emu_osd->realize_opengl(this);

	setUpdatesEnabled(true);
}

void MyGLWidget::paintGL()
{
	EMU_OSD *emu_osd = dynamic_cast<EMU_OSD *>(emu);
	emu_osd->update_screen_gl(this->context());
	gui->UpdatedScreen();

//	this->context()->swapBuffers(this);
}

void MyGLWidget::resizeGL(int w, int h)
{
//	if ( !IsShownOnScreen() )
//		return;

//	SetCurrent(*glcontext);

//	int cw, ch;
	QWidget *p = dynamic_cast<QWidget *>(this->parent());
	if (!p) return;

	EMU_OSD *emu_osd = dynamic_cast<EMU_OSD *>(emu);
	emu_osd->set_mode_opengl(this, w, h);

	// set swap interval
	emu_osd->set_interval_opengl();
}

//
// EMU Thread
//
EmuThread::EmuThread(QObject * parent) :
	QThread(parent)
{
	working = false;
}
EmuThread::~EmuThread()
{
}

void EmuThread::stop()
{
	working = false;
}

/**
 *	emulator thread event loop
 *
 *	@attention called by another thread
 */
void EmuThread::run()
{
	working = true;

	const int fskip[6] = {1, 2, 3, 4, 5, 6};
	int fskip_remain = 0;

	t_frame_count frames = { 0, 0, 0 };

	uint32_t ms = 1000;
	uint32_t next_interval;
	CALC_NEXT_INTERVAL(ms, next_interval, 1000)

	int split_num = 0;

	uint32_t current_time = 0;
	int    remain_time = 0;
	uint32_t ideal_next_time = 0;
	uint32_t real_next_time = 0;
#ifdef LOG_MEASURE
	uint32_t skip_reason = 0;
#endif
#ifdef USE_PERFORMANCE_METER
	uint32_t lpCount1,lpCount2,lpCount3;
	lpCount1 = lpCount2 = lpCount3 = 0;
#endif

	// wait a sec.
	msleep(500);

	uint32_t update_fps_time = static_cast<uint32_t>(timer->elapsed()) + 1000;

	// play sound
	emu->mute_sound(false);

	// process coming messages at first.
	while (emumsg.Process()) {}

	ideal_next_time = static_cast<uint32_t>(timer->elapsed());

#ifdef USE_PERFORMANCE_METER
	lpCount1 = ideal_next_time;
#endif

	// loop
	while (working) {
		split_num = 0;
		// get next period
		CALC_NEXT_INTERVAL(ms, next_interval, 1000)

//		start_time = next_time;
//		next_time += emu->now_skip() ? 0 : next_interval;
		ideal_next_time += next_interval;
		real_next_time += next_interval;
		current_time = static_cast<uint32_t>(timer->elapsed());
#ifdef LOG_MEASURE
		skip_reason = 0;
#endif

		// reset ideal time when proccesing is too late...
		if (current_time > ideal_next_time + 200) {
			ideal_next_time = current_time + next_interval;
#ifdef LOG_MEASURE
			skip_reason = 0x0200;
#endif
		}
		// sync next time when the real time have one frame difference from ideal time.
		if ((real_next_time > (next_interval + ideal_next_time)) || (ideal_next_time > (next_interval + real_next_time))) {
			real_next_time = ideal_next_time;
#ifdef LOG_MEASURE
			skip_reason = 0x0100;
#endif
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A1: cur:%ld nt:%ld %ld ms:%d int:%d st:%04x", current_time, ideal_next_time, real_next_time, ms, next_interval, skip_reason);
#endif
#if (FRAME_SPLIT_NUM > 1)
		for(int i=0; i < (FRAME_SPLIT_NUM - 1); i++) {
			// drive machine
			if(emu)	emu->run(split_num);
			split_num++;
			while (emumsg.Process()) {}
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A2: rti:%ld nt:%ld",SDL_GetTicks(),ideal_next_time);
#endif
#endif
		if(emu) {
			// drive machine
			emu->run(split_num);

			// record video
			emu->record_rec_video();

			frames.total++;

			if(config.fps_no >= 0) {
				if (fskip_remain <= 0) {
					// constant frames per 1 second
					if (gui->NeedUpdateScreen()) {
#ifndef USE_QT_UPDATE
						emit needUpdateScreen();
#endif
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
					fskip_remain = fskip[config.fps_no];
#ifdef LOG_MEASURE
					skip_reason |= 0x11;
#endif
				} else {
					frames.skip++;
#ifdef LOG_MEASURE
					skip_reason |= 0x12;
#endif
				}
				if (fskip_remain > 0) fskip_remain--;
			}
			else {
				current_time = static_cast<uint32_t>(timer->elapsed());
				if(real_next_time > current_time) {
					// update window if enough time
					if (gui->NeedUpdateScreen()) {
#ifndef USE_QT_UPDATE
						emit needUpdateScreen();
#endif
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
#ifdef LOG_MEASURE
					skip_reason |= 0x01;
#endif
				}
				else if(frames.skip > MAX_SKIP_FRAMES) {
					// update window at least once per 10 frames
					if (gui->NeedUpdateScreen()) {
#ifndef USE_QT_UPDATE
						emit needUpdateScreen();
#endif
						frames.draw++;
						frames.skip = 0;
					} else {
						frames.skip++;
					}
#ifdef LOG_MEASURE
					skip_reason |= 0x02;
#endif
//					real_next_time = SDL_GetTicks();
				}
				else {
					frames.skip++;
#ifdef LOG_MEASURE
					skip_reason |= 0x03;
#endif
				}
			}
			emu->skip_screen(frames.skip > 0);
		}
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A3: cur:%ld nt:%ld (total:%d fps:%d skip:%d) st:%04x",current_time,ideal_next_time,frames.total,frames.draw,frames.skip,skip_reason);
#endif
		gui->PostDrive();

		while (emumsg.Process()) {}
		if (frames.skip > 0) {
			real_next_time -= 2;
		}
		current_time = static_cast<uint32_t>(timer->elapsed());
#ifdef USE_PERFORMANCE_METER
		if (config.show_pmeter) {
			lpCount2 = current_time;
		}
#endif
		remain_time = static_cast<int>(real_next_time - current_time);
#if defined(__APPLE__) && defined(__MACH__)
		if (remain_time > 3) {
			msleep(remain_time - 2);
		}
#else
		if (remain_time > 3) {
			msleep(static_cast<unsigned long>(remain_time - 1));
		}
#endif
#ifdef LOG_MEASURE
		logging->out_logf(LOG_DEBUG, "A4: rti:%ld nt:%ld %ld fu:%ld",timer->elapsed(),ideal_next_time,real_next_time,update_fps_time);
#endif
		// calc frame rate
		current_time = static_cast<uint32_t>(timer->elapsed());
		if(update_fps_time <= current_time + 8) {
			frames_result = frames;
			need_update_title = true;
			emit needUpdateTitle();

			update_fps_time += 1000;
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
			frames.total = frames.draw = 0;
		}
#ifdef USE_PERFORMANCE_METER
		if (config.show_pmeter) {
			lpCount3 =  current_time;
			if (lpCount3 > lpCount1) {
				gdPMvalue = ((lpCount2 - lpCount1) * 100 / (lpCount3 - lpCount1)) & 0xfff;
			}
			lpCount1 = lpCount3;
		}
#endif
	}

	emu->release_on_emu_thread();

	this->exit(0);
}

