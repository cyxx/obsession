
#ifndef HOST_H__
#define HOST_H__

#include "intern.h"

enum {
	HOST_SURFACES_COUNT = 2,
	AUDIO_SAMPLE_RATE = 22050,
	GAME_HZ = 60,
};

struct Host {
	bool quit;

	void (*Debug)(const char *fmt, ...);
	void (*Warn)(const char *fmt, ...);
	void (*Error)(const char *fmt, ...);

	void (*InitAudio)();
	void (*FiniAudio)();
	int (*LockAudio)(int lock);

	int (*LockSurface565)(int handle, uint16_t **p, int *pitch);
	int (*UnlockSurface565)(int handle);
	uint16_t (*MapRGB565)(int handle, uint16_t rgb444);

	FILE *(*OpenAsset)(const char *filename);
};

extern struct Host g_host;

#endif
