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
#include <QAudioOutput>
#include <QThread>

class EMU;

class MyAudioOutput : public QAudioOutput
{
	Q_OBJECT

public:
	explicit MyAudioOutput(const QAudioFormat &format, QObject *parent, EMU *new_emu, int samples, int latency);
	~MyAudioOutput();
	void start();
	void stop();
	QIODevice *getIODevice();
//	int bytesFree();
//	void setVolume(qreal volume);

public slots:
	void notifySlot();
	void stateChangedSlot(QAudio::State state);

signals:

private:
	EMU *emu;
	QThread *th;
	QIODevice *io;
};

#endif // QT_SOUND_H

