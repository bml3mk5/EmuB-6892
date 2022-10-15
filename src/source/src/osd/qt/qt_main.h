/** @file qt_main.h

	Skelton for retropc emulator
	Qt edition

	@author Sasaji
	@date   2016.11.15

	@brief [ qt_main ]
*/

#ifndef QT_MAIN_H
#define QT_MAIN_H

#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <QThread>

// signal to main loop
extern bool need_update_title;

// frame rate
//extern const int fps[6];
//extern const int fskip[6];
//extern int rec_fps_no;

/// for calculate frame rate
typedef struct st_frame_count {
	int total;
	int draw;
	int skip;
} t_frame_count;

extern t_frame_count frames_result;

extern int *key_mod;

/// emulation core
class EMU;
class GUI;
class CLocale;
class CParseOptions;

extern EMU *emu;
extern GUI *gui;
extern CParseOptions *options;
//extern QApplication *app;

///
/// @brief Main Window
///
class MyWidget : public QWidget
{
	Q_OBJECT

public:
	MyWidget(QWidget *parent);
	~MyWidget();

protected:
	void paintEvent(QPaintEvent *event);
};

///
/// @brief OpenGL panel in Window
///
class MyGLWidget : public QOpenGLWidget
{
	Q_OBJECT

public:
	MyGLWidget(QWidget *parent);
	~MyGLWidget();

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);
};


///
/// @brief Emulator Thread
///
class EmuThread : public QThread
{
    Q_OBJECT

public:
    EmuThread(QObject * parent);
	~EmuThread();
	void stop();

signals:
	void needUpdateScreen();
	void needUpdateTitle();

protected:
	void run();

private:
	bool working;
};

#endif // QT_MAIN_H
