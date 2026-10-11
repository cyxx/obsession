
#include "host.h"
#include "mixer.h"
#include "game.h"

extern void update_game_state();
extern void process_bonus();

struct vars_t g_vars;

static void clear_attract_lights() {
	light_set5(0x2a);
	light_set5(0x46);
	light_set5(0x2c);
	light_set5(0x48);
	light_set5(0x2e);
	light_set5(0x4a);
	light_set5(0x30);
	light_set5(0x4c);
	light_set5(0x32);
	light_set5(0x4e);
	light_set5(0x60);
	light_set5(0x62);
	light_set5(0x3e);
	light_set5(0x40);
	light_set5(0x42);
	light_set5(0x3c);
	light_set5(0x3a);
	light_set5(0x64);
	light_set5(0x52);
	light_set5(0x50);
	light_set5(0x00);
	light_set5(0x1C);
	light_set5(0x66);
	light_set5(0x1E);
	light_set5(0x20);
	light_set5(0x22);
	light_set5(0x24);
}

static void set_attract_lights() {
	light_set2(0x2a);
	light_set2(0x46);
	light_set2(0x2c);
	light_set2(0x48);
	light_set2(0x2e);
	light_set2(0x4a);
	light_set2(0x30);
	light_set2(0x4c);
	light_set2(0x32);
	light_set2(0x4e);
	light_set2(0x60);
	light_set2(0x62);
	light_set2(0x3e);
	light_set2(0x40);
	light_set2(0x42);
	light_set2(0x3c);
	light_set0(0x3a);
	light_set0(0x64);
	light_set0(0x52);
	light_set0(0x50);
	light_set0(0x00);
	light_set0(0x1c);
	light_set0(0x66);
	light_set0(0x1e);
	light_set0(0x20);
	light_set0(0x22);
	light_set0(0x24);
}

static void start_game() {
	memset(g_vars.bonus_activation_flags, 0, sizeof(g_vars.bonus_activation_flags));
	g_vars.deep_ramp_counter = 0;
	g_vars.pearl_double_bonus = 0;
	g_vars.pearl_letter_count = 0;
	g_vars.shoot_again_flag = 0;
	g_vars.skill_shot_score_index = 0;
	g_vars.score_bcd = 0;
	g_vars.bonus_score1_bcd = 0;
	g_vars.bonus_score2_bcd = 0;

	memset(g_vars.lights_state_table, 0xFF, sizeof(g_vars.lights_state_table));
	g_vars.bonus_multiplier = 1;
	g_vars.balls_count = BALLS_COUNT;
	g_vars.special_next = 1;
	g_vars.kickback_enabled = 1;
	g_vars.state.bonus_multiplier_hold = 0;
}

extern void clear_mission_state();

static void reset_game_state() {
	start_game();

	clear_mission_state();

	g_vars.last_key_code = -1;
}

static void reset_game() {
	g_host.Debug("reset_game");
	g_vars.current_player = 1;
	g_vars.is_screen_scrolling_following_ball = 1;
	g_vars.draw_ball_flag = 1;
	g_vars.f_keys_enabled_flag = 1;
	reset_game_state();
	g_vars.state_num = 0;
	g_vars.game_over_flag = 0;
}

static void exit_attract_mode() {
	g_vars.prev_vscroll = g_vars.vscroll;

	g_vars.led_panel_mode = 0;
	g_vars.led_panel_mode_state = 2;
	g_vars.score_bcd = 0;
	clear_attract_lights();

	g_vars.f_keys_disabled = 1;
	reset_game();

}

static void attract_mode_loop() {

	g_vars.tilt_table_right = 0;
	if (g_vars.keys[AMIGA_KEY_M]) {
		do_key_M_pressed();
	} else {
		do_key_M_released();
	}
	if (g_vars.keys[AMIGA_KEY_J]) {
		do_key_J_pressed();
	} else {
		do_key_J_released();
	}
	if (g_vars.keys[AMIGA_KEY_S]) {
		do_key_S_pressed();
	} else {
		do_key_S_released();
	}
	lights_draw();
	if (!g_vars.attract_mode_vscroll_down) {
		if (g_vars.vscroll != 0) {
			g_vars.vscroll -= 2;

			led_update();
		} else {
			g_vars.attract_mode_vscroll_down = true;
		}
	} else {
		if (g_vars.vscroll != 135 * 2) {
			g_vars.vscroll += 2;

			led_update();
		} else {
			g_vars.attract_mode_vscroll_down = false;
		}
	}

	if (g_vars.keys[AMIGA_KEY_DOWN] || g_vars.keys[AMIGA_KEY_SPACE]) {
		exit_attract_mode();
		g_vars.in_attract_mode = false;
	}
}

