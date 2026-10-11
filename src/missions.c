#include "host.h"
#include "game.h"

static void set_lights_entering_mission() {
	g_vars.state.special_num = g_vars.special_next;
	g_vars.state.special_timer = 1;
	g_vars.jingle_order_num = 44;
	memcpy(g_vars.lights_state_saved, g_vars.lights_state_table, sizeof(g_vars.lights_state_table));
	reset_lights_score_milestones();
	reset_lights_hb();
	reset_lights_sea();
	reset_lights_arrows();
	static const uint8_t special_mission_lights[] = {
		1, 0x66, 0, 0, 0, 0, 0, 0,
		1, 0x60, 0, 0, 0, 0, 0, 0,
		1, 0x62, 0, 0, 0, 0, 0, 0,
		2, 0x4C, 0x4E, 0, 0, 0, 0, 0,
		5, 0x34, 0x36, 0x38, 0x40, 0x42, 0, 0
	};
	const uint8_t *entry = special_mission_lights;
	int16_t mission = g_vars.state.special_num - 1;
	if (mission >= 0) {
		entry += (mission << 4) / sizeof(uint16_t);
		int16_t count = *entry++;
		for (int i = 0; i < count; ++i) {
			light_set2(*entry++);
		}
	}
}

static void restore_lights(uint32_t off) {
	memcpy(g_vars.lights_state_table, g_vars.lights_state_saved, sizeof(g_vars.lights_state_table));
	lights_update_state();
	g_vars.left_flipper_on = off;
	g_vars.right_flipper_on = off;
	g_vars.special_input_mask = off;
}

static void skill_shot() {
	if (g_vars.game_over_tilt == 0) {
		g_vars.bonus_activation_flags[1] = 0;
		g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_SKILL_SHOT, 0);
		g_vars.led_panel_animation_style_flag = 1;
		g_vars.led_change_color_flag = 1;
		g_vars.led_unused_flag = 0;
		g_vars.led_panel_counter = 177;
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		if (g_vars.game_over_tilt == 0) {
			g_vars.jingle_order_num = 57;
		}
		g_vars.skill_shot_score_index += 4;
		if (g_vars.skill_shot_score_index > 16) {
			g_vars.skill_shot_score_index = 16;
		}
		static const uint32_t scores[] = {
			BCD_100_000 ,
			BCD_250_000,
			BCD_500_000,
			BCD_1_000_000,
			BCD_2_000_000,
		};
		score_bcd(scores[g_vars.skill_shot_score_index / 4]);
	}
}

void special_award_events(uint32_t bonus_active, uint32_t zero) {
	int16_t n, tmp, tmp2, light;
	uint8_t flags;

	g_vars.special_input_mask = (g_vars.left_flipper_on << 16) | g_vars.right_flipper_on;
	if (bonus_active == 0) {

		goto special_done;
	} else {
		if ((READ_BE_UINT32(g_vars.bonus_data) | READ_BE_UINT32(g_vars.bonus_data + 4)) != 0) {

			goto bonus_data_0_8;
		}
		if ((READ_BE_UINT32(g_vars.bonus_data + 8) | READ_BE_UINT32(g_vars.bonus_data + 12)) != 0) {

			goto bonus_data_8_16;
		}
		flags = g_vars.bonus_data[16];
		if (flags == 3) {
			g_vars.bonus_activation_flags[6] = 1;
		}
		flags = g_vars.bonus_data[17];
		if (flags == 1 && g_vars.state.special_bonus_data_num != 17) {
			g_vars.state.special_bonus_data_num = 17;
			bonus_score1_bcd(BCD_75_000);
			if (g_vars.state.dive_mission_available != 0) {
				light_set5(0x1e);
				light_set5(0x20);
				light_set5(0x22);
				light_set5(0x24);
				light_set5(0x50);
				light_set5(0x3e);
				if (g_vars.state.bonus_increase_available != 0) {
					light_set5(0x4a);
					light_set5(0x62);
				}
				g_vars.state.dive_letters_mask = zero;
				g_vars.state.dive_mission_available = zero;
				g_vars.state.dive_mission_completed = zero;
				g_vars.state.bonus_increase_available = zero;
				g_vars.state.bonus_increase_completed = zero;
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_ENTERING_MISSION, 0);
				g_vars.led_panel_animation_style_flag = 0;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
				set_lights_entering_mission();
			}
			goto special_done;
		}

		flags = g_vars.bonus_data[18];
		if (flags != 0 && g_vars.state.special_bonus_data_num != 18) {
			g_vars.state.special_bonus_data_num = 18;
			g_vars.sound_num = 26;
			g_vars.sound_volume = 56;
			score_bcd(BCD_20_000);
			if ((flags & 1) != 0) {
				g_vars.state.lane_a_lit = 1;
				light_set1(0x26);
				if (g_vars.state.lane_b_lit != 0 && g_vars.state.bonus_multiplier_hold == 0) {
					g_vars.state.lanes_complete_flag = 1;
					light_set2(0x46);
					light_set2(0x62);
				}
			} else {
				g_vars.state.lane_b_lit = 1;
				light_set1(0x28);
				if (g_vars.state.lane_a_lit != 0 && g_vars.state.bonus_multiplier_hold == 0) {
					g_vars.state.lanes_complete_flag = 1;
					light_set2(0x46);
					light_set2(0x62);
				}
			}
			goto special_done;
		}

		flags = g_vars.bonus_data[19];
		if (flags != 0 && g_vars.state.special_bonus_data_num != 19) {
			g_vars.state.special_bonus_data_num = 19;
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 59;
			}
			score_bcd(BCD_7_000_000);
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_UDS_IS_GOOD_FOR_YOU, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 96;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		}
		goto special_done;
	}

