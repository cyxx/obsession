#include "host.h"
#include "game.h"

static void patch_phys_map(uint8_t val) {
	uint8_t *p = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET + 0x4ab0;
	static const int16_t T[] = { 0, 0xaa, 0x154, 0x1fe, 0x2a8, 0x352, 0x3fc, 0x4a6 };
	for (int i = 0; i < sizeof(T) / sizeof(T[0]) - 1; ++i) {
		memset(p + T[i], val, 4);
	}
	memset(p + T[sizeof(T) / sizeof(T[0]) - 1], val, 2);
}

void patch_phys_map_open() {
	patch_phys_map(0x00);
}

void patch_phys_map_close() {
	patch_phys_map(0xb6);
}

void update_physmap(int frame, const uint8_t *src, uint8_t *dst) {
	assert((frame & 1) == 0);
	frame /= sizeof(int16_t);
	assert(frame >= 0 && frame < 6);
	static const uint16_t offsets[] = { 0, 0x268, 0x4D0, 0x738, 0x738, 0x738 };
	src += offsets[frame];
	for (int i = 0; i < 22; ++i, dst += 0xA8) {
		memcpy(dst, src, 7 * 4);
		src += 7 * 4;
	}
}

int16_t clip_s16(int16_t x) {
	return x;
}

void reset_vars() {
	g_vars.ball_out_of_play = 1;
	g_vars.reset_tilt = 1;
	g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_GOING_DOWN, 0);
	g_vars.led_panel_animation_style_flag = 1;
	g_vars.led_change_color_flag = 0;
	g_vars.led_unused_flag = 0;
	g_vars.led_panel_counter = -1;
	g_vars.led_panel_mode_state = 2;
	g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
	g_vars.ball_xpos1 = 314;
	g_vars.ball_ypos1 = 462;
	g_vars.ball_xpos2 = 314;
	g_vars.ball_ypos2 = 462;
	g_vars.ball_xpos3 = 314;
	g_vars.ball_ypos3 = 462;
	g_vars.ball_xpos4 = 314;
	g_vars.ball_ypos4 = 462;
	g_vars.ball_xvel = 0;
	g_vars.ball_yvel = 0;
	g_vars.ball_xvel_prev = 0;
	g_vars.ball_yvel_prev = 0;
	g_vars.ball_xvel_acc = 0;
	g_vars.ball_yvel_acc = 0;
}

const int16_t _collision_response_angle_table[] = {
	  0,  22,  44,  66,  90, 112, 134, 156, 180, 202, 224, 246, 270, 292, 314, 336,
	360, 382, 404, 426, 450, 472, 494, 516, 540, 562, 584, 606, 630, 652, 674, 696
};

void init_physics_lut() {
	{
		const uint8_t *src = Resource_GetData(g_vars.res, RES_LFLIPPER_PHYSMAP, 0);
		uint8_t *dst = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET + 0x8e94;
		for (int j = 0; j < 22; ++j) {
			for (int i = 0; i < 7; ++i) {
				memcpy(dst, src, 4);
				dst += 4;
				src += 4;
			}
			dst += 140;
		}
	}
	{
		const uint8_t *src = Resource_GetData(g_vars.res, RES_RFLIPPER_PHYSMAP, 0);
		uint8_t *dst = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET + 0x8ebc;
		for (int j = 0; j < 22; ++j) {
			for (int i = 0; i < 7; ++i) {
				memcpy(dst, src, 4);
				dst += 4;
				src += 4;
			}
			dst += 140;
		}
	}
	{
		int16_t *dst = g_vars.collision_response_angle_table;
		int n0 = 10;
		int n1 = 31;
		do {
			int n2 = n0 / 100;
			n2 += n2;
			if (n2 >= 720) {
				n2 -= 720;
			}
			assert(n2 == _collision_response_angle_table[dst - g_vars.collision_response_angle_table]);
			*dst++ = n2;
			n0 += 1125;
		} while (--n1 != -1);
	}
	{
		const uint8_t *src = g_vars.normals_src_table;
		int16_t *dst = g_vars.normals_dst_table;
		for (int i = 0; i < 450; ++i) {
			*dst++ = READ_BE_UINT16(src);
			src += 20;
		}
		assert(g_vars.normals_dst_table[1] == 0x11d);
		assert(g_vars.normals_dst_table[4] == 0x476);
	}
	{
		uint8_t *src = g_vars.bounce_energy_lookup_table;
		*src++ = 0;
		int n0 = 1;
		int n2 = 1;
		do {
			int n1 = n2;
			do {
				*src++ = n0;
			} while (--n1 != -1);
			n2 += 2;
			++n0;
		} while (n0 != 93);
	}
	for (int i = 0; i < 90; ++i) {
		g_vars.cos[270 - i - 1] = -g_vars.cos[90 + i];
	}
	for (int i = 0; i < 90; ++i) {
		g_vars.cos[i] += 4;
	}
	for (int i = 0; i < 90; ++i) {
		g_vars.cos[270 + i] -= 4;
	}
	assert((uint16_t)g_vars.cos[0] == 0xB8);
	assert((uint16_t)g_vars.cos[8] == 0xA8);
	assert((uint16_t)g_vars.cos[359] == 0xFF4A);
}
