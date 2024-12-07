/** @file qt_sound.h

	Skelton for retropc emulator
	Qt edition

	@author Sasaji
	@date   2016.11.10

	@brief [ qt sound ]
*/

#ifndef QT_SOUND_H
#define QT_SOUND_H

#include <QObject>
#if QT_VERSION >= 0x060000
#include <QAudioSink>
#include <QAudioFormat>
#include <QTimer>
#else
#include <QAudioOutput>
#include <QAudioFormat>
#include <QThread>
#endif

class EMU;

#if QT_VERSION >= 0x060000
class MyAudioOutput : public QAudioSink
{
	Q_OBJECT

public:
	explicit MyAudioOutput(const QAudioFormat &format, QObject *parent, EMU *new_emu, int samples, int latency);
	~MyAudioOutput();
	void start();
	void stop();
	int BytesFree() const;
	void WriteTo(const char *buffer, qint64 len);

public slots:
	void notifySlot();
	void stateChangedSlot(QAudio::State state);

private:
	EMU *emu;
//	QThread *th;
	QIODevice *io;
	QTimer *timer;
};
#else
class MyAudioOutput : public QAudioOutput
{
	Q_OBJECT

public:
	explicit MyAudioOutput(const QAudioFormat &format, QObject *parent, EMU *new_emu, int samples, int latency);
	~MyAudioOutput();
	void start();
	void stop();
	QIODevice *getIODevice();
	int BytesFree() const;
//	void setVolume(qreal volume);
	void WriteTo(const char *buffer, qint64 len);

public slots:
	void notifySlot();
	void stateChangedSlot(QAudio::State state);

signals:

private:
	EMU *emu;
	QThread *th;
	QIODevice *io;
};
#endif

#endif // QT_SOUND_H

