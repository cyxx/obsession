
#include "host.h"
#include "game.h"

static const uint16_t LED_COLORS[] = {
	0x220, 0x440, 0x660, 0x880, 0xAA0, 0xCC0, 0xFF0
};

static void decode_led_bitmap(uint8_t *dst, int x) {
	const uint8_t *p = g_vars.led_animation_current_ptr;
	while (1) {
		const uint8_t control = *p++;
		if (control == 0) {
			break;
		}
		if ((control & 0x80) == 0) {
			x += control;
			continue;
		}
		x += control & 0x7f;
		while (1) {
			const uint8_t pixel = *p++;
			dst[x] = pixel & 0x7f;
			++x;
			if ((pixel & 0x80) != 0) {
				break;
			}
		}
	}
	g_vars.led_animation_current_ptr = p;
}

static void led_clear_bitmap() {
	memset(g_vars.led_temp_bitmap, 0, LED_PITCH * 15);
}

static void led_copy_bitmap() {
	memcpy(g_vars.led_screen, g_vars.led_temp_bitmap, LED_PITCH * 15);
}

static void draw_led_digit(int offset, int digit) {
	const uint8_t *fnt = Resource_GetData(g_vars.res, RES_FONT_16_5, 0);
	offset -= 1722;
	const int x = offset % LED_PITCH;
	const int y = offset / 82;
	uint8_t *dst = g_vars.led_screen + y * TABLE_W / 8 + x;
	for (int y = 0; y < 5; ++y) {
		memcpy(dst + LED_PITCH * y, fnt + y * 20 + digit * 2, 2);
	}
}

static void draw_led_current_player_ball() {
	int16_t value;

	if (g_vars.balls_count != 0) {
		value = g_vars.current_player;
	} else {

		value = 1;
	}
	draw_led_digit(1820, value);
	if (g_vars.balls_count != 0) {

		value = BALLS_COUNT - g_vars.balls_count + 1;
		if (value > 99) {
			draw_led_digit(2488, value / 100);
			value %= 100;
		}
		if (value > 9) {
			draw_led_digit(2490, value / 10);
			value %= 10;
		}
		draw_led_digit(2492, value);
	}
}

static void led_zero(int size, int offset) {
	assert(offset >= 1722);
	offset -= 1722;
	const int x = offset % LED_PITCH;
	const int y = offset / 82;
	uint8_t *dst = g_vars.led_screen + y * TABLE_W / 8 + x;
	memset(dst, 0, size);
}

static void led_or(const uint8_t *src, int size, int offset) {
	assert(offset >= 1722);
	offset -= 1722;
	const int x = offset % LED_PITCH;
	const int y = offset / 82;
	uint8_t *dst = g_vars.led_screen + y * TABLE_W / 8 + x;
	for (int i = 0; i < size; ++i) {
		dst[i] |= src[i];
	}
}

static void draw_led_score_digit(const uint8_t *src, int offset) {
	assert(offset >= 1722);
	offset -= 1722;
	const int x = offset % LED_PITCH;
	const int y = offset / 82;
	uint8_t *dst = g_vars.led_screen + y * TABLE_W / 8 + x;
	for (int y = 0; y < 13; ++y) {
		memcpy(dst + y * 84 / 2, src, 4); src += 4;
	}
}

static void draw_unaligned_led_score_digit(const uint8_t *src, int offset) {
	assert(offset >= 1722);
	offset -= 1722;
	const int x = offset % LED_PITCH;
	const int y = offset / 82;
	uint8_t *dst = g_vars.led_screen + y * TABLE_W / 8 + x;
	for (int y = 0; y < 13; ++y) {
		uint8_t *q = dst + y * 84 / 2;
		*q++ |= *src++;
		*q++ |= *src++;
		*q++ = *src++;
		*q++ = *src++;
	}
}

static void draw_led_glyph(const uint8_t *src, int offset) {
	assert(offset >= 1722);
	offset -= 1722;
	const int x = offset % LED_PITCH;
	const int y = offset / 82;
	uint8_t *dst = g_vars.led_screen + y * TABLE_W / 8 + x;
	for (int y = 0; y < 13; ++y) {
		if (y < 2) {
			uint8_t *q = dst + y * 84 / 2;
			*q++ |= *src++;
			*q++ |= *src++;
			*q++ = *src++;
			*q++ = *src++;
		} else {
			memcpy(dst + y * 84 / 2, src, 4); src += 4;
		}
	}
}