bonus_data_0_8:
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
		goto special_done;
	}

	flags = g_vars.bonus_data[1];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 1) {
		g_vars.state.special_bonus_data_num = 1;
		score_bcd(BCD_320);
		g_vars.sound_num = 29;
		g_vars.sound_volume = 64;
		goto special_done;
	}

	flags = g_vars.bonus_data[2];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 2) {
		g_vars.state.special_bonus_data_num = 2;
		g_vars.sound_num = 24;
		g_vars.sound_volume = 64;
		goto bonus_data_5;
	}

	flags = g_vars.bonus_data[3];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 3) {
		g_vars.state.special_bonus_data_num = 3;
		g_vars.sound_num = 26;
		g_vars.sound_volume = 56;
		score_bcd(BCD_150);
		goto special_done;
	}

	flags = g_vars.bonus_data[4];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 4) {
		g_vars.sound_num = 26;
		g_vars.sound_volume = 56;
		if ((flags & 1) != 0) {

			g_vars.state.sea_s_letter_on = 1;
			light = 0x34;
			if (g_vars.bonus_activation_flags[0] != 0) {
				if (g_vars.light_sea_letter == light) {
					skill_shot();
				} else {
					g_vars.skill_shot_score_index = zero;
					light_set5(g_vars.light_sea_letter);
				}
			}

		} else if ((flags & 2) != 0) {

			g_vars.state.sea_e_letter_on = 1;
			light = 0x36;
			if (g_vars.bonus_activation_flags[0] != 0) {
				if (g_vars.light_sea_letter == light) {
					skill_shot();
				} else {
					g_vars.skill_shot_score_index = zero;
					light_set5(g_vars.light_sea_letter);
				}
			}

		} else {

			g_vars.state.sea_a_letter_on = 1;
			light = 0x38;
			if (g_vars.bonus_activation_flags[0] != 0) {
				if (g_vars.light_sea_letter == light) {
					skill_shot();
				} else {
					g_vars.skill_shot_score_index = zero;
					light_set5(g_vars.light_sea_letter);
				}
			}

		}

		g_vars.bonus_activation_flags[0] = zero;
		n = g_vars.state.sea_s_letter_on + g_vars.state.sea_e_letter_on + g_vars.state.sea_a_letter_on;
		if (n == 3) {
			g_vars.state.sea_s_letter_on = zero;
			g_vars.state.sea_e_letter_on = zero;
			g_vars.state.sea_a_letter_on = zero;
			light_set0(0x34);
			light_set0(0x36);
			light_set0(0x38);
			g_vars.state.sea_letters_cooldown = 1;
			g_vars.state.underwater_progress = 1;
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 66;
			}
			++g_vars.state.sea_score_index;
			if (g_vars.state.sea_score_index > 9) {
				g_vars.state.sea_score_index = 9;
			}
			static const uint32_t scores[] = {
				BCD_25_000,
				BCD_50_000,
				BCD_100_000,
				BCD_200_000,
				BCD_300_000,
				BCD_500_000,
				BCD_750_000,
				BCD_1_000_000,
				BCD_2_000_000,
				BCD_3_000_000,
			};
			bonus_score1_bcd(scores[g_vars.state.sea_score_index]);
			score_bcd(BCD_250_000);
		} else {
			score_bcd(BCD_10_000);
			if (g_vars.state.sea_letters_cooldown == 0) {
				light_set1(light);
			}
		}
		goto special_done;
	}

bonus_data_5:
	flags = g_vars.bonus_data[5];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 5) {
		g_vars.state.special_bonus_data_num = 5;
		score_bcd(BCD_750);
		if ((flags & (1 << 0)) != 0) {
			g_vars.state.dive_letters_mask |= 0x1111;
			light_set0(0x1e);
		}
		if ((flags & (1 << 1)) != 0) {
			g_vars.state.dive_letters_mask |= 0x2222;
			light_set0(0x20);
		}
		if ((flags & (1 << 2)) != 0) {
			g_vars.state.dive_letters_mask |= 0x4444;
			light_set0(0x22);
		}
		if ((flags & (1 << 3)) != 0) {
			g_vars.state.dive_letters_mask |= 0x8888;
			light_set0(0x24);
		}

		if (g_vars.state.dive_letters_mask == 0xFFFF) {
			if (g_vars.state.bonus_increase_completed == 0 && g_vars.bonus_multiplier != 7) {
				g_vars.state.bonus_increase_available = 1;
				g_vars.state.bonus_increase_completed = 1;
				g_vars.led_panel_text_loop_flag = 0;
				g_vars.led_color_blinking_speed = 1;
				g_vars.led_panel_text = Resource_GetData(g_vars.res, RES_TXT_INCREASE_BONUS_LIT, 0);
				g_vars.led_panel_mode = 1;
				g_vars.led_panel_mode_state = 2;
				light_set2(0x4a);
				light_set2(0x62);
			}

			if (g_vars.state.dive_mission_completed == 0) {
				g_vars.state.dive_mission_available = 1;
				g_vars.state.dive_mission_completed = 1;
				light_set2(0x50);
				light_set2(0x3e);
			}
		}
		goto special_done;
	}

	flags = g_vars.bonus_data[6];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 6) {
		g_vars.state.special_bonus_data_num = 6;
		++g_vars.state.secret_passage_hits;
		if (g_vars.state.secret_passage_hits >= 7) {
			patch_phys_map_open();
		}
		goto special_done;
	}

	flags = g_vars.bonus_data[7];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 7) {
		g_vars.state.special_bonus_data_num = 7;
		if (g_vars.kickback_enabled != 0) {
			g_vars.kickback_enabled = zero;
			g_vars.ball_yvel = -420;
			g_vars.ball_xvel = zero;
		}
		goto special_done;
	}

	goto special_done;

