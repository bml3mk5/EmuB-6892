/** @file qt_sound.cpp

	Skelton for retropc emulator
	SDL edition

	@author Sasaji
	@date   2012.02.21

	@brief [ sdl sound ]

	@note
	This code is based on win32_sound.cpp of the Common Source Code Project.
	Author : Takeda.Toshiya
*/

#include "qt_emu.h"
#include "../../vm/vm.h"
#include "../../config.h"
#include "../../video/rec_audio.h"
#include <QAudioOutput>
#include <QAudioFormat>
#include <QThread>
#include "qt_sound.h"
#include "qt_main.h"


MyAudioOutput::MyAudioOutput(const QAudioFormat &format, QObject *parent, EMU *new_emu, int samples, int latency) :
	QAudioOutput(format, parent),
	emu(new_emu)
{
	io = nullptr;

	setBufferSize(samples * format.channelCount() * format.sampleSize() / 8 * 2);
	setNotifyInterval(latency);
	setVolume(1.0);

	QObject::connect(this, SIGNAL(notify()), this, SLOT(notifySlot()));
	QObject::connect(this, SIGNAL(stateChanged(QAudio::State)), this, SLOT(stateChangedSlot(QAudio::State)));

	th = new QThread();
	moveToThread(th);
}
MyAudioOutput::~MyAudioOutput()
{
	th->exit();
	th->wait();
	delete th;
}
void MyAudioOutput::notifySlot()
{
	EMU_OSD *emu_osd = dynamic_cast<EMU_OSD *>(emu);
	emu_osd->update_sound();
}
void MyAudioOutput::stateChangedSlot(QAudio::State state)
{
	logging->out_debugf("MyAudioOutput::stateChanged: %d", state);
	if (state == QAudio::IdleState) {
		EMU_OSD *emu_osd = dynamic_cast<EMU_OSD *>(emu);
		emu_osd->update_sound();
	} else if (state == QAudio::StoppedState) {
//		if (sound->error() == QAudio::UnderrunError) {
//			// restart sound
//			io = sound->start();
//		}
	}
}
void MyAudioOutput::start()
{
	if (!th->isRunning()) th->start();

	io = QAudioOutput::start();
	QAudioOutput::suspend();
	QAudioOutput::resume();
}
void MyAudioOutput::stop()
{
	QAudioOutput::stop();
	io = nullptr;
}
QIODevice *MyAudioOutput::getIODevice()
{
	return io;
}

//
//
//

void EMU_OSD::EMU_SOUND()
{
	sound_prev_time = 0;

	rec_audio = new REC_AUDIO(this);
}

void EMU_OSD::initialize_sound(int rate, int samples, int latency)
{
	EMU::initialize_sound(rate, samples, latency);

	QAudioFormat fmt;
	fmt.setSampleRate(rate);
	fmt.setSampleType(QAudioFormat::SignedInt);
	fmt.setChannelCount(2);
	fmt.setSampleSize(16);
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setCodec("audio/pcm"); // Linear PCM

	sound = new MyAudioOutput(fmt, Q_NULLPTR, this, samples, latency);
	if (!sound) {
		return;
	}

	logging->out_logf(LOG_INFO, _T("sound ok: rate:%d samples:%d latency:%d"), rate, samples, latency);
	sound_ok = true;
}

void EMU_OSD::initialize_sound()
{
	EMU::initialize_sound();
}

void EMU_OSD::release_sound()
{
	if (sound) {
		delete sound;
		sound = nullptr;
	}
	delete rec_audio;
	sound_ok = false;
}

void EMU_OSD::release_sound_on_emu_thread()
{
	end_sound();
}

void EMU_OSD::start_sound()
{
	if (sound) {
		sound->start();
	}

	sound_ok = true;
}

void EMU_OSD::end_sound()
{
	if (sound) {
		sound->stop();
	}

	sound_ok = false;

	// stop recording
	stop_rec_sound();
}

void EMU_OSD::update_sound()
{
	int extra_frames = 0;
	int samples = (sound->bytesFree() >> 2);
	if (samples > sound_samples) samples = sound_samples;

//	out_debugf(_T("EMU_OSD::update_sound st sz:%d period:%d free:%d"), sound->bufferSize(), sound->periodSize(), sound->bytesFree());
	QIODevice *io = sound->getIODevice();
	if(io != nullptr && samples > 0) {
		// sound buffer must be updated
		int16_t* sound_buffer = vm->create_sound(&extra_frames, samples);
#ifndef SOUND_RECORD_IN_MIXER
		record_sound(sound_buffer, samples);
#endif
		if(sound_buffer) {
			io->write(reinterpret_cast<const char *>(sound_buffer), samples << 2);
		}
//		out_debugf(_T("EMU_OSD::update_sound ed f:%d sz:%d period:%d free:%d"), extra_frames, sound->bufferSize(), sound->periodSize(), sound->bytesFree());
//	} else {
//		out_debug(_T("EMU_OSD::update_sound ed skipped"));
	}

}

void EMU_OSD::mute_sound(bool mute)
{
	if (mute) {
// sound is continuous driving during mute
		sound->setVolume(0.0);
		now_mute = true;
	} else {
		if (sound_ok && !vm_pause){
			sound->setVolume(1.0);
			now_mute = false;
		}
	}
}

#if 0
void EMU_OSD::set_volume(int UNUSED_PARAM(volume))
{
	// set volume for sound devices.
	if (vm) {
		vm->set_volume();
	}
}

void EMU_OSD::start_rec_sound(int type)
{
#ifdef USE_REC_AUDIO
	rec_audio->Start(type, sound_rate, false);
#endif
}

void EMU_OSD::stop_rec_sound()
{
#ifdef USE_REC_AUDIO
	rec_audio->Stop();
#endif
}

void EMU_OSD::restart_rec_sound()
{
#ifdef USE_REC_AUDIO
	rec_audio->Restart();
#endif
}
#endif

#if 0
void EMU_OSD::record_rec_sound(uint8_t *buffer, int samples)
{
#ifdef USE_REC_AUDIO
	rec_audio->Record(buffer, samples);
#endif
}

void EMU_OSD::record_rec_sound(int16_t *buffer, int samples)
{
#ifdef USE_REC_AUDIO
	rec_audio->Record(buffer, samples);
#endif
}
#endif

#if 0
void EMU_OSD::record_rec_sound(int32_t *buffer, int samples)
{
#ifdef SOUND_RECORD_IN_MIXER
#ifdef USE_REC_AUDIO
	rec_audio->Record(buffer, samples);
#endif
#endif
}

bool EMU_OSD::now_rec_sound()
{
	return rec_audio->NowRecording();
}

const _TCHAR **EMU_OSD::get_rec_sound_codec_list(int type)
{
#ifdef USE_REC_AUDIO
	return rec_audio->GetCodecList(type);
#else
	return nullptr;
#endif
}

bool *EMU_OSD::get_now_rec_sound_ptr()
{
	return rec_audio->GetNowRecordingPtr();
}

bool EMU_OSD::rec_sound_enabled(int type)
{
#ifdef USE_REC_AUDIO
	return rec_audio->IsEnabled(type);
#else
	return false;
#endif
}
#endif

void EMU_OSD::lock_sound_buffer()
{
}
void EMU_OSD::unlock_sound_buffer()
{
}
