#include "host.h"
#include "game.h"

void add_u32(uint32_t *p, uint32_t x) {
	assert((int32_t)x >= 0);
	if (*p + x >= *p) {
		*p += x;
	} else {
		*p = UINT_MAX;
	}
}

void score_bcd(uint32_t score) {
	if (g_vars.game_over_tilt == 0) {
		add_u32(&g_vars.score_bcd, score);
	}
}

void bonus_score1_bcd(uint32_t score) {
	if (g_vars.game_over_tilt == 0) {
		add_u32(&g_vars.bonus_score1_bcd, score);
	}
}

void bonus_score2_bcd(uint32_t score) {
	if (g_vars.game_over_tilt == 0) {
		add_u32(&g_vars.bonus_score2_bcd, score);
	}
}

static void process_mission_score(uint32_t bonus_active, uint32_t zero) {
	uint8_t flags;

	if (bonus_active == 0) {

	} else {
		flags = g_vars.bonus_data[0];
		if (flags != 0 && g_vars.state.special_bonus_data_num != 0) {
			g_vars.state.special_bonus_data_num = 0;
			if (g_vars.ball_xpos2 < 70) {
				score_bcd(BCD_640);
			} else {
				score_bcd(BCD_1_280);
			}
			bonus_score1_bcd(BCD_9_000);
			g_vars.sound_num = 21;
			g_vars.sound_volume = 64;
		} else if (g_vars.bonus_data[1] != 0 && g_vars.state.special_bonus_data_num != 1) {

			g_vars.state.special_bonus_data_num = 1;
			score_bcd(BCD_320);
			g_vars.sound_num = 29;
			g_vars.sound_volume = 64;
		} else if (g_vars.bonus_data[2] != 0 && g_vars.state.special_bonus_data_num != 2) {

			g_vars.state.special_bonus_data_num = 2;
			score_bcd(BCD_750);
			g_vars.sound_num = 24;
			g_vars.sound_volume = 64;
		} else if (g_vars.bonus_data[3] != 0 && g_vars.state.special_bonus_data_num != 3) {

			g_vars.state.special_bonus_data_num = 3;
			g_vars.sound_num = 26;
			g_vars.sound_volume = 56;
			score_bcd(BCD_150);
		} else if (g_vars.bonus_data[4] != 0 && g_vars.state.special_bonus_data_num != 4) {

			score_bcd(BCD_10_000);
			g_vars.sound_num = 26;
			g_vars.sound_volume = 56;
		} else if (g_vars.bonus_data[13] != 0 && g_vars.state.special_bonus_data_num != 13) {

			g_vars.ball_exit_locked_counter = 40;
		} else if (g_vars.bonus_data[18] != 0 && g_vars.state.special_bonus_data_num != 18) {

			g_vars.state.special_bonus_data_num = 18;
			g_vars.sound_num = 26;
			g_vars.sound_volume = 56;
			score_bcd(BCD_20_000);
		} else if (g_vars.bonus_data[15] != 0 && g_vars.state.special_bonus_data_num != 15) {

			g_vars.bonus_activation_flags[3] = 1;
			g_vars.state.special_bonus_data_num = 15;
			g_vars.sound_num = 26;
			g_vars.sound_volume = 56;
			score_bcd(BCD_150);
		}
	}

	if (g_vars.bonus_activation_flags[3] != 0) {
		++g_vars.bonus_activation_flags[3];
	}
	if (g_vars.bonus_activation_flags[3] >= 200) {
		g_vars.bonus_activation_flags[3] = 0;
	}
	if (g_vars.ball_ypos2 <= 380) {
		g_vars.bonus_activation_flags[3] = 0;
	}
	switch (g_vars.state.special_num) {
	case 0:
		special_award_events(bonus_active, zero);
		break;
	case 1:
		dive_mission_150k_score(bonus_active, zero);
		break;
	case 2:
		dive_mission_250k_score(bonus_active, zero);
		break;
	case 3:
		dive_mission_500k_score(bonus_active, zero);
		break;
	case 4:
		dive_mission_pearl_500k(bonus_active, zero);
		break;
	case 5:
		sea_lane_scoring(bonus_active, zero);
		break;
	}
}

void process_bonus() {
	int i = 0;
	uint32_t zero, bonus_active;

	zero = 0;
	bonus_active = 1;
	for (i = 0; i < 24; ++i) {
		if (g_vars.bonus_data[i] != 0) {
			break;
		}
	}
	if (i == 24) {
		g_vars.state.special_bonus_data_num = -1;
		bonus_active = 0;
	}
	if (g_vars.draw_ball_flag == 0) {
		return;
	}
	zero = 0;

	if (g_vars.state.special_num == 0) {
		special_award_events(bonus_active, zero);
	} else {
		process_mission_score(bonus_active, zero);
	}
}