static void draw_led_score(uint32_t score) {
	int16_t digit, drawn;

	uint8_t bcd[8];
	for (int i = 0; i < 5; ++i) {
		const int lo = score % 10; score /= 10;
		const int hi = score % 10; score /= 10;
		bcd[4 - i] = (hi << 4) | lo;
	}

	const uint8_t *font = Resource_GetData(g_vars.res, RES_FONT_LED_8_13, 0);
	const uint8_t *p = bcd;
	int16_t offset;
	const uint8_t *glyph;

	drawn = 0;
	digit = p[0] & 0xf0;

	if (digit != 0) {
		glyph = font + (digit >> 2) * 13;

		offset = 1822;
		draw_led_score_digit(glyph, offset);

		glyph = font + 0x208;
		led_or(glyph, 4, offset + 2);
		led_or(glyph + 4, 4, offset + 86);
	}

	digit = p[0] & 0xf;

	if (digit != 0 || drawn != 0) {
		drawn = -1;
		glyph = font + 0x23c + digit * 52;

		offset = 1826;
		draw_led_score_digit(glyph, offset);
	}

	digit = p[1] & 0xf0;

	if (digit != 0 || drawn != 0) {
		drawn = -1;
		glyph = font + 0x23c + (digit >> 2) * 13;

		offset = 1828;
		draw_led_score_digit(glyph, offset);
	}

	offset = 1832;
	led_zero(2, offset);
	led_zero(2, offset + 84);
	digit = p[1] & 0xf;

	if (digit != 0 || drawn != 0) {
		drawn = -1;
		glyph = font + 0x23c + digit * 52;

		offset = 1830;
		draw_led_score_digit(glyph, offset);

		glyph = font + 0x444;
		led_or(glyph, 4, offset);
		led_or(glyph + 4, 4, offset + 84);
	}

	digit = p[2] & 0xf0;

	if (digit != 0 || drawn != 0) {
		drawn = -1;
		glyph = font + 0x478 + (digit >> 2) * 13;

		offset = 1832;
		draw_led_glyph(glyph, offset);
	}

	digit = p[2] & 0xf;

	if (digit != 0 || drawn != 0) {
		glyph = font + 0x478 + digit * 52;

		offset = 1834;
		if (drawn != 0) {
			draw_unaligned_led_score_digit(glyph, offset);
		} else {
			draw_led_score_digit(glyph, offset);
			drawn = -1;
		}
	}

	digit = p[3] & 0xf0;

	if (digit != 0 || drawn != 0) {
		glyph = font + 0x478 + (digit >> 2) * 13;

		offset = 1836;
		if (drawn != 0) {
			draw_unaligned_led_score_digit(glyph, offset);
		} else {
			draw_led_score_digit(glyph, offset);
			drawn = -1;
		}

		glyph = font + 0x680;
		offset += 2;
		led_or(glyph, 4, offset);
		led_or(glyph + 4, 2, offset + 84);
	}
	digit = p[3] & 0xf;

	if (digit != 0 || drawn != 0) {
		glyph = font + 0x6b4 + digit * 52;

		offset = 1838;
		if (drawn != 0) {
			draw_unaligned_led_score_digit(glyph, offset);
		} else {
			draw_led_score_digit(glyph, offset);
			drawn = -1;
		}
	}

	digit = p[4] & 0xf0;

	if (digit != 0 || drawn != 0) {
		glyph = font + 0x6b4 + (digit >> 2) * 13;

		offset = 1840;
		if (drawn != 0) {
			draw_unaligned_led_score_digit(glyph, offset);
		} else {
			draw_led_score_digit(glyph, offset);
			drawn = -1;
		}
	}

	digit = p[4] & 0xf;

	glyph = font + 0x6b4 + digit * 52;

	offset = 1842;
	draw_unaligned_led_score_digit(glyph, offset);
}

static void shift_led_text();
static void render_led_text();