bonus_data_8_16:
	flags = g_vars.bonus_data[8];
	if (flags == 1 && g_vars.state.special_bonus_data_num != 8) {
		g_vars.state.special_bonus_data_num = 8;
		score_bcd(BCD_100_000);
		if (g_vars.state.lanes_complete_flag != 0) {
			g_vars.state.lanes_complete_flag = zero;
			g_vars.state.bonus_multiplier_hold = 1;
			light_set1(0x46);
			light_set5(0x62);
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 39;
			}
		}

		if (g_vars.state.bonus_increase_available != 0) {
			light_set5(0x50);
			light_set5(0x3e);
			light_set5(0x1e);
			light_set5(0x20);
			light_set5(0x22);
			light_set5(0x24);
			if (g_vars.bonus_activation_flags[5] == 0) {
				light_set5(0x4a);
				light_set5(0x62);
			} else {
				light_set2(0x4a);
			}

			++g_vars.bonus_multiplier;
			if (g_vars.bonus_multiplier >= 8) {
				g_vars.bonus_multiplier = 7;
			}
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 39;
			}

			g_vars.state.dive_letters_mask = zero;
			g_vars.state.bonus_increase_completed = zero;
			g_vars.state.bonus_increase_available = zero;
			g_vars.state.dive_mission_completed = zero;
			g_vars.state.dive_mission_available = zero;
			n = 0x50 + g_vars.bonus_multiplier * 2;
			light_set1(n);
			n += 2;
			if (n >= 0x60) {
				light_set1(0x4a);
				goto special_done;
			}

			light_set0(n);
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_BONUS_INCREASED, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 80;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		}

		if (g_vars.bonus_activation_flags[5] != 0) {
			g_vars.state.bonus_increase_available = zero;
			g_vars.state.bonus_increase_completed = zero;
			++g_vars.bonus_multiplier;
			if (g_vars.bonus_multiplier >= 8) {
				g_vars.bonus_multiplier = 7;
			}
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 39;
			}
			n = 0x50 + g_vars.bonus_multiplier * 2;
			light_set1(n);
			n += 2;
			if (n >= 0x60) {
				light_set1(0x4a);
				goto special_done;
			}

			light_set0(n);
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_BONUS_INCREASED, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 80;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		}

		if (g_vars.state.extra_ball_flag1 != 0) {
			++g_vars.balls_count;
			g_vars.state.extra_ball_flag1 = zero;
			light_set5(0x48);
			light_set5(0x62);
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 65;
			}
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_EXTRA_BALL, 0);
			g_vars.led_panel_animation_style_flag = 0;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		}

		if (g_vars.state.extra_ball_lit_counter) {
			++g_vars.balls_count;
			g_vars.state.extra_ball_lit_counter = zero;
			g_vars.state.special_mode = zero;
			light_set5(0x48);
			light_set5(0x62);
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 65;
			}
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_EXTRA_BALL, 0);
			g_vars.led_panel_animation_style_flag = 0;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		}

		goto special_done;
	}

	if (g_vars.bonus_data[9] != 0 && g_vars.state.special_bonus_data_num != 9) {
		g_vars.state.special_bonus_data_num = 9;
		goto special_done;
	}

	if (g_vars.bonus_data[11] != 0 && g_vars.state.special_bonus_data_num != 11) {
		g_vars.state.special_bonus_data_num = 11;
		g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_SHRIMP_ATTACK, 0);
		g_vars.led_panel_animation_style_flag = 1;
		g_vars.led_change_color_flag = 1;
		g_vars.led_unused_flag = 0;
		g_vars.led_panel_counter = 70;
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		tmp = 0;
		if (g_vars.state.shrimp_attack_timer != 0) {
			g_vars.state.shrimp_attack_timer = zero;
			tmp = 1;
			score_bcd(BCD_2_000_000);
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_TWO, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 0;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			g_vars.state.led_message_timer = 0x30;
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 39;
				g_vars.state.special_mode = 0;
			}
			light_set5(0x3a);
			light_set5(0x40);
			light_set5(0x42);
		}

		if (g_vars.bonus_activation_flags[5] != 0) {
			g_vars.bonus_activation_flags[5] = zero;
			g_vars.state.shrimp_attack_timer = zero;
			tmp = 1;
			score_bcd(BCD_2_000_000);
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_TWO, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 0;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			g_vars.state.led_message_timer = 0x30;
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 39;
				g_vars.state.special_mode = 0;
			}
			light_set5(0x4a);
			light_set5(0x62);
			light_set5(0x3a);
			light_set5(0x40);
			light_set5(0x42);
		}

		if (g_vars.state.million_point_bonus_flag != 0) {
			g_vars.state.million_point_bonus_flag = zero;
			tmp = 1;
			score_bcd(BCD_4_000_000);
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 57;
			}
		}

		if (g_vars.state.safe_ball_lit != 0) {
			g_vars.state.safe_ball_lit = zero;
			g_vars.bonus_activation_flags[8] = 1;
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 65;
			}
			g_vars.led_panel_text_loop_flag = 0;
			g_vars.led_color_blinking_speed = 1;
			g_vars.led_panel_text = Resource_GetData(g_vars.res, RES_TXT_SAFE_BALL, 0);
			g_vars.led_panel_mode = 1;
			g_vars.led_panel_mode_state = 2;
			light_set1(0x52);
			light_set5(0x40);
			light_set5(0x42);
		}

		if (tmp == 0) {
			score_bcd(BCD_75_000);
		}
		goto special_done;
	}

	flags = g_vars.bonus_data[12];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 12) {
		g_vars.state.special_bonus_data_num = 12;
		if (g_vars.state.special_mode == 0) {
			score_bcd(BCD_100_000);
			bonus_score1_bcd(BCD_50_000);
			g_vars.jingle_order_num = 55;
		}
		n = g_vars.deep_ramp_counter + 1;
		if (n == 10) {
			g_vars.deep_ramp_counter = n;
			goto start_dive_mission;
		}

		g_vars.deep_ramp_counter = n;
		if (n == 9) {
			g_vars.kickback_enabled = 1;
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_KICKBACK_ACTIVATED, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 100;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = 4;
			g_vars.state.extra_ball_flag1 = 1;
			g_vars.led_panel_text_loop_flag = 0;
			g_vars.led_color_blinking_speed = 1;
			g_vars.led_panel_text = Resource_GetData(g_vars.res, RES_TXT_EXTRA_BALL_LIT, 0);
			g_vars.led_panel_mode = 1;
			g_vars.led_panel_mode_state = 2;
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 59;
			}
			light_set2(0x48);
			light_set2(0x62);
			goto special_done;
		} else if (n == 7) {
			g_vars.kickback_enabled = 1;
			if (g_vars.bonus_activation_flags[8] != 0) {
				goto start_dive_mission;
			}
			g_vars.state.safe_ball_lit = 1;
			g_vars.led_panel_text_loop_flag = 0;
			g_vars.led_color_blinking_speed = 1;
			g_vars.led_panel_text = Resource_GetData(g_vars.res, RES_TXT_SAFE_BALL_LIT, 0);
			g_vars.led_panel_mode = 1;
			g_vars.led_panel_mode_state = 2;
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 59;
			}
			light_set2(0x52);
			light_set2(0x40);
			light_set2(0x42);
			goto special_done;

		} else if (n == 5 || n == 3) {
			g_vars.kickback_enabled = 1;
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_KICKBACK_ACTIVATED, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 100;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = 4;
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 66;
			}
			goto special_done;

		} else {

			if (n & 1) {
				goto special_done;
			}
		}

