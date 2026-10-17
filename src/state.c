#include "host.h"
#include "game.h"

static void load_player_bonus_state(int player) {

}

static void state_next_ball_delay() {
	++g_vars.state_counter;
	if (g_vars.state_counter >= 50) {
		g_vars.state_counter = 0;
		load_player_bonus_state(g_vars.current_player);
		lights_update_state();
		g_vars.state_num = 1;
	}
}

static void prepare_ball_for_launch() {
	g_vars.led_panel_mode = 0;
	g_vars.led_panel_mode_state = 2;

	if (g_vars.shoot_again_flag) {
		g_vars.shoot_again_flag = 0;
		g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_SHOOT_AGAIN, 0);
		g_vars.led_panel_animation_style_flag = 1;
		g_vars.led_change_color_flag = 1;
		g_vars.led_unused_flag = 0;
		g_vars.led_panel_counter = 100;
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
	}
	g_vars.is_screen_scrolling_following_ball = 1;
	g_vars.draw_ball_flag = 1;
	g_vars.is_ball_pushed_on_spring = 1;
	g_vars.ball_out_of_play = 0;
	g_vars.sound_num = 0x1B;
	g_vars.sound_volume = 64;
}

static void state_prepare_ball() {
	prepare_ball_for_launch();
	g_vars.state_num = 2;
}

static void ball_drained_set_music() {
	if (g_vars.game_over_tilt == 0) {
		g_vars.jingle_order_num = 33;
	}
	g_vars.music_order_num = 67;
	int flag = 0;
	if (g_vars.bonus_activation_flags[1] != 0 && g_vars.game_over_tilt == 0) {
		flag = 1;
	} else if (g_vars.bonus_activation_flags[8] != 0 && g_vars.bonus_activation_flags[3] != 0) {
		flag = 2;
	}
	g_vars.shoot_again_flag = flag;
	g_vars.bonus_activation_flags[8] = 0;
	g_vars.draw_ball_flag = 0;
}

static void ball_drained_reset_lights() {
	if (g_vars.bonus_multiplier_hold == 0) {
		reset_lights_multipliers();
	}
	reset_lights_starfish();
	reset_lights_arrows();
	reset_lights_dive();
}

static void ball_drained_award_bonus() {
	if (g_vars.shoot_again_flag != 1) {
		if (g_vars.game_over_tilt == 0 && g_vars.bonus_activation_flags[1] == 0) {
			int count = g_vars.bonus_multiplier - 1;
			do {
				score_bcd(g_vars.bonus_score1_bcd);
			} while (--count != -1);
			g_vars.score_multiplier = g_vars.bonus_multiplier;
		}
		if (g_vars.shoot_again_flag != 2) {
			--g_vars.balls_count;
		}
	}
	ball_drained_reset_lights();
	reset_lights_sea();
}

static void state_ball_drained() {
	++g_vars.state_counter;
	if (g_vars.state_counter != 2) {
		ball_drained_set_music();
	} else {
		g_vars.state_counter = 0;
		ball_drained_award_bonus();
		g_vars.state_num = 4;
	}
}

static void state_drain_delay() {
	++g_vars.state_counter;
	if (g_vars.state_counter >= 100) {
		g_vars.state_counter = 0;
		g_vars.state_num = 5;
	}
}

static void reset_progress_lights() {
	reset_lights_hb();
	reset_lights_score_milestones();
	reset_lights_sea();
	if (g_vars.state.pearl_letter_timer != 0) {
		reset_lights_pearl();
	}
}

static void reset_progress_lights_and_led() {
	g_vars.led_bonus_shown = 0;
	reset_progress_lights();
}

static void state_reset_progress_lights() {
	reset_progress_lights_and_led();
	if (g_vars.shoot_again_flag != 0) {
		g_vars.state_counter = 60;
	}
	g_vars.state_num = 6;
}

