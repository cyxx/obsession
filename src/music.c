
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "music.h"

#define PAULA_FREQ 3546897

static uint8_t _dat[512 * 1024];
static int _dat_size;

static const uint16_t PERIODS[36 * 16] = {
	856,808,762,720,678,640,604,570,538,508,480,453,428,404,381,360,339,320,302,285,269,254,240,226,214,202,190,180,170,160,151,143,135,127,120,113,
	850,802,757,715,674,637,601,567,535,505,477,450,425,401,379,357,337,318,300,284,268,253,239,225,213,201,189,179,169,159,150,142,134,126,119,113,
	844,796,752,709,670,632,597,563,532,502,474,447,422,398,376,355,335,316,298,282,266,251,237,224,211,199,188,177,167,158,149,141,133,125,118,112,
	838,791,746,704,665,628,592,559,528,498,470,444,419,395,373,352,332,314,296,280,264,249,235,222,209,198,187,176,166,157,148,140,132,125,118,111,
	832,785,741,699,660,623,588,555,524,495,467,441,416,392,370,350,330,312,294,278,262,247,233,220,208,196,185,175,165,156,147,139,131,124,117,110,
	826,779,736,694,655,619,584,551,520,491,463,437,413,390,368,347,328,309,292,276,260,245,232,219,206,195,184,174,164,155,146,138,130,123,116,109,
	820,774,730,689,651,614,580,547,516,487,460,434,410,387,365,345,325,307,290,274,258,244,230,217,205,193,183,172,163,154,145,137,129,122,115,109,
	814,768,725,684,646,610,575,543,513,484,457,431,407,384,363,342,323,305,288,272,256,242,228,216,204,192,181,171,161,152,144,136,128,121,114,108,
	907,856,808,762,720,678,640,604,570,538,508,480,453,428,404,381,360,339,320,302,285,269,254,240,226,214,202,190,180,170,160,151,143,135,127,120,
	900,850,802,757,715,675,636,601,567,535,505,477,450,425,401,379,357,337,318,300,284,268,253,238,225,212,200,189,179,169,159,150,142,134,126,119,
	894,844,796,752,709,670,632,597,563,532,502,474,447,422,398,376,355,335,316,298,282,266,251,237,223,211,199,188,177,167,158,149,141,133,125,118,
	887,838,791,746,704,665,628,592,559,528,498,470,444,419,395,373,352,332,314,296,280,264,249,235,222,209,198,187,176,166,157,148,140,132,125,118,
	881,832,785,741,699,660,623,588,555,524,494,467,441,416,392,370,350,330,312,294,278,262,247,233,220,208,196,185,175,165,156,147,139,131,123,117,
	875,826,779,736,694,655,619,584,551,520,491,463,437,413,390,368,347,328,309,292,276,260,245,232,219,206,195,184,174,164,155,146,138,130,123,116,
	868,820,774,730,689,651,614,580,547,516,487,460,434,410,387,365,345,325,307,290,274,258,244,230,217,205,193,183,172,163,154,145,137,129,122,115,
	862,814,768,725,684,646,610,575,543,513,484,457,431,407,384,363,342,323,305,288,272,256,242,228,216,203,192,181,171,161,152,144,136,128,121,114,
};

static const int16_t SIN_TABLE[64] = {
	   0,   24,   49,   74,   97,  120,  141,  161,
	 180,  197,  212,  224,  235,  244,  250,  253,
	 255,  253,  250,  244,  235,  224,  212,  197,
	 180,  161,  141,  120,   97,   74,   49,   24,
	   0,  -24,  -49,  -74,  -97, -120, -141, -161,
	-180, -197, -212, -224, -235, -244, -250, -253,
	-255, -253, -250, -244, -235, -224, -212, -197,
	-180, -161, -141, -120,  -97,  -74,  -49,  -24
};

static uint8_t _music_data_hdr_notes_index_size;
static uint8_t _music_data_hdr_channels_count;
static const uint8_t *_music_restart_pos;
static const uint8_t *_music_data_orders;
static const uint8_t *_music_data_patterns;
static const uint8_t *_music_data_notes;