start_dive_mission:
		if (g_vars.state.special_mode == 0 || g_vars.state.special_mode == 1) {

			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_DIVING_DEEPER, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 0;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = 4;
			g_vars.state.shrimp_attack_timer = 1;
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 41;
				g_vars.state.special_mode = 1;
			}
			light_set2(0x3a);
			light_set2(0x40);
			light_set2(0x42);
		}

		goto special_done;
	}

	if (g_vars.bonus_data[13] != 0 && g_vars.state.special_bonus_data_num != 13) {
		g_vars.state.special_bonus_data_num = 13;
		if (g_vars.state.special_mode != 0) {
			g_vars.ball_exit_locked_counter = 40;
			goto special_done;
		}
		n = g_vars.random_num;
		tmp = n % 150;
		n %= 50;
		g_vars.bonus_activation_flags[4] = n;
		if (g_vars.state.underwater_progress == 0 && tmp >= 50) {
			g_vars.ball_exit_locked_counter = 40;
			g_vars.bonus_activation_flags[2] = 1;
			light_set1(0x3c);
			goto special_done;
		}

		g_vars.state.underwater_progress = zero;
		light_set1(0x3c);
		if (g_vars.state.special_mode == 0) {
			g_vars.jingle_order_num = 56;
		}

		g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_HAPPY_STARFISH, 0);
		g_vars.led_panel_animation_style_flag = 1;
		g_vars.led_change_color_flag = 0;
		g_vars.led_unused_flag = 0;
		g_vars.led_panel_counter = 105;
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
		if (g_vars.state.starfish_bonus_timer == 0) {
			g_vars.state.starfish_bonus_timer = 1;
		}
		goto special_done;
	}

	if (g_vars.bonus_data[14] != 0 && g_vars.state.special_bonus_data_num != 14) {
		g_vars.state.special_bonus_data_num = 14;
		if (g_vars.state.pearl_letter_timer != 0) {
			g_vars.state.pearl_letter_timer = zero;
			g_vars.pearl_letter_count = zero;
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 54;
				g_vars.state.special_mode = 0;
			}
			g_vars.ball_exit_locked_counter = 80;
			g_vars.state.pearl_bonus_timer = 1;
			if (g_vars.pearl_double_bonus == 0) {
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_THREE, 0);
				g_vars.led_panel_animation_style_flag = 0;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
				g_vars.state.led_message_timer = 0x30;
				score_bcd(BCD_3_000_000);
			} else {
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_SIX, 0);
				g_vars.led_panel_animation_style_flag = 0;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
				g_vars.state.led_message_timer = 0x3a;
				score_bcd(BCD_6_000_000);
			}

			n = 0x4c;
			if (g_vars.pearl_double_bonus != 0) {
				n = 0x4e;
			}
			light_set5(n);
			g_vars.pearl_double_bonus = 1;
			light_set6(0x44);
			goto special_done;
		}

		score_bcd(BCD_200_000);
		if (g_vars.state.special_mode == 0) {
			g_vars.jingle_order_num = 40;
		}

		++g_vars.pearl_letter_count;
		if (g_vars.pearl_letter_count >= 5) {
			if (g_vars.state.special_mode != 0) {
				g_vars.pearl_letter_count = 4;
				goto special_done;
			}

			g_vars.jingle_order_num = 34;
			g_vars.state.special_mode = 2;
			light_set0(0x32);
			g_vars.state.pearl_letter_timer = 1;
			n = 0x4c;
			if (g_vars.pearl_double_bonus != 0) {
				n = 0x4e;
			}
			light_set2(n);
		} else {

			light_set0(0x28 + g_vars.pearl_letter_count * 2);
		}

		goto special_done;
	}

	if (g_vars.bonus_data[15] != 0 && g_vars.state.special_bonus_data_num != 15) {
		g_vars.state.special_bonus_data_num = 15;

		g_vars.sound_num = 26;
		g_vars.sound_volume = 56;
		score_bcd(BCD_150);
		goto special_done;
	}

