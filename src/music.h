
#ifndef MUSIC_H__
#define MUSIC_H__

#include "intern.h"

struct Paula {
	void (*GetSampleInfo)(int sample, int *finetune, int *volume);
	void (*SetChannelVolume)(int channel, int volume);
	void (*SetChannelPeriod)(int channel, int period);
	void (*SetChannelSample)(int channel, int sample);
	void (*SetChannelOffset)(int channel, int offset);
	void (*SetInterruptProc)(void (*proc)());
};

int Music_Load(const uint8_t *uds, int uds_size, struct Paula *paula);
void Music_Play(int num);
void Music_PlayJingle(int num);
void Music_Mix(int16_t *buf, int len);

#endif