static const uint8_t _tilt_screen_table[] = { 1, 2, 3, 4, 4, 2, 1 };

void table_init() {
	g_vars.offscreen = (uint8_t *)malloc(TABLE_W * TABLE_H);
	memcpy(g_vars.offscreen, g_vars.res->table_pic, TABLE_W * TABLE_H);
	g_vars.cos = Resource_GetCosTable(g_vars.res);
	g_vars.vscroll_table = Resource_GetData(g_vars.res, RES_VSCROLL, 0);
	g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res) + 151 * 168;
	assert(g_vars.phys_mat_cur[0] == 0);
	assert(g_vars.phys_mat_cur[1] == 0x2C);
	assert(g_vars.phys_mat_cur[2] == 0x20);
	g_vars.angle_conversion_table = Resource_GetData(g_vars.res, RES_FRICTION, 0);
	assert(g_vars.angle_conversion_table[5] == 1);
	g_vars.normals_src_table = Resource_GetData(g_vars.res, RES_NORMALS, 0);
	assert(READ_BE_UINT16(g_vars.normals_src_table + 2 * sizeof(uint16_t)) == 0x39);
	g_vars.ball_collision_table = g_vars.res->ball_collision;
	assert(g_vars.ball_collision_table[0].behaviour_type == 1);
	assert(g_vars.ball_collision_table[0].param2 == -1);
	g_vars.random_num = 1;
	g_vars.in_attract_mode = true;

	g_vars.left_flipper_state = 1;
	g_vars.right_flipper_state = 1;
	memset(g_vars.lights_queue, 0xFF, sizeof(g_vars.lights_queue));
	memset(g_vars.lights_blinking_list, 0xFF, sizeof(g_vars.lights_blinking_list));
	g_vars.music_order_num = 61;
	g_vars.jingle_order_num = -1;

	init_physics_lut();

	g_vars.led_panel_mode = 1;
	g_vars.led_panel_mode_state = 2;
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

	if (g_vars.in_attract_mode) {

		led_reset_text();
		g_vars.led_color_blinking_speed = 0;
		g_vars.led_panel_text = Resource_GetData(g_vars.res, RES_TXT_COPYRIGHT, 0);
		g_vars.led_panel_text_loop_flag = 1;
		g_vars.led_panel_mode = 1;
		g_vars.led_panel_mode_state = 2;

		set_attract_lights();
	} else {
		exit_attract_mode();
	}
}