special_done:
	zero = 0;
	if (g_vars.special_input_mask != 0) {
		g_vars.left_flipper_on = 0;
		g_vars.right_flipper_on = 0;
		g_vars.special_input_mask = 0;
		tmp = g_vars.state.sea_s_letter_on;
		tmp2 = g_vars.state.sea_e_letter_on;
		light = g_vars.state.sea_a_letter_on;
		if (g_vars.state.sea_letters_cooldown == 0) {
			if (light != g_vars.state.sea_s_letter_on) {
				n = 0x34;
				if (light != 0) {
					light_set1(n);
				} else {
					light_set5(n);
				}
			}
			if (tmp != g_vars.state.sea_e_letter_on) {
				n = 0x36;
				if (tmp != 0) {
					light_set1(n);
				} else {
					light_set5(n);
				}
			}
			if (tmp2 != g_vars.state.sea_a_letter_on) {
				n = 0x38;
				if (tmp2 != 0) {
					light_set1(n);
				} else {
					light_set5(n);
				}
			}
		}
		g_vars.state.sea_e_letter_on = tmp;
		g_vars.state.sea_a_letter_on = tmp2;
		g_vars.state.sea_s_letter_on = light;
	}

	if (g_vars.state.shrimp_attack_timer != 0) {
		++g_vars.state.shrimp_attack_timer;
	}
	if (g_vars.state.pearl_letter_timer != 0) {
		++g_vars.state.pearl_letter_timer;
	}
	if (g_vars.state.starfish_bonus_timer != 0) {
		++g_vars.state.starfish_bonus_timer;
	}
	if (g_vars.state.extra_ball_lit_counter != 0) {
		++g_vars.state.extra_ball_lit_counter;
	}
	if (g_vars.state.sea_letters_cooldown != 0) {
		++g_vars.state.sea_letters_cooldown;
	}
	if (g_vars.bonus_activation_flags[3] != 0) {
		++g_vars.bonus_activation_flags[3];
	}

	if (g_vars.state.pearl_bonus_timer != 0) {
		++g_vars.state.pearl_bonus_timer;
	}
	if (g_vars.bonus_activation_flags[1] != 0) {
		++g_vars.bonus_activation_flags[1];
	}
	if (g_vars.bonus_activation_flags[5] != 0) {
		++g_vars.bonus_activation_flags[5];
	}
	n = g_vars.state.led_message_timer - 1;
	if (n < 0) {
		n = 0;
	}

	g_vars.state.led_message_timer = n;
	if (n == 1) {
		g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_MESSAGE_END, 0);
		g_vars.led_panel_animation_style_flag = 0;
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
	}
	n = g_vars.bonus_activation_flags[6];
	if (n != 0) {
		++g_vars.bonus_activation_flags[6];
	}
	if (g_vars.state.shrimp_attack_timer == 702 && g_vars.game_over_tilt == 0) {
		g_vars.jingle_order_num = 43;
	}
	if (g_vars.state.shrimp_attack_timer >= 782) {
		g_vars.state.shrimp_attack_timer = 0;
		g_vars.state.special_mode = 0;
		light_set5(0x3a);
		light_set5(0x40);
		light_set5(0x42);
	}
	if (g_vars.state.extra_ball_lit_counter == 702 && g_vars.game_over_tilt == 0) {
		g_vars.jingle_order_num = 43;
	}
	if (g_vars.state.extra_ball_lit_counter >= 782) {
		g_vars.state.extra_ball_lit_counter = 0;
		g_vars.state.special_mode = 0;
		if (g_vars.state.extra_ball_flag1 == 0) {
			light_set5(0x48);
			light_set5(0x62);
		}
	}

	if (g_vars.state.pearl_letter_timer == 1622 && g_vars.game_over_tilt == 0) {
		g_vars.jingle_order_num = 35;
	}
	if (g_vars.state.pearl_letter_timer >= 1734) {
		g_vars.state.pearl_letter_timer = 0;
		g_vars.pearl_letter_count = 0;
		g_vars.state.special_mode = 0;
		n = 0x4c;
		if (g_vars.pearl_double_bonus != 0) {
			n = 0x4e;
		}
		light_set5(n);
		light_set5(0x2a);
		light_set5(0x2c);
		light_set5(0x2e);
		light_set5(0x30);
		light_set5(0x32);
	}

	if (g_vars.bonus_activation_flags[3] >= 200) {
		g_vars.bonus_activation_flags[3] = 0;
	}
	if (g_vars.ball_ypos2 <= 380) {
		g_vars.bonus_activation_flags[3] = 0;
	}
	if (g_vars.state.pearl_bonus_timer >= 154) {
		g_vars.state.pearl_bonus_timer = 0;
		light_set7(0x44);
	}
	if (g_vars.bonus_activation_flags[1] >= 275) {
		g_vars.bonus_activation_flags[1] = 0;
	}
	if (g_vars.bonus_activation_flags[6] >= 40) {
		g_vars.bonus_activation_flags[6] = 0;
		++g_vars.ball_xvel;
		if (g_vars.ball_xvel < 0) {
			g_vars.ball_xvel -= 2;
		}
	}

	if (g_vars.bonus_activation_flags[5] == 500) {
		if (g_vars.state.bonus_increase_available == 0) {
			light_set0(0x4a);
		}
		if (g_vars.state.shrimp_attack_timer == 0) {
			light_set0(0x3a);
		}
	}
	if (g_vars.bonus_activation_flags[5] >= 750) {
		g_vars.bonus_activation_flags[5] = 0;
		g_vars.state.special_mode = 0;
		if (g_vars.state.bonus_increase_available == 0) {
			light_set5(0x4a);
			light_set5(0x62);
		}

		if (g_vars.state.shrimp_attack_timer == 0) {
			light_set5(0x3a);
			light_set5(0x40);
			light_set5(0x42);
		}
	}

	if (g_vars.state.starfish_bonus_timer >= 102) {
		g_vars.state.starfish_bonus_timer = 0;
		light_set5(0x3c);
		n = g_vars.bonus_activation_flags[4];
		if (n < 29) {
			if (n < 10) {
				score_bcd(BCD_50_000);
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_50_000, 0);
				g_vars.led_panel_animation_style_flag = 1;
				g_vars.led_change_color_flag = 1;
				g_vars.led_unused_flag = 0;
				g_vars.led_panel_counter = 70;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
				if (g_vars.state.special_mode == 0) {
					g_vars.jingle_order_num = 66;
				}
			} else if (n < 18) {
				score_bcd(BCD_100_000);
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_100_000, 0);
				g_vars.led_panel_animation_style_flag = 1;
				g_vars.led_change_color_flag = 1;
				g_vars.led_unused_flag = 0;
				g_vars.led_panel_counter = 70;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
				if (g_vars.state.special_mode == 0) {
					g_vars.jingle_order_num = 66;
				}
			} else if (n < 24) {
				score_bcd(BCD_200_000);
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_200_000, 0);
				g_vars.led_panel_animation_style_flag = 1;
				g_vars.led_change_color_flag = 1;
				g_vars.led_unused_flag = 0;
				g_vars.led_panel_counter = 70;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
				if (g_vars.state.special_mode == 0) {
					g_vars.jingle_order_num = 66;
				}
			} else {
				score_bcd(BCD_500_000);
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_500_000, 0);
				g_vars.led_panel_animation_style_flag = 1;
				g_vars.led_change_color_flag = 1;
				g_vars.led_unused_flag = 0;
				g_vars.led_panel_counter = 70;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
				if (g_vars.state.special_mode == 0) {
					g_vars.jingle_order_num = 66;
				}
			}
			goto after_special_award;

		} else if (n < 33) {

			score_bcd(BCD_2_000_000);
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_TWO, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 0;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			g_vars.state.led_message_timer = 48;
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 59;
			}
			goto after_special_award;

		} else if (n < 39) {
			g_vars.state.shrimp_attack_timer = 1;
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 41;
				g_vars.state.special_mode = 1;
			}
			light_set2(0x3a);
			light_set2(0x40);
			light_set2(0x42);
			goto after_special_award;

		} else if (n < 45) {

			g_vars.kickback_enabled = 1;
			g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_KICKBACK_ACTIVATED, 0);
			g_vars.led_panel_animation_style_flag = 1;
			g_vars.led_change_color_flag = 1;
			g_vars.led_unused_flag = 0;
			g_vars.led_panel_counter = 80;
			g_vars.led_panel_mode_state = 2;
			g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 66;
			}
			goto after_special_award;

		} else if (n < 48) {

			++g_vars.bonus_multiplier;
			if (g_vars.bonus_multiplier >= 8) {
				g_vars.bonus_multiplier = 7;
			}
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 39;
			}
			n = 0x50 + g_vars.bonus_multiplier * 2;
			light_set1(n);
			n += 2;
			if (n >= 0x60) {
				light_set1(0x4a);
			} else {
				light_set0(n);
				g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_BONUS_INCREASED, 0);
				g_vars.led_panel_animation_style_flag = 1;
				g_vars.led_change_color_flag = 1;
				g_vars.led_unused_flag = 0;
				g_vars.led_panel_counter = 80;
				g_vars.led_panel_mode_state = 2;
				g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
			}
			goto after_special_award;
		} else {

			g_vars.state.extra_ball_lit_counter = 1;
			if (g_vars.game_over_tilt == 0) {
				g_vars.jingle_order_num = 41;
				g_vars.state.special_mode = 3;
			}

			g_vars.led_panel_text_loop_flag = 0;
			g_vars.led_color_blinking_speed = 1;
			g_vars.led_panel_text = Resource_GetData(g_vars.res, RES_TXT_EXTRA_BALL_LIT, 0);
			g_vars.led_panel_mode = 1;
			g_vars.led_panel_mode_state = 2;
			light_set2(0x48);
			light_set2(0x62);
			goto after_special_award;
		}
	}

	if (g_vars.state.sea_letters_cooldown >= 50) {
		g_vars.state.sea_letters_cooldown = zero;
		n = 0x34;
		if (g_vars.state.sea_s_letter_on != 0) {
			light_set1(n);
		} else {
			light_set5(n);
		}
		n = 0x36;
		if (g_vars.state.sea_e_letter_on != 0) {
			light_set1(n);
		} else {
			light_set5(n);
		}
		n = 0x38;
		if (g_vars.state.sea_a_letter_on != 0) {
			light_set1(n);
		} else {
			light_set5(n);
		}
	}

