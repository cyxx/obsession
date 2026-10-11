
#include "host.h"
#include "render.h"
#include "resource.h"
#include "game.h"

static void DrawGfx(int num, ResourcesData *rd, uint8_t *p) {
	const struct gfx_t *gfx = Resource_GetGfx(rd, num);
	const uint8_t *src = Resource_GetData(rd, RES_TABLE_GFX, 0) + gfx->src;
	const int pitch = 10040;

	const int dst_y = gfx->dst / 42;
	const int dst_x = (gfx->dst % 42) * 8;
	int src_offset = 0;

	p += dst_y * TABLE_W;

	for (int y = 0; y < gfx->h; ++y) {
		for (int x = 0; x < gfx->w; ++x) {
			uint16_t m[4];
			for (int d = 0; d < 4; ++d) {
				m[d] = READ_BE_UINT16(src + pitch * d + src_offset);
			}
			src_offset += 2;
			for (int b = 0; b < 16; ++b) {
				const int mask = 1 << (15 - b);
				uint8_t color = 0;
				for (int d = 0; d < 4; ++d) {
					if ((m[d] & mask) != 0) {
						color |= (1 << d);
					}
				}
				if (color != 0) {
					p[dst_x + x * 16 + b] = color;
				}
			}
		}
		src_offset += 40 - gfx->w * 2;
		p += TABLE_W;
	}
}

static uint16_t _palette[COLORS_COUNT];

struct flipper_gfx_t {
	uint16_t src;
	uint32_t dst;
};

static const struct flipper_gfx_t _left_flipper_draw_frame_state0_data[] = {
	{ 352, 18658 },
	{ 352, 18658 },
	{ 360, 18658 },
	{ 368, 18658 },
	{  24, 18196 },
	{  24, 18196 },
};

static const struct flipper_gfx_t _left_flipper_draw_frame_state1_data[] = {
	{ 352, 18658 },
	{ 352, 18658 },
	{   8, 18196 },
	{  16, 18196 },
	{  24, 18196 },
	{  24, 18196 },
};

static const struct flipper_gfx_t _right_flipper_draw_frame_state0_data[] = {
	{ 352, 18668 },
	{ 352, 18668 },
	{ 360, 18668 },
	{ 368, 18668 },
	{  24, 18206 },
	{  24, 18206 },
};

static const struct flipper_gfx_t _right_flipper_draw_frame_state1_data[] = {
	{ 352, 18668 },
	{ 352, 18668 },
	{   8, 18206 },
	{  16, 18206 },
	{  24, 18206 },
	{  24, 18206 },
};

static void DrawFlipper(const uint8_t *gfx, uint32_t pos, int surface) {

	const int gfx_pitch = FLIPPER_H * 32;

	const int dst_y = pos / 42;
	const int dst_x = (pos % 42) * 8;

	const int W = 64;
	const int H = 34;

	int pitch;
	uint16_t *p = 0;
	g_host.LockSurface565(surface, &p, &pitch);

	p += dst_y * pitch;

	for (int y = 0; y < H; ++y) {
		const int palette_offset = ((dst_y + y) / 50) * 32;
		for (int x = 0; x < W; x += 8) {
			uint8_t m[5];
			for (int d = 0; d < 5; ++d) {
				m[d] = gfx[d * gfx_pitch + (x >> 3)];
			}
			for (int b = 0; b < 8; ++b) {
				const int mask = 1 << (7 - b);
				uint8_t color = 0;
				for (int d = 0; d < 5; ++d) {
					if ((m[d] & mask) != 0) {
						color |= (1 << d);
					}
				}
				if (color != 0) {
					p[dst_x + x + b] = _palette[palette_offset + color];
				}
			}
		}
		gfx += FLIPPER_W / 8;
		p += pitch;
	}

	g_host.UnlockSurface565(surface);
}

static void DrawFlippers(ResourcesData *rd, int s) {
	const struct flipper_gfx_t *lgfx;
	if (g_vars.left_flipper_state == 0) {
		lgfx = &_left_flipper_draw_frame_state0_data[g_vars.left_flipper_frame_num];
	} else {
		lgfx = &_left_flipper_draw_frame_state1_data[g_vars.left_flipper_frame_num];
	}

	const uint8_t *lflipper_gfx = Resource_LeftFlipperGfx(rd) + lgfx->src;
	DrawFlipper(lflipper_gfx, lgfx->dst, s);

	const struct flipper_gfx_t *rgfx;
	if (g_vars.right_flipper_state == 0) {
		rgfx = &_right_flipper_draw_frame_state0_data[g_vars.right_flipper_frame_num];
	} else {
		rgfx = &_right_flipper_draw_frame_state1_data[g_vars.right_flipper_frame_num];
	}

	const uint8_t *rflipper_gfx = Resource_RightFlipperGfx(rd) + rgfx->src;
	DrawFlipper(rflipper_gfx, rgfx->dst, s);
}