static struct Paula *_paula;

static int find_period_index(int period, int finetune) {
	for (int p = 0; p < 36; ++p) {
		if (PERIODS[p] == period) {
			return finetune * 36 + p;
		}
	}
	fprintf(stdout, "Invalid period:%d finetune:%d\n", period, finetune);
	return 0;
}

static uint16_t rol(uint8_t val) {
	const int shift = _music_data_hdr_notes_index_size + 5;
	return (val << shift) | (val >> (15 - shift));
}

static void load_music(const uint8_t *data) {
	int val;

	val = data[5];
	_music_data_hdr_notes_index_size = 1;
	if (val != 0x42) {
		_music_data_hdr_notes_index_size = 2;
	}

	val = data[6];
	_music_data_hdr_channels_count = 4;
	if (val != 0x34) {
		_music_data_hdr_channels_count = 8;
	}

	const uint8_t *p = &data[8];
	_music_data_orders = p;

	val = data[7];
	if (val == 0x7F) {
		val = 0;
	}
	val *= _music_data_hdr_channels_count;

	p = &data[8 + val];
	_music_restart_pos = p;

	p = data + 8;
	while ((int16_t)READ_BE_UINT16(p++) != -8888);

	_music_data_patterns = p + 1;
	assert((int16_t)READ_BE_UINT16(_music_data_patterns - 2) == -8888);

	while ((int16_t)READ_BE_UINT16(p++) != -9999);

	_music_data_notes = p + 1;
	assert((int16_t)READ_BE_UINT16(_music_data_notes - 2) == -9999);
}

struct note_t {
	uint8_t sample;
	uint16_t period;
	uint8_t effect;
	uint8_t param;
};

struct mod_pattern_t {
	uint32_t input;
	struct note_t note[64][4];
};

static int _mod_orders_count;
static uint8_t _mod_orders[128];
static int _mod_patterns_count;
static struct mod_pattern_t _mod_patterns[128];

static void convert_note(uint32_t note, struct note_t *n) {
	int period = ((note >> 24) & 0xFF);
	assert((period & 1) == 0);
	int sample = ((note >> 16) & 0xFF);
	assert(sample < 32);
	const int func = ((note >> 8) & 0xFF);
	assert((func & 3) == 0);
	const int param = (note & 0xFF);

	if (period != 0) {
		period = PERIODS[period / 2 - 1];
	}
	n->sample = sample;
	n->period = period;
	n->effect = func >> 2;
	n->param = param;
}

static void convert_pattern_data() {
	const int size_orders_data = _music_data_patterns - 2 - _music_data_orders;
	assert((size_orders_data % _music_data_hdr_channels_count) == 0);

	const int size_notes_data = _dat_size - (_music_data_notes - _dat);
	int a = 0;
	while (a < size_orders_data) {
		assert(_music_data_hdr_channels_count <= 4);
		uint32_t input = 0;
		const uint8_t *orders[4];
		for (int channel = 0; channel < _music_data_hdr_channels_count; ++channel) {
			const uint8_t val = _music_data_orders[a++];

			const uint8_t *order = _music_data_patterns + rol(val);
			orders[channel] = order;
			input |= val << (8 * channel);
		}
		int found = 0;
		for (int i = 0; i < _mod_patterns_count; ++i) {
			if (_mod_patterns[i].input == input) {

				found = 1;
				assert(_mod_orders_count < 128);
				_mod_orders[_mod_orders_count++] = i;
				break;
			}
		}
		if (found) {
			continue;
		}
		assert(_mod_orders_count < 128);
		_mod_orders[_mod_orders_count++] = _mod_patterns_count;
		assert(_mod_patterns_count < 128);
		struct mod_pattern_t *pattern = &_mod_patterns[_mod_patterns_count++];
		pattern->input = input;

		for (int row = 0; row < 64; ++row) {
			for (int channel = 0; channel < _music_data_hdr_channels_count; ++channel) {
				int num;
				if (_music_data_hdr_notes_index_size == 1) {
					num = orders[channel][row];
				} else {
					num = READ_BE_UINT16(orders[channel] + row * sizeof(uint16_t));
				}

				assert(num * 4 < size_notes_data);
				const uint32_t note = READ_BE_UINT32(_music_data_notes + num * 4);
				struct note_t *n = &pattern->note[row][channel];
				convert_note(note, n);
			}
		}
	}

}

