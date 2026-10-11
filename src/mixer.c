
#include "host.h"
#include "mixer.h"
#include "music.h"

#define SAMPLES_COUNT 31

struct sample_t {
	uint16_t len;
	uint8_t finetune;
	uint8_t volume;
	uint16_t loop_start;
	uint16_t loop_len;
	const int8_t *data;
};

static struct sample_t _samples[SAMPLES_COUNT];

#define SOUND_CHANNELS 4

struct mixer_channel_t {
	const int8_t *buffer;
	uint32_t offset, size, step;
	int volume;
};

#define FRAC_BITS 16

static struct mixer_channel_t _channels[SOUND_CHANNELS];
static MixerLockProc _lock;

static int _rate;

static bool _music_enabled;
static bool _jingles_enabled;
static bool _sounds_enabled;

#define PAULA_FREQ 3546897

struct paula_channel_t {
	struct sample_t *sample;
	const int8_t *data;
	const int8_t *data_loop;
	uint32_t length;
	uint32_t length_loop;
	uint32_t offset, step;
	int volume;
	int period;
	int panning;
};

#define PAULA_CHANNELS 4

static struct paula_channel_t _paula_channels[PAULA_CHANNELS];
static void (*_paula_interrupt)();

#define MAX_VOLUME 96

static const uint8_t *_smp;
static int _smp_size;

static const uint8_t *_uds;
static int _uds_size;

static int _music_order_num, _jingle_order_num;

static int _music_samples_left;

static const uint8_t SIG_SMP[] = { 0x55, 0x07, 0x44, 0x07 };

static const uint8_t SIG_UDS[] = { 0x53, 0x8F, 0x4E, 0x47 };

static struct mixer_channel_t *find_free_channel() {
	for (int i = 0; i < SOUND_CHANNELS; ++i) {
		if (!_channels[i].buffer) {
			return &_channels[i];
		}
	}

	return 0;
}

static void free_channel(struct mixer_channel_t *channel) {
	memset(channel, 0, sizeof(struct mixer_channel_t));
}

static void Paula_GetSampleInfo(int sample, int *finetune, int *volume) {
	assert(sample != 0);
	--sample;
	assert(sample >= 0 && sample < SAMPLES_COUNT);
	*finetune = _samples[sample].finetune;
	*volume   = _samples[sample].volume;
}

static void Paula_SetChannelVolume(int channel, int volume) {
	assert(channel >= 0 && channel < PAULA_CHANNELS);

	_paula_channels[channel].volume = volume;
}

static void Paula_SetChannelPeriod(int channel, int period) {
	assert(channel >= 0 && channel < PAULA_CHANNELS);

	_paula_channels[channel].period = period;
	if (period != 0) {
		const int freq = PAULA_FREQ / period;
		_paula_channels[channel].step = (freq << FRAC_BITS) / _rate;
	}
}

static void Paula_SetChannelSample(int channel, int sample) {
	assert(channel >= 0 && channel < PAULA_CHANNELS);

	if (sample == 0) {
		_paula_channels[channel].data = 0;
	} else {
		--sample;
		assert(sample >= 0 && sample < SAMPLES_COUNT);
		_paula_channels[channel].sample = &_samples[sample];
		_paula_channels[channel].data = _samples[sample].data;
		_paula_channels[channel].length = _samples[sample].len * 2;
		if (_samples[sample].loop_start > 2) {
			_paula_channels[channel].data_loop = _samples[sample].data + _samples[sample].loop_start * 2;
			_paula_channels[channel].length_loop = _samples[sample].loop_len * 2;
		} else {
			_paula_channels[channel].data_loop = 0;
			_paula_channels[channel].length_loop = 0;
		}
		_paula_channels[channel].offset = 0;
	}
}

static void Paula_SetChannelOffset(int channel, int offset) {
	assert(channel >= 0 && channel < PAULA_CHANNELS);
	_paula_channels[channel].offset = offset;
}

static void Paula_SetInterruptProc(void (*proc)()) {
	_paula_interrupt = proc;
}

static struct Paula _paula = {
	Paula_GetSampleInfo,
	Paula_SetChannelVolume,
	Paula_SetChannelPeriod,
	Paula_SetChannelSample,
	Paula_SetChannelOffset,
	Paula_SetInterruptProc,
};

int Mixer_Init(int hz, MixerLockProc lock) {
	_rate = hz;
	_lock = lock;
	_music_enabled = 1;
	_jingles_enabled = 1;
	_sounds_enabled = 1;
	return 0;
}

int Mixer_Fini() {
	return 0;
}

void Mixer_SetSoundData(const uint8_t *data, int size) {
	g_host.Debug("Mixer_SetSoundData 0x%08x size:%d", READ_BE_UINT32(data), size);
	assert(memcmp(data, SIG_SMP, 4) == 0);
	if (!_lock(1)) return;
	_smp = data + 4;
	_smp_size = size - 4;

	int offset = 0;
	int prev_end = 0;
	for (int i = 0; i < SAMPLES_COUNT; ++i) {
		const uint32_t offset0 = READ_BE_UINT32(_smp + offset); offset += 4;
		const uint32_t offset1 = READ_BE_UINT32(_smp + offset); offset += 4;
		const uint32_t offset2 = READ_BE_UINT32(_smp + offset); offset += 4;
		int len = offset1 - offset0;
		const uint8_t volume = _smp[offset++];
		const uint8_t finetune = _smp[offset++];
		const uint16_t pad = READ_BE_UINT16(_smp + offset); offset += 2;

		assert(pad == 0);
		assert((offset0 & 1) == 0);
		assert((offset1 & 1) == 0);
		assert((offset2 & 1) == 0);
		if (i != 0 && prev_end != offset0) {

		}
		prev_end = offset1;
		if (offset2 != 0) {

		}
		if (len != 0 && offset1 >= size - 4) {
			g_host.Warn("sample oob %d,%d,%d len:%d size:%d for sample num:%d", offset0, offset1, offset2, len, size, i);
			len = (size - 4) - offset0;
		}
		_samples[i].len = len / 2;
		_samples[i].finetune = finetune;
		_samples[i].volume = volume;
		if (offset2 != 0) {
			_samples[i].loop_start = (len - offset2) / 2;
			_samples[i].loop_len = offset2 / 2;
		}

		_samples[i].data = (const int8_t *)_smp + offset0;
	}
	_lock(0);
}