static void DrawBall(ResourcesData *rd, int surface, int x_pos, int y_pos) {
	const uint8_t *tbl = Resource_GetData(rd, RES_TABLE_MSK, 0) + ((y_pos * TABLE_W) >> 3);
	const uint8_t *src = rd->ball_gfx;
	const uint8_t *msk = rd->ball_msk;

	int pitch;
	uint16_t *p = 0;
	g_host.LockSurface565(surface, &p, &pitch);

	p += y_pos * pitch;

	for (int y = 0; y < BALL_H; ++y) {
		if (y_pos + y >= 0 && y_pos + y < TABLE_H) {
			int x2 = x_pos;
			for (int x = 0; x < BALL_W; ++x, ++x2) {
				if (!(x_pos + x >= 0 && x_pos + x < TABLE_W)) {
					continue;
				}
				int mask = 1 << (7 - (x & 7));
				if ((msk[x / 8] & mask) == 0) {
					continue;
				}
				if (!(g_vars.ball_layer == LAYER_OVERHEAD || g_vars.draw_ball_masked_flag)) {
					mask = 1 << (7 - (x2 & 7));
					if ((tbl[x2 / 8] & mask) != 0) {
						continue;
					}
				}
				const uint8_t color = src[x];
				p[x_pos + x] = _palette[color];
			}
		}
		src += BALL_W;
		msk += BALL_W / 8;
		tbl += TABLE_W >> 3;
		p += pitch;
	}

	g_host.UnlockSurface565(surface);
}

static void DrawTable(ResourcesData *rd, int surface, const uint8_t *src) {
	int pitch;
	uint16_t *p = 0;
	g_host.LockSurface565(surface, &p, &pitch);
	for (int y = 0; y < TABLE_H; ++y) {
		const int palette_offset = (y / 50) * 32;
		for (int x = 0; x < TABLE_W; x += 2) {
			const uint16_t color1 = _palette[palette_offset + *src++];
			const uint16_t color2 = _palette[palette_offset + *src++];
			*(uint32_t *)(p + x) = color1 | (color2 << 16);
		}
		p += pitch;
	}
	g_host.UnlockSurface565(surface);
}

static void DrawLine(uint16_t *p, int w, uint16_t color) {
	for (int x = 0; x < w; x += 2) {
		*(uint32_t *)(p + x) = color | (color << 16);
	}
}

static bool _init_led = 0;

static void DrawLED(ResourcesData *rd, int surface) {
	uint32_t color;

	int pitch;
	uint16_t *p = 0;
	g_host.LockSurface565(surface, &p, &pitch);

	p += pitch;

	if (!_init_led) {
		color = g_host.MapRGB565(surface, 0x888);
		DrawLine(p, LED_W, color);
	}
	p += pitch;

	const uint8_t *src = g_vars.led_screen;
	color = g_host.MapRGB565(surface, g_vars.led_color_rgb444);

	p += pitch;
	for (int y = 0; y < 15; ++y, p += pitch * 2) {
		for (int x = 0; x < LED_W; x += 8, ++src) {
			for (int b = 0; b < 8; ++b) {
				const int mask = 1 << (7 - b);
				p[x + b] = (*src & mask) != 0 ? color : 0;
			}
		}
	}

	if (!_init_led) {
		color = g_host.MapRGB565(surface, 0x888);
		DrawLine(p, LED_W, color);
	}

	g_host.UnlockSurface565(surface);

	_init_led = 1;
}

void InitPalette(const ResourcesData *rd, int surface) {
	for (int i = 0; i < COLORS_COUNT; ++i) {
		const uint16_t rgb444 = rd->colors[i];
		_palette[i] = g_host.MapRGB565(surface, rgb444);
	}
}

static int _Lights_Draw = 1;

void Draw(ResourcesData *data) {
	uint8_t *p = (_Lights_Draw & 1) ? g_vars.offscreen : data->table_pic;
	for (int i = 0; i < data->gfx_ofs_count; ++i) {
		if (g_vars.gfx_lights[i]) {
			DrawGfx(i, data, p);
		}
	}
	_Lights_Draw ^= 1;
	DrawTable(data, DRAW_SURFACE_TABLE, p);
	DrawFlippers(data, DRAW_SURFACE_TABLE);
	DrawBall(data, DRAW_SURFACE_TABLE, g_vars.ball_xpos2, g_vars.ball_ypos2);
	DrawLED(data, DRAW_SURFACE_LED);
}