#define NUM_TRACKS 4

struct track_t {
	uint8_t sample;
	uint8_t volume;
	uint8_t finetune;
	int vibrato;
	uint8_t vibrato_index;
	uint8_t vibrato_speed;
	uint8_t vibrato_depth;
	int period;
	uint8_t period_index;
	int porta_note;
	int porta_speed;
};

struct player_t {
	int current_tick;
	int current_pos;
	int current_row;
	int song_speed;
	int song_length;
	int pattern_delay;
	bool has_pattern_break;
	int skip_row;
	bool has_jump_pattern;
	int jump_to_pattern;
	struct track_t tracks[NUM_TRACKS];
};

struct player_t _player_music;
struct player_t _player_jingle;

static void do_porta(struct track_t *track) {
	if (track->porta_note != 0 && track->porta_speed != 0) {
		const int distance = track->period - track->porta_note;
		const int sign = distance > 0 ? 1 : -1;
		if ((sign * distance) > track->porta_speed) {
			track->period -= sign * track->porta_speed;
		} else {
			track->period = track->porta_note;
		}
	}
}

static void do_vibrato(struct track_t *track) {
	track->vibrato = (track->vibrato_depth * SIN_TABLE[track->vibrato_index]) / 128;
	track->vibrato_index += track->vibrato_speed;
	track->vibrato_index &= 63;
}

static void do_volume_slide(struct track_t *track, int ex, int ey) {
	int volume = track->volume;
	if (ex == 0) {
		volume -= ey;
	} else if (ey == 0) {
		volume += ex;
	}
	track->volume = (volume < 0) ? 0 : ((volume > 64) ? 64 : volume);
}

static void update_row(struct player_t *player, int tick) {
	for (int track = 0; track < NUM_TRACKS; ++track) {
		player->tracks[track].vibrato = 0;

		const int pattern = _mod_orders[player->current_pos];
		const struct note_t *note = &_mod_patterns[pattern].note[player->current_row][track];
		const uint8_t effect = note->effect;
		const int sample = note->sample;
		const int period = note->period;
		if (tick == 0) {
			if (sample != 0) {
				if (player->tracks[track].sample != sample) {
					player->tracks[track].vibrato_index = 0;
				}
				int finetune, volume;
				_paula->GetSampleInfo(sample, &finetune, &volume);
				player->tracks[track].finetune = finetune;
				player->tracks[track].volume = volume;
				player->tracks[track].sample = sample;
			}
			if (period != 0) {
				if (effect != 3 && effect != 5) {
					const int period_index = find_period_index(period, player->tracks[track].finetune);
					player->tracks[track].period_index = period_index;
					player->tracks[track].period = PERIODS[period_index];
				}
			}
		}
		const uint8_t exy = note->param & 255;
		const uint8_t ex  = (exy >> 4) & 15;
		const uint8_t ey  =  exy       & 15;
		switch (effect) {
		case 0x0:
			if (tick != 0 && exy != 0) {
				switch (tick & 3) {
				case 1:
					player->tracks[track].period = PERIODS[player->tracks[track].period_index + ex];
					break;
				case 2:
					player->tracks[track].period = PERIODS[player->tracks[track].period_index + ey];
					break;
				}
			}
			break;
		case 0x1:
			if (tick != 0) {
				player->tracks[track].period -= exy;
			}
			break;
		case 0x2:
			if (tick != 0) {
				player->tracks[track].period += exy;
			}
			break;
		case 0x3:
			if (tick == 0) {
				if (period) {
					player->tracks[track].porta_note = period;
				}
				if (exy) {
					player->tracks[track].porta_speed = exy;
				}
			} else {
				do_porta(&player->tracks[track]);
			}
			break;
		case 0x4:
			if (tick == 0) {
				if (exy != 0) {
					player->tracks[track].vibrato_speed = ex;
					player->tracks[track].vibrato_depth = ey;
				}
			}
			break;
		case 0x5:
			do_porta(&player->tracks[track]);
			do_volume_slide(&player->tracks[track], ex, ey);
			break;
		case 0x6:
			do_vibrato(&player->tracks[track]);
			do_volume_slide(&player->tracks[track], ex, ey);
			break;
		case 0x8:
			if (tick == 0) {
				if (player == &_player_jingle) {
					player->song_length = 0;
					for (int track = 0; track < NUM_TRACKS; ++track) {
						_paula->SetChannelSample(track, 0);
					}
					return;
				}
			}
			break;
		case 0x9:
			if (tick == 0) {
				if (exy != 0) {
					_paula->SetChannelOffset(track, exy * 256);
				}
			}
			break;
		case 0xA:
			if (tick != 0) {
				do_volume_slide(&player->tracks[track], ex, ey);
			}
			break;
		case 0xB:
			if (tick == 0) {
				player->has_jump_pattern = true;
				player->jump_to_pattern = exy;
			}
			break;
		case 0xC:
			if (tick == 0) {
				player->tracks[track].volume = exy;
			}
			break;
		case 0xD:
			if (tick == 0) {
				player->has_pattern_break = true;
				player->skip_row = ex * 10 + ey;
			}
			break;
		case 0xF:
			if (tick == 0) {
				assert(exy < 32);
				player->song_speed = exy;
			}
			break;
		default:
			fprintf(stdout, "update_row: unhandled effect:0x%x xy:0x%x x:%d y:%d tick:%d\n", effect, exy, ex, ey, tick);
			break;
		}
	}
}