void led_update() {
	int16_t color;

	g_vars.led_color_rgb444 = 0xFF0;

	if (g_vars.led_quit_flag) {
		return;
	}

	if (g_vars.led_panel_mode == 0) {

		if (g_vars.led_panel_mode_state == 2) {
			const uint8_t *image;
			if (g_vars.balls_count == 0) {
				image = Resource_GetData(g_vars.res, RES_LED_PLAYER_GAME_OVER, 0);
			} else {
				image = Resource_GetData(g_vars.res, RES_LED_PLAYER_BALL, 0);
			}
			memcpy(g_vars.led_screen, image, LED_PITCH * 15);
			draw_led_current_player_ball();
			g_vars.led_panel_mode_state = 1;
			return;

		} else if (g_vars.led_panel_mode_state == 1) {
			if (g_vars.led_color_index == 0) {
				draw_led_score(g_vars.score_bcd);
				return;
			}
			color = g_vars.led_color_index;
			if (color < 0) {
				color = 0;
			}
			if (color == 0 || color > 7) {
				g_host.Warn("led_color_index:%d", g_vars.led_color_index);
				return;
			}
			g_vars.led_color_rgb444 = LED_COLORS[color - 1];
			color += 1;
			if (color >= 8) {
				color = 0;
			}
			g_vars.led_color_index = color;
			draw_led_score(g_vars.score_bcd);
			return;

		} else if (g_vars.led_panel_mode_state == 0) {
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = 0;
		}
		return;

	} else if (g_vars.led_panel_mode == 1) {

		if (g_vars.led_panel_mode_state == 2) {
			led_reset_text();
			g_vars.led_panel_mode_state = 1;
			memset(g_vars.led_screen, 0, LED_PITCH * 15);
		} else if (g_vars.led_panel_mode_state == 1) {
			shift_led_text();
			render_led_text();
		} else if (g_vars.led_panel_mode_state == 0) {
			g_vars.led_panel_mode = 0;
			g_vars.led_panel_mode_state = 2;
		}
		return;

	} else if (g_vars.led_panel_mode == 2) {

		if (g_vars.led_panel_mode_state == 2) {

			const uint8_t *image = Resource_GetData(g_vars.res, RES_LED_BONUS, 0);
			memcpy(g_vars.led_screen, image, LED_PITCH * 15);
			g_vars.led_panel_mode_state = 1;

		} else if (g_vars.led_panel_mode_state == 1) {

			if (g_vars.led_color_index == 0) {
				draw_led_score(g_vars.bonus_score2_bcd);
				return;
			}

			color = g_vars.led_color_index;
			if (color < 0) {
				color = 0;
			}
			if (color == 0 || color > 7) {
				g_host.Warn("led_color_index:%d", g_vars.led_color_index);
				return;
			}
			g_vars.led_color_rgb444 = LED_COLORS[color - 1];
			color += 1;
			if (color >= 8) {
				color = 0;
			}
			g_vars.led_color_index = color;
			draw_led_score(g_vars.bonus_score2_bcd);
			return;

		} else if (g_vars.led_panel_mode_state == 0) {

			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = 0;
		}
		return;

	} else if (g_vars.led_panel_mode == 3) {
		if (g_vars.led_panel_mode_state == 2) {
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = 0;
			memset(g_vars.led_screen, 0, LED_PITCH * 15);
		}
		return;

	} else if (g_vars.led_panel_mode == LED_MODE_4_PLAYING_ANIMATION) {
		if (g_vars.led_panel_mode_state == 2) {
			g_vars.led_animation_counter = 0;
			g_vars.led_animation_size = READ_BE_UINT16(g_vars.led_animation_ptr);

			g_vars.led_animation_current_ptr = g_vars.led_animation_ptr + 2;
			g_vars.led_panel_mode_state = 1;
			led_clear_bitmap();

			return;
		}

		if (g_vars.led_panel_mode_state == 1) {
			if (g_vars.led_animation_counter == 0) {
				led_copy_bitmap();
			}
			const int step = g_vars.led_animation_counter;
			++g_vars.led_animation_counter;
			switch (step) {
			case 0:
				decode_led_bitmap(g_vars.led_temp_bitmap, 0);
				break;
			case 1:
				decode_led_bitmap(g_vars.led_temp_bitmap, 0x7e);
				break;
			case 2:
				decode_led_bitmap(g_vars.led_temp_bitmap, 0xfc);
				break;
			case 3:
				decode_led_bitmap(g_vars.led_temp_bitmap, 0x17a);
				break;
			case 4:
				decode_led_bitmap(g_vars.led_temp_bitmap, 0x1f8);
				g_vars.led_animation_counter = 0;
				led_copy_bitmap();
				break;
			}

			--g_vars.led_animation_size;
			if (g_vars.led_animation_size == 0) {
				g_vars.led_panel_mode_state = 0;
			}
			return;
		}

		if (g_vars.led_panel_animation_style_flag != 0 && g_vars.led_panel_counter != 0) {
			if (g_vars.led_panel_counter > 0) {
				--g_vars.led_panel_counter;
			}
			if (g_vars.led_change_color_flag) {
				int speed = g_vars.led_color_blinking_speed;
				assert(speed >= 0 && speed < 8);
				if ((g_vars.led_blink_counter & (1 << speed)) != 0) {
					g_vars.led_color_rgb444 = 0x333;
				} else {
					g_vars.led_color_rgb444 = 0xFF0;
				}
			}
			++g_vars.led_blink_counter;
			assert(g_vars.led_unused_flag == 0);

			return;
		}

		g_vars.led_panel_mode = 0;
		g_vars.led_panel_mode_state = 2;
		led_clear_bitmap();
		return;

	} else if (g_vars.led_panel_mode == 5) {

		if (g_vars.led_panel_mode_state == 2) {

			g_vars.led_panel_mode_state = 1;
		} else if (g_vars.led_panel_mode_state == 1) {

		} else if (g_vars.led_panel_mode_state == 0) {

			g_vars.led_panel_mode = 0;
			g_vars.led_panel_mode_state = 2;
		}
	}
}