after_special_award:

	n = g_vars.score_bcd / 100;
	if ((n / 10000) % 100 != 0 && (n % 10000) == 0) {
		if (g_vars.state.million_point_bonus_flag == 0) {
			g_vars.state.million_point_bonus_flag = 1;
			light_set2(0x64);
			light_set2(0x40);
			light_set2(0x42);
			if (g_vars.state.special_mode == 0) {
				g_vars.jingle_order_num = 57;
			}
		}
		return;
	}

	if (g_vars.state.million_point_bonus_flag != 0) {
		g_vars.state.million_point_bonus_flag = zero;
		light_set5(0x64);
		light_set5(0x40);
		light_set5(0x42);
	}
}

static void dive_mission_completed() {
	g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_DIVE_MISSION_COMPLETED, 0);
	g_vars.led_panel_animation_style_flag = 0;
	g_vars.led_panel_mode_state = 2;
	g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
}

void dive_mission_150k_score(uint32_t bonus_active, uint32_t zero) {
	if (g_vars.state.dive_mission_counter != 0) {
		if (g_vars.state.dive_mission_counter >= 87) {
			g_vars.state.dive_mission_counter = zero;
			g_vars.state.special_num = 0;
			++g_vars.special_next;
			dive_mission_completed();
			restore_lights(zero);
		} else {

			const int16_t counter = g_vars.state.dive_mission_counter;
			++g_vars.state.dive_mission_counter;
			if ((counter & 7) == 0) {
				score_bcd(BCD_150_000);
				g_vars.sound_num = 21;
				g_vars.sound_volume = 64;
				g_vars.led_color_index = 1;
			}
		}
	} else {

		if (g_vars.bonus_data[12] != 0 && g_vars.state.special_bonus_data_num == 12) {
			g_vars.state.special_bonus_data_num = 12;
			g_vars.state.dive_mission_counter = 1;
			g_vars.state.special_timer = 0;
			g_vars.jingle_order_num = 57;
			light_set5(0x66);
		} else {
			++g_vars.state.special_timer;
			if (g_vars.state.special_timer >= 1430) {
				g_vars.state.special_timer = 0;
				g_vars.state.special_num = 0;
				g_vars.jingle_order_num = 57;
				light_set5(0x66);
				restore_lights(zero);
			}
		}
	}
}

