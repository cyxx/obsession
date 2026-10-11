#include "host.h"
#include "mixer.h"
#include "game.h"

static void do_tilt();

void do_key_M_pressed() {
	if (g_vars.led_panel_mode == 5) {
		return;
	}
	if (!g_vars.key_M_pressed) {
		g_vars.key_M_pressed = 1;
		Mixer_Toggle(MIXER_TOGGLE_MUSIC);
	}
}

void do_key_M_released() {
	g_vars.key_M_pressed = 0;
}

void do_key_J_pressed() {
	if (g_vars.led_panel_mode == 5) {
		return;
	}
	if (!g_vars.key_J_pressed) {
		g_vars.key_J_pressed = 1;
		Mixer_Toggle(MIXER_TOGGLE_JINGLES);
	}
}

void do_key_J_released() {
	g_vars.key_J_pressed = 0;
}

void do_key_S_pressed() {
	if (g_vars.led_panel_mode == 5) {
		return;
	}
	if (!g_vars.key_S_pressed) {
		g_vars.key_S_pressed = 1;
		Mixer_Toggle(MIXER_TOGGLE_SOUNDS);
	}
}

void do_key_S_released() {
	g_vars.key_S_pressed = 0;
}

void do_key_ALT_LEFT_pressed() {
	if (!g_vars.game_over_tilt && g_vars.left_flipper_state == 1) {
		g_vars.left_flipper_pushed = 1;
		g_vars.left_flipper_state = 0;
		g_vars.sound_num = 25;
		g_vars.sound_volume = 38;
		g_vars.left_flipper_on = 1;
	}
}

void do_key_ALT_LEFT_released() {
	if (g_vars.left_flipper_state == 0) {
		g_vars.left_flipper_pushed = 1;
		g_vars.left_flipper_state = 1;
	}
}

void do_key_AMIGA_LEFT_pressed() {
	if (g_vars.led_panel_mode != 5 && g_vars.tilt_table_left == 0) {
		g_vars.tilt_table_left = 1;
		g_vars.tilt_direction = 2;
		do_tilt();
	}
}

void do_key_ALT_RIGHT_pressed() {
	if (!g_vars.game_over_tilt && g_vars.right_flipper_state == 1) {
		g_vars.right_flipper_pushed = 1;
		g_vars.right_flipper_state = 0;
		g_vars.sound_num = 25;
		g_vars.sound_volume = 38;
		g_vars.right_flipper_on = 1;
	}
}

void do_key_ALT_RIGHT_released() {
	if (g_vars.right_flipper_state == 0) {
		g_vars.right_flipper_pushed = 1;
		g_vars.right_flipper_state = 1;
	}
}

void do_key_AMIGA_RIGHT_pressed() {
	if (g_vars.led_panel_mode != 5 && g_vars.tilt_table_right == 0) {
		g_vars.tilt_table_right = 1;
		g_vars.tilt_direction = 3;
		do_tilt();
	}
}

static void do_tilt() {
	if (g_vars.tilt_block_until_launch) {
		return;
	}
	if (g_vars.tilt_launch_cooldown) {
		return;
	}
	g_vars.tilt_screen_index = 6;
	if (g_vars.tilt_detection_counter == 0) {
		g_vars.tilt_detection_counter = 60;
	} else if (g_vars.tilt_confirmation_counter == 0) {
		g_vars.tilt_confirmation_counter = 60;
	} else {
		if (g_vars.draw_ball_flag && g_vars.ball_xpos2 < 306) {
			++g_vars.game_over_tilt;
		}
	}
}

void do_key_SPACE_pressed() {
	if (g_vars.led_panel_mode == 5) {
		return;
	}
	if (g_vars.tilt_table_middle) {
		return;
	}
	g_vars.tilt_table_middle = 1;
	g_vars.tilt_direction = 1;
	do_tilt();
}

void do_key_ESC_pressed() {
	g_vars.esc_pressed_flag = 1;
	g_vars.led_quit_flag = 1;
	const uint8_t *image = Resource_GetData(g_vars.res, RES_LED_QUIT, 0);
	memcpy(g_vars.led_screen, image, 42 * 15);
}

void do_key_DOWN_pressed() {
	if (g_vars.led_panel_mode == 5) {
		return;
	}
	if (g_vars.draw_ball_flag == 0) {
		if (g_vars.vscroll < 270) {
			g_vars.vscroll += 8;
			if (g_vars.vscroll >= 270) {
				g_vars.vscroll = 270;
			}

		}
	} else {
		if (g_vars.ball_xpos2 < 306 || g_vars.ball_ypos2 > 450) {
			return;
		}
		if (g_vars.spring_pull_initiated == 0) {
			g_vars.f_keys_disabled = 0;
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_PERCENTAGE, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 0;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = -1;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			g_vars.spring_pull_initiated = 1;
		}
		g_vars.spring_force += 10;
		if (g_vars.spring_force >= 511) {
			g_vars.spring_force = 511;
			if (g_vars.spring_super_charge_flag == 0) {
				g_vars.spring_super_charge_flag = 1;
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_SUPER_CHARGE, 0);
				g_vars.led_panel_animation_style_flag = 1;
				g_vars.led_change_color_flag = 1;
				g_vars.led_unused_flag = 0;
				g_vars.led_panel_counter = -1;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			}
		}
		g_vars.spring_down_flag = 1;
	}
}

void do_key_DOWN_released() {
	if (g_vars.led_panel_mode == 5) {
		return;
	}
	g_vars.spring_pull_initiated = 0;
	if (g_vars.spring_down_flag) {
		g_vars.spring_down_flag = 0;
		g_vars.spring_super_charge_flag = 0;
		g_vars.led_panel_counter = 0;
		g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_BLANK, 0);
		g_vars.led_panel_animation_style_flag = 1;
		g_vars.led_change_color_flag = 0;
		g_vars.led_unused_flag = 0;
		g_vars.led_panel_counter = 0;
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		if (g_vars.ball_ypos2 >= 439 && g_vars.ball_xpos2 >= 306) {
			g_vars.spring_plunger_released = 1;
			g_vars.sound_num = 28;
			g_vars.sound_volume = 64;
			const int spring = -g_vars.spring_force;
			int abs_yvel = g_vars.ball_yvel;
			if (abs_yvel > 0) {
				abs_yvel = -abs_yvel;
			}
			if (abs_yvel >= spring) {
				g_host.Debug("Start game ball with yvel %d spring %d", abs_yvel, spring);
				g_vars.ball_yvel = spring;
			}
		}
	}
	g_vars.spring_force = 0;
}