void led_reset_text() {
	g_vars.led_panel_text_ptr = g_vars.led_panel_text;
	g_vars.led_panel_text_index = 0;
	g_vars.led_panel_text_speed = 0;
}

static void shift_led_text() {
	uint8_t *row = g_vars.led_screen;
	for (int y = 0; y < 15; ++y) {
		uint8_t lo_mask = 0;
		uint8_t *src = row + 41;
		for (int x = 0; x < LED_PITCH; ++x) {
			const uint8_t mask = (*src << 2) | lo_mask;
			lo_mask = *src >> 6;
			*src = mask;
			--src;
		}
		row += LED_PITCH;
	}
}

static void render_led_text() {
	int16_t ch, shift, index;
	uint8_t *dst;
	const uint8_t *data, *speed_table;

	if (g_vars.led_panel_text_speed != 0) {
		--g_vars.led_panel_text_speed;
		shift = (int16_t)g_vars.led_panel_text_counter;
		--g_vars.led_panel_text_counter;
		data = g_vars.led_panel_text_letter_mask;

		dst = g_vars.led_screen + LED_PITCH - sizeof(uint16_t);
		for (int i = 0; i < 15; ++i) {
			const uint16_t bits = (READ_BE_UINT16(data) >> shift) & 3;

			dst[0] |= bits >> 8;
			dst[1] |= bits & 255;
			data += 40;
			dst += 84 / sizeof(uint16_t);
		}
	} else {
led_animation_end:
		++g_vars.led_panel_text_index;
		index = g_vars.led_panel_text_index;
		ch = 0;
		data = Resource_GetData(g_vars.res, RES_TABLE_LED_TEXT, 0);
		shift = g_vars.led_panel_text_ptr[index];
		if (shift == 0) {
			if (g_vars.led_panel_text_loop_flag) {
				g_vars.led_panel_text_ptr = g_vars.led_panel_text;
				g_vars.led_panel_text_index = 0;
				goto led_animation_end;
			} else {
				g_vars.led_panel_mode = 0;
				g_vars.led_panel_mode_state = 2;
			}
		} else {
			ch = shift & 0xFF;
			speed_table = Resource_GetData(g_vars.res, RES_TABLE_LED_TEXT, 0) + 122;
			g_vars.led_panel_text_speed = speed_table[ch - 32];
			ch = (int8_t)data[ch - 32];
			shift = ch / 20;
			ch %= 20;
			shift *= 640;
			ch *= 2;
			ch += shift;
			g_vars.led_panel_text_letter_mask = Resource_GetData(g_vars.res, RES_FONT_LED_SCROLL_TEXT, 0) + ch;
			g_vars.led_panel_text_counter = 15;
		}
	}
}