void Music_UpdateTick() {
	struct player_t *player = &_player_jingle;
	if (player->song_length == 0) {
		player = &_player_music;
		if (player->song_length == 0) {
			return;
		}
	}

	if (player->current_tick == 0) {
		if (player->has_jump_pattern) {
			player->has_jump_pattern = false;
			player->current_pos = player->jump_to_pattern;
			player->current_row = 0;
		} else if (player->has_pattern_break) {
			player->has_pattern_break = false;
			player->current_row = player->skip_row;
			player->current_pos = (player->current_pos + 1) % player->song_length;
		}
		if (player->current_row >= 64) {
			player->current_row = 0;
			player->current_pos = (player->current_pos + 1) % player->song_length;
		}
	}
	update_row(player, player->current_tick);

	assert(player->song_speed != 0);
	player->current_tick = (player->current_tick + 1) % (player->song_speed + player->pattern_delay * player->song_speed);
	if (player->current_tick == 0) {
		++player->current_row;
		player->pattern_delay = 0;
	}

	for (int track = 0; track < NUM_TRACKS; ++track) {
		_paula->SetChannelVolume(track, player->tracks[track].volume);
		_paula->SetChannelPeriod(track, player->tracks[track].period + player->tracks[track].vibrato);
		if (player->tracks[track].sample != 0) {
			_paula->SetChannelSample(track, player->tracks[track].sample);
			player->tracks[track].sample = 0;
		}
	}
}

static void player_start(struct player_t *player) {
	for (int track = 0; track < NUM_TRACKS; ++track) {
		_paula->SetChannelSample(track, 0);
	}
	player->current_tick = 0;
	player->current_row = 0;
	player->song_speed = 6;
}

int Music_Load(const uint8_t *uds, int uds_size, struct Paula *paula) {
	memcpy(_dat, uds, uds_size);
	_dat_size = uds_size;
	load_music(_dat);
	convert_pattern_data();
	_paula = paula;
	paula->SetInterruptProc(Music_UpdateTick);
	return 0;
}

void Music_Play(int num) {
	player_start(&_player_music);
	_player_music.current_pos = num;
	_player_music.song_length = _mod_orders_count;
}

void Music_PlayJingle(int num) {
	player_start(&_player_jingle);
	_player_jingle.current_pos = num;
	_player_jingle.song_length = 1;
}