void dive_mission_250k_score(uint32_t bonus_active, uint32_t zero) {
	if (g_vars.state.dive_mission_counter != 0) {
		if (g_vars.state.dive_mission_counter >= 87) {
			g_vars.state.dive_mission_counter = zero;
			g_vars.state.special_num = 0;
			++g_vars.special_next;
			dive_mission_completed();
			restore_lights(zero);
		} else {
			const int16_t counter = g_vars.state.dive_mission_counter;
			++g_vars.state.dive_mission_counter;
			if ((counter & 7) == 0) {
				score_bcd(BCD_250_000);
				g_vars.sound_num = 21;
				g_vars.sound_volume = 64;
				g_vars.led_color_index = 1;
			}
		}
	} else {
		if (g_vars.bonus_data[13] != 0 && g_vars.state.special_bonus_data_num == 13) {
			g_vars.state.special_bonus_data_num = 13;
			g_vars.state.dive_mission_counter = 1;
			g_vars.state.special_timer = 0;
			g_vars.jingle_order_num = 57;
			light_set5(0x60);
		} else {
			++g_vars.state.special_timer;
			if (g_vars.state.special_timer >= 1430) {
				g_vars.state.special_timer = 0;
				g_vars.state.special_num = 0;
				g_vars.jingle_order_num = 57;
				light_set5(0x60);
				restore_lights(zero);
			}
		}
	}
}

void dive_mission_500k_score(uint32_t bonus_active, uint32_t zero) {
	if (g_vars.state.dive_mission_counter != 0) {
		if (g_vars.state.dive_mission_counter >= 87) {
			g_vars.state.dive_mission_counter = zero;
			g_vars.state.special_num = 0;
			++g_vars.special_next;
			dive_mission_completed();
			restore_lights(zero);
		} else {
			const int16_t counter = g_vars.state.dive_mission_counter;
			++g_vars.state.dive_mission_counter;
			if ((counter & 7) == 0) {
				score_bcd(BCD_500_000);
				g_vars.sound_num = 21;
				g_vars.sound_volume = 64;
				g_vars.led_color_index = 1;
			}
		}
	} else {
		if (g_vars.bonus_data[8] != 0 && g_vars.state.special_bonus_data_num == 8) {
			g_vars.state.special_bonus_data_num = 8;
			g_vars.state.dive_mission_counter = 1;
			g_vars.state.special_timer = 0;
			g_vars.jingle_order_num = 57;
			light_set5(0x62);
		} else {
			++g_vars.state.special_timer;
			if (g_vars.state.special_timer >= 1430) {
				g_vars.state.special_timer = 0;
				g_vars.state.special_num = 0;
				g_vars.jingle_order_num = 57;
				light_set5(0x62);
				restore_lights(zero);
			}
		}
	}
}