void Mixer_SetMusicData(const uint8_t *data, int size) {
	g_host.Debug("Mixer_SetMusicData 0x%08x size:%d", READ_BE_UINT32(data), size);
	assert(memcmp(data, SIG_UDS, 4) == 0);
	if (!_lock(1)) return;
	_uds = data;
	_uds_size = size;
	Music_Load(_uds, _uds_size, &_paula);
	_lock(0);
}

void Mixer_PlaySample(int num, int volume) {
	g_host.Debug("PlaySample num:%d", num);
	if (!_lock(1)) return;
	if (_sounds_enabled && _samples[num].len != 0) {
		struct mixer_channel_t *ch = find_free_channel();
		if (ch) {
			ch->buffer = _samples[num].data;
			ch->offset = 0;
			ch->size = _samples[num].len * sizeof(int16_t);
			const int freq = PAULA_FREQ / 0xa0;
			ch->step = (freq << FRAC_BITS) / _rate;
			ch->volume = volume;
		}
	}
	_lock(0);
}

void Mixer_PlayJingle(int num1, int num2) {
	g_host.Debug("PlayJingle music:%d jingle:%d", num1, num2);
	if (!_lock(1)) return;
	if (_music_enabled) {
		_music_order_num = num1;
		if (_music_order_num != -1) {
			Music_Play(_music_order_num);
			_music_order_num = -1;
		}
	}
	if (_jingles_enabled) {
		_jingle_order_num = num2;
		if (_jingle_order_num != -1) {
			Music_PlayJingle(_jingle_order_num);
			_jingle_order_num = -1;
		}
	}
	_lock(0);
}

static int16_t S8_to_S16(int8_t a) {
        return ((uint8_t)a) * 0x101;
}

static int16_t clipS16(int sample) {
	return ((sample < SHRT_MIN) ? SHRT_MIN : ((sample > SHRT_MAX) ? SHRT_MAX : sample));
}

static void mixPaula(struct paula_channel_t *ch, int16_t *samples, int len) {
	if (!ch->data || ch->period == 0) {
		return;
	}
	for (int i = 0; i < len * 2; i += 2) {
		uint32_t pos = ch->offset >> FRAC_BITS;
		if (pos >= ch->length) {
			ch->offset = 0;
			ch->data = ch->data_loop;
			ch->length = ch->length_loop;
			pos = ch->offset >> FRAC_BITS;
		}
		if (pos >= ch->length) {
			ch->data = 0;
			break;
		}
		const int16_t sample = S8_to_S16(ch->data[pos] * ch->volume / MAX_VOLUME);
		samples[i]     = clipS16(samples[i]     + sample);
		samples[i + 1] = clipS16(samples[i + 1] + sample);
		ch->offset += ch->step;
	}
}

void Mixer_Toggle(enum MixerToggle toggle) {
	if (!_lock(1)) return;
	switch (toggle) {
	case MIXER_TOGGLE_MUSIC:
		_music_enabled = !_music_enabled;
		break;
	case MIXER_TOGGLE_JINGLES:
		_jingles_enabled = !_jingles_enabled;
		break;
	case MIXER_TOGGLE_SOUNDS:
		_sounds_enabled = !_sounds_enabled;
		break;
	}
	_lock(0);
}

int Mixer_MixStereoS16(int16_t *samples, int len) {
	memset(samples, 0, sizeof(int16_t) * 2 * len);
	const int samples_per_tick = _rate / 50;
	int16_t *music_ptr = samples;
	int music_len = len;
	while (music_len != 0) {
		if (_music_samples_left == 0) {
			if (_paula_interrupt) {
				_paula_interrupt();
			}
			_music_samples_left = samples_per_tick;
		}
		int count = _music_samples_left;
		if (count > music_len) {
			count = music_len;
		}
		for (int ch = 0; ch < PAULA_CHANNELS; ++ch) {
			mixPaula(&_paula_channels[ch], music_ptr, count);
		}
		music_ptr += count * 2;
		music_len -= count;
		_music_samples_left -= count;
	}
	for (int i = 0; i < SOUND_CHANNELS; ++i) {
		struct mixer_channel_t *ch = &_channels[i];
		if (ch->buffer) {
			for (int i = 0; i < len * 2; i += 2) {
				const uint32_t pos = ch->offset >> FRAC_BITS;
				if (pos >= ch->size) {
					free_channel(ch);
					ch->buffer = 0;
					break;
				}
				const int16_t sample = S8_to_S16(ch->buffer[pos] * ch->volume / MAX_VOLUME);
				ch->offset += ch->step;
				samples[i]     = clipS16(samples[i]     + sample);
				samples[i + 1] = clipS16(samples[i + 1] + sample);
			}
		}
	}
	return 0;
}