void table_update() {
	if (g_vars.game_over_flag) {
		exit_attract_mode();
		g_vars.game_over_flag = 0;
		return;
	}

	if (g_vars.in_attract_mode) {
		attract_mode_loop();
		return;
	}

	if (g_vars.esc_pressed_flag) {
		const int n0 = g_vars.last_key_code;
		if (!(n0 < 0)) {
			if (n0 == AMIGA_KEY_Y) {
				g_host.quit = true;
				return;
			} else if (n0 != AMIGA_KEY_ESCAPE) {
				g_vars.esc_pressed_flag = 0;
				g_vars.led_quit_flag = 0;
				g_vars.led_panel_mode = 0;
				g_vars.led_panel_mode_state = 2;
			}
		}
	}

	++g_vars.random_num;
	if (g_vars.game_over_tilt != 0 && g_vars.game_over_tilt > 1) {
		g_vars.jingle_order_num = 64;
		g_vars.state.special_mode = 5;
		if (g_vars.music_order_num != 46) {
			g_vars.music_order_num = 67;
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_TILT, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 100;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = 4;
		}
		g_vars.game_over_tilt = 2;
	}

	if (g_vars.keys[AMIGA_KEY_LSHIFT]) {
		do_key_ALT_LEFT_pressed();
	} else {
		do_key_ALT_LEFT_released();
	}
	if (g_vars.keys[AMIGA_KEY_RSHIFT]) {
		do_key_ALT_RIGHT_pressed();
	} else {
		do_key_ALT_RIGHT_released();
	}
	if (g_vars.keys[AMIGA_KEY_SPACE]) {
		do_key_SPACE_pressed();
	} else {
		g_vars.tilt_table_middle = 0;
	}
	if (g_vars.keys[AMIGA_KEY_ESCAPE]) {
		do_key_ESC_pressed();
	}
	if (g_vars.keys[AMIGA_KEY_DOWN]) {
		do_key_DOWN_pressed();
	} else {
		do_key_DOWN_released();
	}
	if (g_vars.keys[AMIGA_KEY_LAMIGA]) {
		do_key_AMIGA_LEFT_pressed();
	} else {
		g_vars.tilt_table_left = 0;
	}
	if (g_vars.keys[AMIGA_KEY_RAMIGA]) {
		do_key_AMIGA_RIGHT_pressed();
	} else {
		g_vars.tilt_table_right = 0;
	}

	if (g_vars.esc_pressed_flag == 0) {
		ball_update();
		if (g_vars.ball_xpos2 >= 306 && g_vars.ball_ypos2 >= 441 && g_vars.ball_ypos2 <= 450) {

			const int n0 = ABS(g_vars.ball_yvel);
			if (n0 < 20) {
				g_vars.ball_ypos2 = 442;
			}
		}
	}

	if (g_vars.left_flipper_pushed == 1) {
		update_left_flipper();
	}
	if (g_vars.right_flipper_pushed == 1) {
		update_right_flipper();
	}
	lights_draw();

	if (g_vars.is_screen_scrolling_following_ball) {
		int16_t n0, n1, n2;

		g_vars.vscroll = g_vars.prev_vscroll;
		n2 = READ_BE_UINT16(g_vars.vscroll_table + g_vars.vscroll * sizeof(int16_t));
		n0 = g_vars.vscroll + n2;
		n1 = g_vars.ball_ypos2 - n0;
		n2 = READ_BE_UINT16(g_vars.vscroll_table + 0x356 + n1 * sizeof(int16_t));
		n1 = g_vars.vscroll + n2;
		if (n1 >= 0) {
			if (n1 > 270) {
				n1 = 270;
			}
		} else {
			if (g_vars.ball_ypos2 < 484) {
				n1 = 0;
			} else {
				n1 = 270;
			}
		}

		g_vars.vscroll = n1;
		g_vars.prev_vscroll = g_vars.vscroll;
		if (g_vars.tilt_screen_index >= 0) {
			g_vars.vscroll += _tilt_screen_table[g_vars.tilt_screen_index];
			--g_vars.tilt_screen_index;
		}

	}

	if (g_vars.ball_out_of_play) {
		g_vars.state_num = 3;
		g_vars.state_counter = 0;
		g_vars.ball_out_of_play = 0;
	}
	update_game_state();
	if (g_vars.esc_pressed_flag == 0) {
		process_bonus();
	}

	led_update();

}

int table_get_state() {
	if (g_vars.led_quit_flag) {
		return GAME_STATE_CONFIRM_QUIT;
	}
	if (g_vars.in_attract_mode) {
		return GAME_STATE_ATTRACT_MODE;
	}
	if (g_vars.game_over_flag || g_vars.state_num >= 12) {
		return GAME_STATE_MATCH_OVER;
	}
	if (g_vars.game_over_tilt || g_vars.state_num >= 3) {
		return GAME_STATE_TILT_OR_DRAIN;
	}
	if (g_vars.ball_xpos2 >= 306) {
		return GAME_STATE_PLUNGER_READY;
	}
	return GAME_STATE_BALL_ACTIVE;
}

void sound_init() {
	uint32_t smp_size;
	const uint8_t *smp_data = Resource_GetData(g_vars.res, RES_SOUND, &smp_size);
	Mixer_SetSoundData(smp_data, smp_size);
	uint32_t uds_size;
	const uint8_t *uds_data = Resource_GetData(g_vars.res, RES_MUSIC, &uds_size);
	Mixer_SetMusicData(uds_data, uds_size);
}

void sound_update() {

	if (g_vars.sound_num != 0) {
		Mixer_PlaySample(g_vars.sound_num - 1, g_vars.sound_volume);
		g_vars.sound_num = 0;
	}
	if (g_vars.music_order_num != -1 || g_vars.jingle_order_num != -1) {
		Mixer_PlayJingle(g_vars.music_order_num, g_vars.jingle_order_num);
		g_vars.music_order_num = -1;
		g_vars.jingle_order_num = -1;
	}
}