void dive_mission_pearl_500k(uint32_t bonus_active, uint32_t zero) {
	if (g_vars.state.dive_mission_counter != 0) {
		if (g_vars.state.dive_mission_counter >= 119) {
			g_vars.state.dive_mission_counter = zero;
			g_vars.state.special_num = 0;
			++g_vars.special_next;
			dive_mission_completed();
			restore_lights(zero);
		} else {
			int16_t counter = g_vars.state.dive_mission_counter;
			++g_vars.state.dive_mission_counter;
			if ((counter & 7) == 0) {
				score_bcd(BCD_500_000);
				g_vars.sound_num = 21;
				g_vars.sound_volume = 64;
				g_vars.led_color_index = 1;
			}
		}
	} else {

		int16_t flags = g_vars.bonus_data[14];
		if (flags == 1 && g_vars.state.special_bonus_data_num != 14) {
			g_vars.state.special_bonus_data_num = 14;
			g_vars.state.dive_mission_counter = 1;
			g_vars.state.special_timer = 0;
			g_vars.jingle_order_num = 57;
			light_set5(0x4c);
			light_set5(0x4e);
		} else {

			++g_vars.state.special_timer;
			if (g_vars.state.special_timer >= 1430) {
				g_vars.state.special_timer = 0;
				g_vars.state.special_num = 0;
				light_set5(0x4c);
				light_set5(0x4e);
				g_vars.jingle_order_num = 57;
				restore_lights(zero);
			}
		}
	}
}

void sea_lane_scoring(uint32_t bonus_active, uint32_t zero) {
	g_vars.special_input_mask = (g_vars.left_flipper_on << 16) | g_vars.right_flipper_on;
	if (g_vars.state.sea_letters_cooldown != 0) {
		if (g_vars.state.sea_letters_cooldown >= 163) {
			g_vars.state.sea_letters_cooldown = zero;
			g_vars.state.special_num = 0;
			g_vars.special_next = 1;
			light_set5(0x34);
			light_set5(0x36);
			light_set5(0x38);
			dive_mission_completed();
			restore_lights(zero);
		} else {
			const int16_t n = g_vars.state.sea_letters_cooldown;
			++g_vars.state.sea_letters_cooldown;
			if ((n & 3) == 0) {
				score_bcd(BCD_250_000);
				g_vars.sound_num = 29;
				g_vars.sound_volume = 64;
				g_vars.led_color_index = 1;
			}
		}
		return;
	}

	int16_t flags = g_vars.bonus_data[4];
	if (flags != 0 && g_vars.state.special_bonus_data_num != 4) {
		int16_t n, light;

		g_vars.state.special_bonus_data_num = 4;
		if (flags & 1) {
			g_vars.state.sea_lane_s_on = 1;
			light = 0x34;
		} else if (flags & 2) {
			g_vars.state.sea_lane_e_on = 1;
			light = 0x36;
		} else {
			g_vars.state.sea_lane_a_on = 1;
			light = 0x38;
		}
		n = g_vars.state.sea_lane_s_on + g_vars.state.sea_lane_e_on + g_vars.state.sea_lane_a_on;
		if (n == 3) {
			g_vars.state.sea_lane_s_on = zero;
			g_vars.state.sea_lane_e_on = zero;
			g_vars.state.sea_lane_a_on = zero;
			light_set0(0x34);
			light_set0(0x36);
			light_set0(0x38);
			light_set5(0x40);
			light_set5(0x42);
			g_vars.state.sea_letters_cooldown = 1;
			g_vars.state.special_timer = 0;
			g_vars.jingle_order_num = 57;
			return;
		}

		light_set1(light);
	}

	if (g_vars.special_input_mask != 0) {
		g_vars.left_flipper_on = 0;
		g_vars.right_flipper_on = 0;
		g_vars.special_input_mask = 0;
		int16_t tmp = g_vars.state.sea_lane_s_on;
		int16_t tmp2 = g_vars.state.sea_lane_e_on;
		int16_t light = g_vars.state.sea_lane_a_on;
		if (g_vars.state.sea_lane_s_on != light) {
			if (light != 0) {
				light_set1(0x34);
			} else {
				light_set2(0x34);
			}
		}
		if (g_vars.state.sea_lane_e_on != tmp) {
			if (tmp != 0) {
				light_set1(0x36);
			} else {
				light_set2(0x36);
			}
		}
		if (g_vars.state.sea_lane_a_on != tmp2) {
			if (tmp2 != 0) {
				light_set1(0x38);
			} else {
				light_set2(0x38);
			}
		}
		g_vars.state.sea_lane_e_on = tmp;
		g_vars.state.sea_lane_a_on = tmp2;
		g_vars.state.sea_lane_s_on = light;
	}

	++g_vars.state.special_timer;
	if (g_vars.state.special_timer >= 1430) {
		g_vars.state.special_timer = 0;
		g_vars.state.special_num = 0;
		g_vars.jingle_order_num = 57;
		light_set5(0x34);
		light_set5(0x36);
		light_set5(0x38);
		restore_lights(zero);
	}
}