static void bonus_countdown_step() {
	if (g_vars.game_over_tilt == 0 && g_vars.bonus_activation_flags[1] == 0 && g_vars.score_multiplier != 0 && (g_vars.random_num & 7) == 0) {
		--g_vars.score_multiplier;
		g_vars.sound_num = 11;
		g_vars.sound_volume = 64;
		g_vars.led_color_index = 1;
		bonus_score2_bcd(g_vars.bonus_score1_bcd);
		if (g_vars.led_bonus_shown == 0) {
			g_vars.led_bonus_shown = 1;
			g_vars.led_panel_mode = 2;
			g_vars.led_panel_mode_state = 2;
		}
	}
}

static void state_bonus_countdown() {
	bonus_countdown_step();
	++g_vars.state_counter;
	if (g_vars.state_counter >= 90) {
		g_vars.state_counter = 0;
		g_vars.state_num = 7;
	}
}

void clear_mission_state() {
	memset(&g_vars.state, 0, sizeof(g_vars.state));

	int16_t zero = 0;
	if (g_vars.state.pearl_letter_timer != 0) {
		g_vars.state.pearl_letter_timer = zero;
		g_vars.pearl_letter_count = zero;
	}
	if (g_vars.bonus_multiplier_hold == 0) {
		g_vars.bonus_multiplier = 1;
		g_vars.bonus_score1_bcd = 0;
		g_vars.bonus_score2_bcd = 0;
	}
	g_vars.bonus_multiplier_hold = 0;
}

static void state_clear_mission() {
	g_vars.led_panel_mode = 0;
	g_vars.led_panel_mode_state = 2;
	clear_mission_state();
	g_vars.state_num = 8;
}

static void state_ready_for_next() {
	g_vars.state_num = 9;
}

static void reset_pearl_and_multiplier_lights() {
	reset_lights_pearl();
	reset_lights_multipliers();
}

static void state_delay_reset_multiplier() {
	++g_vars.state_counter;
	if (g_vars.state_counter >= 10) {
		g_vars.state_counter = 0;

		reset_pearl_and_multiplier_lights();
		if (g_vars.shoot_again_flag == 0) {

		}
		g_vars.state_num = 10;
	}
}

static void save_player_highscore() {
	g_vars.hall_of_fame = 0;

}

static void state_next_ball_or_game_over() {
	if (g_vars.balls_count > 0) {
		g_vars.state_num = 0;
	} else {
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = 0;
		save_player_highscore();
		if (g_vars.hall_of_fame != 0) {
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_HALL_OF_FAME, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 0;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = -1;
			g_vars.music_order_num = 50;
			g_vars.state_num = 11;
		} else {
			g_vars.music_order_num = 47;
			g_vars.state_num = 12;
		}
	}
}

static void state_hall_of_fame() {
	if (g_vars.state_counter < 125) {
		++g_vars.state_counter;
		if (g_vars.state_counter < 125) {
			return;
		}
		g_vars.last_key_code = -1;
		g_vars.led_panel_mode = 5;
		g_vars.led_panel_mode_state = 2;
	}
	if (g_vars.hall_of_fame == 0) {
		g_vars.state_counter = 0;
		g_vars.state_num = 12;
	}
}

static void state_wait_for_space() {
	if (g_vars.keys[AMIGA_KEY_SPACE]) {
		g_vars.state_counter = 0;
		g_vars.state_num = 13;
	}
}

static void state_game_over() {
	g_host.Debug("state_game_over balls_count:%d", g_vars.balls_count);
	if (g_vars.balls_count == 0) {
		g_vars.music_order_num = 61;
		g_vars.game_over_flag = 1;
	}
	g_vars.state_num = 0;
}

void update_game_state() {

	switch (g_vars.state_num) {
	case 0:
		state_next_ball_delay();
		break;
	case 1:
		state_prepare_ball();
		break;
	case 2:

		break;
	case 3:
		state_ball_drained();
		break;
	case 4:
		state_drain_delay();
		break;
	case 5:
		state_reset_progress_lights();
		break;
	case 6:
		state_bonus_countdown();
		break;
	case 7:
		state_clear_mission();
		break;
	case 8:
		state_ready_for_next();
		break;
	case 9:
		state_delay_reset_multiplier();
		break;
	case 10:
		state_next_ball_or_game_over();
		break;
	case 11:
		state_hall_of_fame();
		break;
	case 12:
		state_wait_for_space();
		break;
	case 13:
		state_game_over();
		break;
	}
}

