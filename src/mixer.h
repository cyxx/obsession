
#ifndef MIXER_H__
#define MIXER_H__

#include "intern.h"

typedef int (*MixerLockProc)(int);

int Mixer_Init(int hz, MixerLockProc lockProc);
int Mixer_Fini();

void Mixer_SetSoundData(const uint8_t *data, int size);
void Mixer_SetMusicData(const uint8_t *data, int size);

void Mixer_PlaySample(int num, int volume);
void Mixer_PlayJingle(int num1, int num2);

enum MixerToggle {
	MIXER_TOGGLE_MUSIC,
	MIXER_TOGGLE_JINGLES,
	MIXER_TOGGLE_SOUNDS,
};

void Mixer_Toggle(enum MixerToggle toggle);

int Mixer_MixStereoS16(int16_t *samples, int len);

#endif
