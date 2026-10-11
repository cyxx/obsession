#include "host.h"
#include "mixer.h"
#include "game.h"

static const int16_t _collision_angle_quadrants[] = {
	540, 360, 0, 180, 0, 100, 0, 100
};

static const int16_t ball_offsets_table[] = {
	0x1FF, 0x157,  0xAE,     5,     4,     3,     2,  0xA9, 0x150, 0x1F8,
	0x2A0, 0x348, 0x3F1, 0x49A, 0x49B, 0x49C, 0x49D, 0x3F6, 0x34F, 0x2A7,
	0x1FF, 0x157,  0xAE,     5,     4,     3,     2,  0xA9, 0x150, 0x1F8,
	0x2A0, 0x348, 0x3F1, 0x49A, 0x49B, 0x49C, 0x49D, 0x3F6, 0x34F, 0x2A7
};

static const int16_t _collision_angle_range_table[] = {
	0, 718, 0, 718, 0, 718, 0, 718, 0, 718,
	0, 718, 0, 718, 0, 718, 0, 718, 0, 0
};

static const int8_t _ball_directions_xy_table[] = {
	 1,  0,  1, -1,  0, -1, -1, -1,
	-1,  0, -1,  1,  0,  1,  1,  1
};

static void apply_flipper_impulse(int16_t impulse) {
	int16_t n0, n1, n2;

	n1 = g_vars.flipper_yvel;
	n2 = impulse;
	if (n2 < 0) {
		if (n1 < 0) {
			goto impulse_clamp;
		}
		n2 = -n2;
	} else {
		if (n1 >= 0) {
			goto impulse_clamp;
		}
		n1 = -n1;
	}
	n1 -= n2;
	if (!(n1 < 0)) {
		n2 = n1 >> 2;
		n1 -= n2;
		impulse += n1;
		if (!(impulse >= 0)) {
			impulse -= n1;
			impulse -= n1;
		}
	}
impulse_clamp:
	if (impulse > 511) {
		impulse = 511;
	} else if (impulse < -511) {
		impulse = -511;
	}
	g_vars.ball_yvel = impulse;
	n1 = g_vars.ball_flipper_xoffset - 19;
	if (n1 < 0) {
		n1 = 0;
	}
	n0 = n1;
	n2 = 16;
	if (g_vars.ball_near_flipper_side != 4) {
		n2 = 15;
	}
	{
		const int x = (n1 * n2 - 300) * n0;
		n1 = clip_s16(x >> 6);
	}
	if (g_vars.ball_near_flipper_side != 3) {
		n1 = -n1;
	}
	if (g_vars.was_ball_on_flipper_area != 0) {
		n1 >>= 1;
	}
	n2 = g_vars.flipper_xvel;
	n0 = n2 >> 1;
	n2 += n0;
	n1 -= n2;
	n1 >>= 1;
	n2 += n1;
	if (n2 > 511) {
		n2 = 511;
	}
	if (n2 < -511) {
		n2 = -511;
	}
	g_vars.ball_xvel = n2;
}

void ball_update() {

	int16_t n0, n1, n2, n3, n4, n5, n6, retry;
	const uint8_t *ptr;

	if (g_vars.ball_collision_type != BALL_IDLE_ON_FLIPPER) {
		g_vars.hint_counter = 0;
	} else {
		++g_vars.hint_counter;
		if (g_vars.hint_counter >= 100) {
			g_vars.hint_counter = -30000;
			g_vars.led_panel_text_loop_flag = 0;
			g_vars.led_color_blinking_speed = 1;
			g_vars.led_panel_text = Resource_GetHint(g_vars.res, g_vars.random_num & 7);
			g_vars.led_panel_mode = 1;
			g_vars.led_panel_mode_state = 2;
		}
	}
	if (g_vars.ball_xpos2 >= 308) {
		n1 = ABS(g_vars.ball_yvel);
		if (n1 <= 14 && g_vars.ball_ypos2 >= 440 && g_vars.tilt_state == 0) {
			g_vars.ball_xvel >>= 1;
			g_vars.ball_yvel >>= 1;
		}
	}

	if (g_vars.ball_exit_locked_counter == 20 && g_vars.bonus_activation_flags[2] != 0) {
		g_vars.bonus_activation_flags[2] = 0;
		light_set5(0x3c);
		g_vars.led_animation_ptr = Resource_GetData(g_vars.res, RES_LED_NOT_TASTY, 0);
		g_vars.led_panel_animation_style_flag = 1;
		g_vars.led_change_color_flag = 0;
		g_vars.led_unused_flag = 0;
		g_vars.led_panel_counter = 50;
		g_vars.led_panel_mode_state = 2;
		g_vars.led_panel_mode = LED_MODE_4_PLAYING_ANIMATION;
	}

	{
		uint8_t *ptr = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET + 0x4621;
		*ptr = 0x27;
		if (g_vars.ball_xpos2 <= 282) {
			*ptr = 0x29;
		}
	}
	--g_vars.ball_hit_cooldown;
	if (g_vars.ball_hit_cooldown < 0) {
		g_vars.ball_hit_cooldown = 0;
	}
	g_vars.tilt_state = 0;
	--g_vars.tilt_detection_counter;
	if (g_vars.tilt_detection_counter < 0) {
		g_vars.tilt_detection_counter = g_vars.tilt_confirmation_counter;
		g_vars.tilt_confirmation_counter = 0;
	}
	--g_vars.tilt_confirmation_counter;
	if (g_vars.tilt_confirmation_counter < 0) {
		g_vars.tilt_confirmation_counter = 0;
	}
	if (g_vars.tilt_detection_counter >= 56) {
		g_vars.tilt_state = 1;
	}
	if (g_vars.tilt_confirmation_counter >= 56) {
		g_vars.tilt_state = 2;
	}
	if (g_vars.tilt_state == 0) {
		g_vars.tilt_direction = 0;
	}
	--g_vars.tilt_launch_cooldown;
	if (g_vars.tilt_launch_cooldown == 1) {
		g_vars.tilt_activation_flag = 1;
		g_vars.reset_tilt = 0;
	}
	if (g_vars.tilt_launch_cooldown < 0) {
		g_vars.tilt_launch_cooldown = 0;
	}
	if (g_vars.reset_tilt != 0) {
		g_vars.tilt_state = 0;
	}
	memset(g_vars.bonus_data, 0, 24);
	if (g_vars.ball_collision_occurred != 0) {
		g_vars.physics_loop_counter = 0;
	}
	g_vars.ball_collision_flags = 0;

	if (g_vars.ball_xpos2 <= 147) {
		n0 = g_vars.left_flipper_frame_num;
		n0 += n0;
		n0 -= 2;
		if (n0 < 0) {
			n0 = 0;
		}
		const uint8_t *src = Resource_GetData(g_vars.res, RES_LFLIPPER_PHYSMAP, 0);
		assert(src[0x70] == 0x26);
		uint8_t *dst = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET + 0x8e94;
		update_physmap(n0, src, dst);
	} else {
		n0 = g_vars.right_flipper_frame_num;
		n0 += n0;
		n0 -= 2;
		if (n0 < 0) {
			n0 = 0;
		}
		const uint8_t *src = Resource_GetData(g_vars.res, RES_RFLIPPER_PHYSMAP, 0);
		assert(src[0x8b] == 0x2a);
		uint8_t *dst = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET + 0x8ebc;
		update_physmap(n0, src, dst);
	}

	g_vars.flipper_hit_counter = 0;
	g_vars.flipper_collision_type = 0;
	if (g_vars.left_flipper_pushed) {
		g_vars.left_flipper_change_counter = 3;
	}
	if (g_vars.right_flipper_pushed) {
		g_vars.right_flipper_change_counter = 3;
	}
	--g_vars.left_flipper_change_counter;
	if (g_vars.left_flipper_change_counter < 0) {
		g_vars.left_flipper_change_counter = 0;
	}
	--g_vars.right_flipper_change_counter;
	if (g_vars.right_flipper_change_counter < 0) {
		g_vars.right_flipper_change_counter = 0;
	}

	n0 = g_vars.ball_xpos2;
	if (g_vars.ball_xpos2 >= 147) {
		g_vars.ball_near_flipper_side = 4;
		g_vars.ball_flipper_collision_state = (g_vars.ball_flipper_collision_state & 0xFF00) | g_vars.right_flipper_state;
		g_vars.ball_flipper_activation_timer = g_vars.right_flipper_change_counter;
		g_vars.ball_phys_flipper_frame_num = g_vars.right_flipper_frame_num;
		n0 = g_vars.ball_xpos2;
		if (n0 < 154) {
			n0 = 154;
		}
		if (n0 > 223) {
			n0 = 223;
		}
		n0 -= 154;
		n0 = -n0;
		n0 += 69;
		g_vars.ball_flipper_xoffset = n0;
		n1 = g_vars.right_flipper_frame_num - 1;
		if (n1 < 0) {
			n1 = 0;
		}
		n1 *= 4;
		n2 = 0;
		if (g_vars.is_ball_on_flipper_area) {
			n2 = 5 * 4;
		}
		n1 += n2;

		assert((n1 & 3) == 0 && n1 / sizeof(uint32_t) < 10);
		const uint8_t *ptr = Resource_GetFlipperCollisionData(g_vars.res, n1 / sizeof(uint32_t));

		n1 -= n2;
		const uint8_t *cur_frame = Resource_GetFlipperCollisionData(g_vars.res, n1 / sizeof(uint32_t));
		n0 *= 2;
		n2 = (int16_t)READ_BE_UINT16(ptr + n0) + 300;
		g_vars.flipper_contact_point_y1 = n2;
		n2 = (int16_t)READ_BE_UINT16(cur_frame + n0) + 300;
		g_vars.flipper_contact_point_y2 = n2;
		n0 += 560;
		n2 = (int16_t)READ_BE_UINT16(ptr + n0) - 180;
		n1 = 180 - n2;
		if (n1 < 0) {
			n1 += 720;
		}
		g_vars.ball_flipper_collision_angle = n1;
		n0 += 560;
		g_vars.flipper_angular_velocity_factor = (int16_t)READ_BE_UINT16(ptr + n0);
	} else {

		g_vars.ball_near_flipper_side = 3;
		g_vars.ball_flipper_collision_state = (g_vars.ball_flipper_collision_state & 0xFF00) | g_vars.left_flipper_state;
		g_vars.ball_flipper_activation_timer = g_vars.left_flipper_change_counter;
		g_vars.ball_phys_flipper_frame_num = g_vars.left_flipper_frame_num;
		if (n0 < 73) {
			n0 = 73;
		}
		if (n0 > 142) {
			n0 = 142;
		}
		n0 -= 73;
		g_vars.ball_flipper_xoffset = n0;
		n1 = g_vars.left_flipper_frame_num - 1;
		if (n1 < 0) {
			n1 = 0;
		}
		n1 *= 4;
		n2 = 0;
		if (g_vars.is_ball_on_flipper_area) {
			n2 = 5 * 4;
		}
		n1 += n2;

		assert((n1 & 3) == 0 && n1 / sizeof(uint32_t) < 10);
		const uint8_t *ptr = Resource_GetFlipperCollisionData(g_vars.res, n1 / sizeof(uint32_t));

		n1 -= n2;
		const uint8_t *cur_frame = Resource_GetFlipperCollisionData(g_vars.res, n1 / sizeof(uint32_t));
		n0 *= 2;
		n2 = (int16_t)READ_BE_UINT16(ptr + n0) + 300;
		g_vars.flipper_contact_point_y1 = n2;
		n2 = (int16_t)READ_BE_UINT16(cur_frame + n0) + 300;
		g_vars.flipper_contact_point_y2 = n2;
		n0 += 560;
		g_vars.ball_flipper_collision_angle = (int16_t)READ_BE_UINT16(ptr + n0);
		n0 += 560;
		g_vars.flipper_angular_velocity_factor = (int16_t)READ_BE_UINT16(ptr + n0);
	}

	g_vars.was_ball_on_flipper_area = g_vars.is_ball_on_flipper_area;
	g_vars.is_ball_on_flipper_area = 0;
	n2 = g_vars.flipper_contact_point_y2 + 20;
	if (g_vars.was_ball_on_flipper_area) {
		n2 -= 8;
	}
	if (n2 <= g_vars.ball_ypos2) {
		g_vars.is_ball_on_flipper_area = 1;
	}
	g_vars.flipper_impulse_strength = 0;
	if (g_vars.light_pearl_letter == 2) {
		g_vars.light_pearl_letter = 0;
		g_vars.ball_layer = LAYER_OVERHEAD;
		g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res);
	}
	if (g_vars.ball_exit_locked_counter != 0) {
		--g_vars.ball_exit_locked_counter;
		if (g_vars.ball_exit_locked_counter != 1) {
			return;
		}

		if (g_vars.light_pearl_letter) {
			g_vars.light_pearl_letter = 2;
		}
		g_vars.ball_xpos2 = g_vars.ball_exit_locked_xpos;
		g_vars.ball_ypos2 = g_vars.ball_exit_locked_ypos;
		g_vars.ball_xvel = g_vars.ball_exit_locked_xvel;
		g_vars.ball_yvel = g_vars.ball_exit_locked_yvel;
		g_vars.ball_exit_locked_counter = 0;
		g_vars.ball_exit_cooldown = 15;
	}

	--g_vars.ball_exit_cooldown;
	if (g_vars.ball_exit_cooldown < 0) {
		g_vars.ball_exit_cooldown = 0;
	}
	if (g_vars.ball_out_of_play) {
		g_vars.tilt_block_until_launch = 1;
		return;
	}

	if (g_vars.is_ball_pushed_on_spring) {

		g_vars.is_ball_pushed_on_spring = 0;
		g_vars.tilt_block_until_launch = 0;
		g_vars.game_over_tilt = 0;
		g_vars.music_order_num = 46;
		g_vars.ball_yvel = -110;
		g_vars.ball_trapped = 1;
		g_vars.spring_release_flag = 0;
		g_vars.tilt_launch_cooldown = 30;
		g_vars.tilt_activation_flag = 0;
		if (g_vars.bonus_multiplier == 1) {
			light_set0(0x54);
		}

		n0 = (g_vars.random_num % 3) * 2 + 0x34;
		g_vars.light_sea_letter = n0;
		light_set0(n0);

	} else if (g_vars.spring_plunger_released) {
		g_vars.spring_plunger_released = 0;
		g_vars.ball_trapped = 0;
		g_vars.spring_release_flag = 1;
	}

	retry = 8;

ball_update_retry:
		n0 = 0;
		g_vars.collision_retry_counter = 0;
		g_vars.ball_collision_occurred = 0;
		g_vars.flipper_impulse_already_applied = 0;
		if (g_vars.spring_release_flag) {
			g_vars.ball_trapped = n0;
		}
		if (g_vars.is_ball_on_path == 0) {
			goto apply_gravity;
		}

		n2 = g_vars.path_traversal_progress;
		n0 = g_vars.bounce_energy;
		if (g_vars.path_diagonal_move) {
			const int x = (n0 << 5) / 45;
			n0 = clip_s16(x);
		}

		n2 += n0;
		g_vars.path_traversal_progress = n2;
		if (n2 < 256) {
			goto on_path_move;
		}
		n2 &= 0xFF00;
		g_vars.path_traversal_progress -= n2;
		n2 >>= 8;
		--n2;

		ptr = g_vars.ball_path_data_ptr;

do {
		n0 = 0;
		n1 = 0;
		n0 = *ptr++;
		if (g_vars.is_ball_path_reversed) {
			ptr -= 2;
			n0 = *ptr;
		}

	if (n0 == 2) {
			n0 = -(g_vars.bounce_energy / 3);
			g_vars.ball_yvel = n0;
			g_vars.ball_exit_cooldown = 60;
			g_vars.ball_layer = LAYER_GROUND;
			g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET;
			goto exit_ball_path;
	} else if (n0 == 7) {
		g_vars.ball_layer = LAYER_GROUND;
		g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET;
		if (g_vars.table_transition_flag) {
			g_vars.table_transition_flag = 0;
			g_vars.bonus_activation_flags[0] = 1;
			if (g_vars.game_over_tilt == 0) {
				g_vars.bonus_activation_flags[1] = 1;
			}

			g_vars.bonus_activation_flags[5] = 1;
			light_set2(0x4a);
			light_set2(0x62);
			light_set2(0x3a);
			light_set2(0x40);
			light_set2(0x42);
			n1 = -g_vars.bounce_energy;
			g_vars.ball_xvel = n1;
			g_vars.ball_yvel = g_vars.random_num & 0x1f;

		} else {

			g_vars.ball_yvel = g_vars.bounce_energy;
			g_vars.ball_xvel = 0;
			if (g_vars.is_ball_path_reversed) {
				g_vars.ball_layer = LAYER_OVERHEAD;
				g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res);
				g_vars.ball_yvel = -g_vars.ball_yvel;
				g_vars.ball_ypos2 -= 3;
				g_vars.is_ball_path_reversed = 0;
			}
		}

exit_ball_path:
		g_vars.is_ball_on_path = 0;
		g_vars.draw_ball_masked_flag = 0;
		g_vars.path_traversal_progress = 0;
		g_vars.ball_xpos1 = g_vars.ball_xpos2;
		g_vars.ball_ypos1 = g_vars.ball_ypos2;
		g_vars.ball_xpos3 = g_vars.ball_xpos2;
		g_vars.ball_ypos3 = g_vars.ball_ypos2;
		g_vars.ball_xvel_prev = g_vars.ball_xvel;
		g_vars.ball_yvel_prev = g_vars.ball_yvel;
		if (--retry != -1) goto ball_update_retry;
		return;

	} else if (n0 == 3) {
		g_vars.is_ball_path_reversed ^= 1;
		ptr = Resource_GetBallPath(g_vars.res, 7) + 0xC5;
		g_vars.bounce_energy = 430;
	}
	assert(n0 >= 0 && n0 < 256);
	n1 = n0;
	n0 >>= 4;
	n1 &= 15;
	if (n0 == 15) {
		n0 = -1;
	}
	if (n1 == 15) {
		n1 = -1;
	}
	if (g_vars.is_ball_path_reversed) {
		n0 = -n0;
		n1 = -n1;
	}
	g_vars.ball_xpos2 += n0;
	g_vars.ball_ypos2 += n1;
	g_vars.path_diagonal_move = 0;
	if (n0 != 0 && n1 != 0) {
		g_vars.path_diagonal_move = 1;
	}
	g_vars.path_moving_upward = 0;
	if (n1 < 0) {
		g_vars.path_moving_upward = 1;
	}
} while (--n2 != -1);

	g_vars.ball_path_data_ptr = ptr;

	if (n1 != 0) {
		g_vars.gravity_accumulator += 180;
		if (g_vars.gravity_accumulator < 256) {
			goto on_path_move;
		}
		n0 = g_vars.gravity_accumulator >> 8;
		if (g_vars.path_moving_upward != 0) {
			n1 = n0 >> 1;
			g_vars.bounce_energy -= n1;
			if (g_vars.bounce_energy < 5) {
				g_vars.bounce_energy = 5;
			}
		} else {
			g_vars.bounce_energy += n0;
		}

		if (g_vars.is_ball_path_reversed != 0) {
			g_vars.bounce_energy -= n0;
		}
		n0 <<= 8;
		g_vars.gravity_accumulator -= n0;
	}

	++g_vars.ball_friction_timer;
	if (g_vars.ball_friction_timer >= 15) {
		g_vars.ball_friction_timer = 0;
		n0 = g_vars.bounce_energy;
		n0 >>= 5;
		g_vars.bounce_energy -= n0;
	}
on_path_move:
	if (g_vars.bounce_energy > 511) {
		g_vars.bounce_energy = 511;
	}
	g_vars.ball_xpos3 = g_vars.ball_xpos2;
	g_vars.ball_ypos3 = g_vars.ball_ypos2;
	g_vars.ball_xvel_prev = g_vars.ball_xvel;
	g_vars.ball_yvel_prev = g_vars.ball_yvel;

	if (--retry != -1) goto ball_update_retry;
	return;

apply_gravity:

	g_vars.gravity_accumulator += 180;
	n0 = g_vars.vertical_impulse_buffer;
	g_vars.gravity_accumulator += n0;
	g_vars.vertical_impulse_buffer = 0;
	n0 = g_vars.ball_yvel;
	if (n0 < 0) {
		n0 = -n0;
		n0 <<= 2;
	}
	n0 >>= 2;
	n1 = 200;
	n2 = 120;
	if (g_vars.ball_xpos2 >= 308) {
		n1 = 66;
		n2 = 126;
	}
	if (n0 >= n1) {
		n0 -= n1;
		if (n0 >= n2) {
			n0 = n2;
		}
		g_vars.gravity_accumulator -= n0;
	}

	if (g_vars.gravity_accumulator >= 256) {
		n0 = g_vars.gravity_accumulator >> 8;
		g_vars.ball_yvel += n0;
		n0 <<= 8;
		g_vars.gravity_accumulator -= n0;
	}

	++g_vars.ball_friction_timer;
	if (g_vars.ball_friction_timer >= 15) {
		g_vars.ball_friction_timer = 0;
		n0 = g_vars.ball_xvel / 38;
		g_vars.ball_xvel -= n0;
		n0 = g_vars.ball_yvel / 30;
		g_vars.ball_yvel -= n0;
	}
	const int _ball_prev_xpos = g_vars.ball_xpos2;
	const int _ball_prev_ypos = g_vars.ball_ypos2;

	const int mat_pos = (g_vars.ball_xpos2 >> 1) + (g_vars.ball_ypos2 >> 1) * 168;
	ptr = g_vars.phys_mat_cur + mat_pos;
	n3 = 0;
	n5 = 0;
	int16_t *match_out = g_vars.collision_match_list;
	n1 = 0x20;

	static const uint16_t T[] = { 0x1FF, 0x157, 0xAE, 5, 4, 3, 2, 0xA9, 0x150, 0x1F8, 0x2A0, 0x348, 0x3F1, 0x49A, 0x49B, 0x49C, 0x49D, 0x3F6, 0x34F, 0x2A7 };
	for (int i = 0; i < 20; ++i) {
		assert(ball_offsets_table[i] == T[i]);
		if (!(n1 > ptr[T[i]])) {
			*match_out++ = i * 2;
			++n3;
		}
	}
	g_vars.ball_collision_type = 0;
	n0 = ptr[0x1FB];

	if (n0 == 0) {
		g_vars.ball_layer = LAYER_GROUND;
		g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET;
	} else if (n0 < 32) {
		g_vars.ball_collision_type = (g_vars.ball_collision_type & 0xFF00) | n0;
		if (n0 == 31) {
			g_vars.ball_layer = LAYER_OVERHEAD;
			g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res);
		}
	}

	struct {
		int16_t n0, n1, n2, n3;
		const uint8_t *ptr;
	} temp;
	temp.n0 = n0;
	temp.n1 = n1;
	temp.n2 = n2;
	temp.n3 = n3;
	temp.ptr = ptr;
	if (n0 != 31 && g_vars.ball_collision_type != 0) {
		goto handle_collision;
	}
	g_vars.collision_processed_flags = 0;

process_collision:
		n0 = temp.n0;
		n1 = temp.n1;
		n2 = temp.n2;
		n3 = temp.n3;
		ptr = temp.ptr;
		if (n3 == 0) {
			goto no_collision;
		}

		{
			const int16_t *offsets = ball_offsets_table;
			const int16_t *matches = g_vars.collision_match_list;
			n3 &= ~1;
			n5 = matches[n3 / sizeof(int16_t)];
			assert((n5 & 1) == 0 && n5 < 20 * sizeof(int16_t));
			n1 = offsets[n5 / sizeof(int16_t)];
		}
		n4 = 0;
		n5 = 0;
		n4 = ptr[n1];
		n0 = n4;
		n4 &= 0x1F;
		n0 &= 0xE0;
		n0 >>= 5;
		if (n0 != 0 && g_vars.game_over_tilt != 0) {
			n0 = 1;
		}

		g_vars.phys_map_type = n0;
		if (n0 == 2) {

			g_vars.ball_collision_flags |= (1 << 2);
			goto no_collision;
		} else if (n0 == 5) {

			if (g_vars.ball_exit_cooldown != 0) {
				goto no_collision;
			}
			n1 = ABS(g_vars.ball_xvel);
			n2 = ABS(g_vars.ball_yvel);
			n0 = g_vars.ball_xpos2 >> 1;
			if (n0 >= 50) {
				if (n0 >= 100) {
					if (n2 < 20 || g_vars.ball_hit_cooldown != 0) {
						goto collision_response;
					}
					g_vars.ball_hit_cooldown = 6;
					g_vars.ball_collision_flags |= (1 << 5);
					g_vars.bonus_data[2] = 1;
					n0 -= 132;
					if (n0 < 0) {
						n0 = 0;
					} else if (n0 > 8) {
						n0 = 8;
					}
					static const uint8_t _right_dive_xpos_bonus_lookup[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1 };
					n0 = _right_dive_xpos_bonus_lookup[n0];
					g_vars.bonus_data[6] |= (1 << n0);
					goto collision_response;
				} else {

					if (n2 < 20 || g_vars.ball_hit_cooldown != 0) {
						goto collision_response;
					}
					g_vars.ball_hit_cooldown = 6;
					g_vars.ball_collision_flags |= (1 << 5);
					g_vars.bonus_data[2] = 1;
					g_vars.bonus_data[5] |= (1 << 3);
					goto collision_response;
				}
			} else {

				if (n1 < 20 || g_vars.ball_hit_cooldown != 0) {
					goto collision_response;
				}
				g_vars.ball_hit_cooldown = 6;
				g_vars.ball_collision_flags |= (1 << 5);
				g_vars.bonus_data[2] = 1;
				n0 = (g_vars.ball_ypos2 >> 1) - 80;
				if (n0 < 0) {
					n0 = 0;
				} else if (n0 > 16) {
					n0 = 16;
				}
				static const uint8_t _left_dive_ypos_bonus_lookup[] = { 1, 1, 1, 3, 3, 3, 3, 3, 2, 2, 2, 6, 6, 6, 6, 6, 4 };
				n0 = _left_dive_ypos_bonus_lookup[n0];
				g_vars.bonus_data[5] |= (1 << n0);
				goto collision_response;
			}

		} else if (n0 == 6) {
			g_vars.ball_collision_flags |= (1 << 6);
			++g_vars.flipper_hit_counter;
			if (g_vars.flipper_hit_counter > 1) {
				goto collision_response;
			}
			n2 = g_vars.was_ball_on_flipper_area;
			if (n2 != g_vars.ball_flipper_collision_state) {
				goto collision_response;
			}
			g_vars.is_ball_on_flipper_area = g_vars.was_ball_on_flipper_area;
			n1 = g_vars.ball_flipper_xoffset - 18;
			if (n1 < 0) {
				n1 = 0;
			}
			if (g_vars.ball_flipper_collision_state == 0) {
				n1 = -n1;
			}
			if (g_vars.ball_near_flipper_side != 4) {
				if (g_vars.left_flipper_change_counter < 1) {
					goto collision_response;
				}
				g_vars.flipper_collision_type = 3;
				n4 = g_vars.ball_flipper_collision_angle;
			} else {
				if (g_vars.right_flipper_change_counter < 1) {
					goto collision_response;
				}
				g_vars.flipper_collision_type = 4;
				n4 = g_vars.ball_flipper_collision_angle;
			}
			{
				const int x = (n1 * 24 * g_vars.flipper_angular_velocity_factor) >> 7;
				n1 = clip_s16(x >> 7);
				if (n1 == 0) {
					n1 = 1;
				}
			}

			g_vars.flipper_impulse_strength = n1;
			g_vars.flipper_yvel = g_vars.ball_yvel;
			g_vars.flipper_xvel = g_vars.ball_xvel;
			goto collision_response;
		} else if (n0 == 7) {

			g_vars.ball_collision_flags |= (1 << 7);
			goto collision_response;
		} else {
			goto collision_response;
		}

handle_collision:
		g_vars.ball_collision_flags |= 1;
		n0 = g_vars.ball_collision_type;
		n6 = n0;
		--n0;
		assert(n0 >= 0 && n0 < 30);

		n1 = g_vars.ball_collision_table[n0].behaviour_type;
		g_vars.collision_response_id = g_vars.ball_collision_table[n0].param1;
		if (n1 == 1) {

			if (n6 < 4) {
				g_vars.bonus_data[0x9] = 1;
			} else if (n6 < 7) {
				g_vars.bonus_data[0xA] = 1;
			} else if (n6 < 10) {
				g_vars.bonus_data[0xC] = 1;
			}

			if (n6 == 24) {
				g_vars.bonus_data[0xA] = 0;
				if (g_vars.ball_yvel > -70) {
					goto process_collision;
				}
				g_vars.table_transition_flag = 1;
				if (g_vars.ball_yvel > -110) {
					g_vars.ball_yvel = -110;
				}
				g_vars.ball_xvel = 0;
				g_vars.tilt_activation_flag = 0;
				g_vars.music_order_num = 0;
				n4 = g_vars.ball_collision_table[n0].param1;
			}
			goto resolve_collision;
		} else if (n1 == 2 ) {

			g_vars.ball_xpos2 -= 2;
			g_vars.light_pearl_letter = 1;
			if (g_vars.pearl_lane_state != 0) {
				g_vars.pearl_lane_state = 0;
				g_vars.bonus_data[0x10] = 1;
			}
			g_vars.ball_exit_locked_counter = g_vars.ball_collision_table[n0].param1;

			n2 = (int8_t)(g_vars.ball_collision_table[n0].param2 >> 8);
			n2 += n2;
			g_vars.ball_exit_locked_xpos = n2;
			n2 = (int8_t)(g_vars.ball_collision_table[n0].param2 & 255);
			n2 += n2;
			g_vars.ball_exit_locked_ypos = n2;

			n2 = (int8_t)(g_vars.ball_collision_table[n0].param3 >> 8);
			n2 += n2;
			g_vars.ball_exit_locked_xvel = n2;
			n2 = (int8_t)(g_vars.ball_collision_table[n0].param3 & 255);
			n2 += n2;
			g_vars.ball_exit_locked_yvel = n2;

			if (g_vars.random_num & 1) {

				n2 = (int8_t)(g_vars.ball_collision_table[n0].param4 >> 8);
				n2 += n2;
				g_vars.ball_exit_locked_xpos = n2;
				n2 = (int8_t)(g_vars.ball_collision_table[n0].param4 & 255);
				n2 += n2;
				g_vars.ball_exit_locked_ypos = n2;

				n2 = (int8_t)(g_vars.ball_collision_table[n0].param5 >> 8);
				n2 += n2;
				g_vars.ball_exit_locked_xvel = n2;
				n2 = (int8_t)(g_vars.ball_collision_table[n0].param5 & 255);
				n2 += n2;
				g_vars.ball_exit_locked_yvel = n2;
			}

			g_vars.bonus_data[BONUS_PEARL] = 1;
			goto no_collision;

		} else if (n1 == 4 ) {

			g_vars.ball_xpos2 = 56;
			g_vars.ball_ypos2 = 60;
			g_vars.ball_exit_locked_counter = g_vars.ball_collision_table[n0].param1;
			g_vars.ball_exit_locked_xpos = g_vars.ball_collision_table[n0].param2;
			g_vars.ball_exit_locked_ypos = g_vars.ball_collision_table[n0].param3;
			g_vars.ball_exit_locked_xvel = g_vars.ball_collision_table[n0].param4;
			g_vars.ball_exit_locked_yvel = g_vars.ball_collision_table[n0].param5;
			g_vars.bonus_data[BONUS_STARFISH_BALL_TRAP] = 1;
			goto no_collision;

		} else if (n1 == 5) {

			patch_phys_map_close();
			g_vars.ball_xpos2 = 285;
			g_vars.ball_ypos2 = 210;
			g_vars.ball_exit_locked_counter = g_vars.ball_collision_table[n0].param1;
			g_vars.ball_exit_locked_xpos = g_vars.ball_collision_table[n0].param2;
			g_vars.ball_exit_locked_ypos = g_vars.ball_collision_table[n0].param3;
			g_vars.ball_exit_locked_xvel = g_vars.ball_collision_table[n0].param4;
			g_vars.ball_exit_locked_yvel = g_vars.ball_collision_table[n0].param5;
			g_vars.bonus_data[0x13] = 1;
			goto no_collision;

		} else if (n1 == 6) {

			if (n6 == 11) {
				if (g_vars.ball_yvel >= 0) {
					if (g_vars.left_ramp_state != 0) {
						g_vars.left_ramp_state = 0;
						g_vars.bonus_data[0x11] = 3;
					}
				} else {
					g_vars.left_ramp_state = 1;
					g_vars.bonus_data[0x11] = 2;
				}
			} else if (n6 == 13) {

				if (g_vars.ball_yvel >= 0) {
					if (g_vars.pearl_lane_state != 0) {
						g_vars.pearl_lane_state = 0;
						g_vars.bonus_data[0x10] = 3;
					}
				} else {
					g_vars.pearl_lane_state = 1;
					g_vars.bonus_data[0x10] = 2;
				}
			} else if (n6 == 25) {

				if (g_vars.ball_yvel >= 0) {
					if (g_vars.center_ramp_state != 0) {
						g_vars.center_ramp_state = 0;
						g_vars.bonus_data[8] = 3;
					}
				} else {
					g_vars.center_ramp_state = 1;
					g_vars.bonus_data[8] = 2;
				}
			} else if (n6 == 27) {

				if (g_vars.ball_yvel >= 0) {
					if (g_vars.right_ramp_state != 0) {
						g_vars.right_ramp_state = 0;
						g_vars.bonus_data[0xB] = 3;
					}
				} else {
					g_vars.right_ramp_state = 1;
					g_vars.bonus_data[0xB] = 2;
				}
			}

			goto process_collision;

		} else if (n1 == 7) {

			if (n6 == 12 && g_vars.left_ramp_state) {
				g_vars.left_ramp_state = 0;
				g_vars.bonus_data[0x11] = 1;
				goto process_collision;
			} else if (n6 == 28 && g_vars.right_ramp_state) {
				g_vars.right_ramp_state = 0;
				g_vars.bonus_data[0xB] = 1;
				goto process_collision;
			} else if (n6 == 17 && g_vars.center_ramp_state) {
				g_vars.center_ramp_state = 0;
				g_vars.bonus_data[0x8] = 1;
				goto process_collision;
			}
		} else if (n1 == 8) {

			if (g_vars.ball_yvel < 0) {
				goto process_collision;
			}
			if (g_vars.ball_xpos2 <= 147) {
				g_vars.bonus_data[7] |= 1 << 0;
			} else {
				g_vars.bonus_data[7] |= 1 << 1;
			}
		}

		{
			n2 = g_vars.ball_collision_table[n0].param1;
			uint8_t *flags = g_vars.bonus_data;
			if (g_vars.ball_exit_cooldown <= 0) {

				if (n1 == 10 && ((1 << (n2 & 31)) & g_vars.collision_processed_flags) == 0) {
					g_vars.collision_processed_flags = (1 << (n2 & 31));
					n3 = g_vars.ball_collision_table[n0].param2;
					assert(n2 >= 0 && n2 < 24);
					if (n3 >= 0) {
						flags[n2] |= 1 << (n3 & 7);
					} else {
						flags[n2] = 1;
					}
					goto process_collision;
				} else if (n1 == 11) {
					flags[n2] = 1;
					n3 = g_vars.ball_collision_table[n0].param2;
					if (n3 < g_vars.ball_xpos2) {
						flags[n2] = 2;
					}
					goto process_collision;
				}
			}
		}

		if (n1 == 12) {
			g_vars.ball_xvel = 0;
			g_vars.ball_yvel = 0;
			g_vars.ball_layer = LAYER_GROUND;
			g_vars.phys_mat_cur = Resource_GetPhysMap(g_vars.res) + PHYSMAP_LAYER_OFFSET;
		}
		goto process_collision;

resolve_collision:
	n3 = 0;
	n5 = g_vars.ball_xvel;
	n6 = g_vars.ball_yvel;
	if (n5 < 0) {
		n3 |= 2;
		n5 = -n5;
	}
	if (n5 < 4) {
		n5 = 4;
	}
	if (n6 < 0) {
		n3 |= 4;
		n6 = -n6;
	}
	if (n6 < 4) {
		n6 = 4;
	}
	n0 = n5;
	n1 = n6;
	assert((n3 & 1) == 0);
	assert(n3 >= 0 && n3 < 8);
	n2 = _collision_angle_quadrants[n3 / sizeof(int16_t)];
	if (n3 == 4 || n3 == 2) {
		const int tmp = n1;
		n1 = n0;
		n0 = tmp;
	}

	{
		assert(n1 != 0);
		const int x = (n0 << 8) / n1;
		n0 = x;
	}
	if (n0 >= 12800) {
		n0 = 12799;
	}

	assert(n0 >= 0 && n0 < 12800);
	n0 = g_vars.angle_conversion_table[n0];
	assert(n0 >= 0 && n0 < 128);
	n0 += n0;
	n0 += n2;
	n3 = n0;
	n1 = n5;
	n2 = n6;

	{
		const int x = n1 * n1 + n2 * n2;
		n2 = clip_s16(x >> 6);
	}
	assert(n2 >= 0 && n2 < 8600);
	n1 = g_vars.bounce_energy_lookup_table[n2];
	assert(n1 >= 0 && n1 < 128);

	n1 <<= 3;
	g_vars.bounce_energy = n1;

	{
		n0 = (g_vars.collision_response_id - 1);
		n0 += n0;
		n0 += n0;
		assert(n0 >= 0 && n0 <= 20 * sizeof(int16_t));
		const int16_t *p = &_collision_angle_range_table[n0 / sizeof(int16_t)];
		if (n3 < p[0] || n3 > p[1]) {
			goto process_collision;
		}
	}

	{
		assert((n0 & 3) == 0);
		ptr = Resource_GetBallPath(g_vars.res, n0 / sizeof(uint32_t));
		ptr += 2;
		g_vars.is_ball_on_path = 1;
		if (g_vars.table_transition_flag == 0) {
			g_vars.draw_ball_masked_flag = 1;
		}
		g_vars.ball_xpos2 = (int16_t)READ_BE_UINT16(ptr) - 6;
		ptr += 2;
		g_vars.ball_ypos2 = (int16_t)READ_BE_UINT16(ptr) - 6;
		ptr += 2;
		++ptr;
	}
	if (g_vars.bounce_energy > 511) {
		g_vars.bounce_energy = 511;
	}
	n0 = g_vars.bounce_energy;
	n1 = n0 >> 3;
	n0 -= n1;
	if (n0 < 10) {
		n0 = 10;
	}
	g_vars.bounce_energy = n0;
	g_vars.ball_path_data_ptr = ptr;
	return;

no_collision:

	g_vars.ball_xpos1 = g_vars.ball_xpos2;
	g_vars.ball_ypos1 = g_vars.ball_ypos2;
	g_vars.physics_loop_counter += 1;
	goto clamp_velocities;

collision_response:
	if (g_vars.ball_xpos2 <= 308 && g_vars.ball_trapped != 0) {

		goto no_collision;
	}

	g_vars.ball_collision_occurred = 1;
	++g_vars.collision_retry_counter;
	if (g_vars.collision_retry_counter >= 2) {
		goto collision_recovery;
	}

	if (n4 >= 0 && n4 < 32) {
		n4 = g_vars.collision_response_angle_table[n4];
	} else {

	}

	g_vars.ball_angle = n4;
	n3 = 0;
	n5 = g_vars.ball_xvel;
	n6 = g_vars.ball_yvel;
	if (n5 < 0) {
		n3 |= 2;
		n5 = -n5;
	}
	if (n5 < 4) {
		n5 = 4;
	}
	if (n6 < 0) {
		n3 |= 4;
		n6 = -n6;
	}
	if (n6 < 4) {
		n6 = 4;
	}

	n0 = n5;
	n1 = n6;
	assert((n3 & 1) == 0);
	assert(n3 >= 0 && n3 < 8 * sizeof(int16_t));
	n2 = _collision_angle_quadrants[n3 / sizeof(int16_t)];
	if (n3 == 4 || n3 == 2) {
		const int tmp = n1;
		n1 = n0;
		n0 = tmp;
	}

	{
		assert(n1 != 0);
		const int x = (n0 << 8) / n1;
		n0 = x;
	}
	if (n0 >= 12800) {
		n0 = 12799;
	}

	assert(n0 >= 0 && n0 < 12800);
	n0 = g_vars.angle_conversion_table[n0];
	assert(n0 >= 0 && n0 < 128);
	n0 += n0;
	n0 += n2;
	g_vars.rebound_direction_flag = 0;
	n1 = n0 - g_vars.ball_angle;
	if (n1 < 0) {
		n1 += 720;
	}
	if (n1 < 180) {
		g_vars.rebound_direction_flag = 1;
		g_vars.ball_collision_flags &= ~(1 << 5);

	} else if (n1 > 540) {
		g_vars.rebound_direction_flag = 1;
		g_vars.ball_collision_flags &= ~(1 << 5);

	} else {

		n1 = n4 / 90;
		{
			assert(n1 >= 0 && n1 * 2 < 16);
			const int8_t *p = &_ball_directions_xy_table[n1 << 1];
			g_vars.ball_xpos2 += p[0];
			g_vars.ball_ypos2 += p[1];
		}

		n1 = n5;
		n2 = n6;
		{
			const int x = n1 * n1 + n2 * n2;
			n2 = clip_s16(x >> 6);
		}
		assert(n2 >= 0 && n2 < 8600);
		n1 = g_vars.bounce_energy_lookup_table[n2];
		assert(n1 >= 0 && n1 < 128);
		n1 <<= 3;
		g_vars.base_bounce_energy = n1;
		g_vars.bounce_energy = n1;
		n4 += n4;
		n4 -= n0;
		n4 -= 360;
		if (n4 < 0) {
			n4 += 720;
		}
		if (n4 < 0) {
			n4 += 720;
		}
		if (n4 >= 720) {
			n4 -= 720;
		}
		g_vars.calculated_rebound_angle = n4;
		n0 = n4 - g_vars.ball_angle;
		if (n0 < 0) {
			n0 += 720;
		}
		if (n0 >= 360) {
			n0 -= 720;
		}

		g_vars.impact_angle_offset = n0;
		assert((n0 & 1) == 0);
		assert(n0 / 2 + 180 >= 0 && n0 / 2 + 180 < 360);
		n2 = g_vars.cos[180 + n0 / sizeof(int16_t)];
		n5 = n2;
		n3 = ABS(g_vars.ball_yvel);
		n5 >>= 2;
		if (n3 >= 100) {
			n2 -= n5;
			if (n3 >= 170) {
				n2 -= n5;
			}
		}

		if ((g_vars.ball_collision_flags & (1 << 6)) != 0 && g_vars.flipper_collision_type) {
			n2 >>= 1;
		}
		n0 += n2;
		{
			const int x = (g_vars.bounce_energy * n0) >> 8;
			n2 = clip_s16(x) >> 1;
		}
		n3 = ABS(n2 - g_vars.previous_velocity_magnitude);

		g_vars.previous_velocity_magnitude = n2;
		n3 += 605;
		n2 = n0;
		if (n2 > 0) {
			n2 = -n2;
		}

		n2 += 180;
		n2 += n2;
		n3 += n2;
		{
			const int x = n1 * 600 / n3;
			assert(x >= 0);
			n1 = clip_s16(x);
		}
		n3 -= n2;
		n3 += 600;
		if (n0 >= 0) {
			n0 = -n0;
			n0 += 180;
			{
				const int x = n0 * 1200 / n3;
				n0 = clip_s16(x);
			}
			n0 = -n0;
			n0 += 180;
		} else {
			n0 = -n0;
			n0 -= 180;
			{
				const int x = n0 * 1200 / n3;
				n0 = clip_s16(x);
			}
			n0 = -n0;
			n0 -= 180;
		}

		n0 += g_vars.ball_angle;
		if (n0 < 0) {
			n0 += 720;
		} else if (n0 >= 720) {
			n0 -= 720;
		}
		n0 &= ~1;
		n4 = n0;
		n2 = g_vars.bounce_energy;
		if (g_vars.ball_ypos2 >= 300) {
			n2 = ABS(g_vars.ball_xvel);
		}

		if (n2 >= 20) {
			if (g_vars.phys_map_type == 3) {
				g_vars.ball_collision_flags |= (1 << 3);
				g_vars.bonus_data[0] = 1;
				{
					const int x = 3840 / (n1 + 1) + 20;
					const int y = (g_vars.impact_angle_offset << 5) / x + g_vars.ball_angle;
					n4 = clip_s16(y);
					if (n4 < 0) {
						n4 += 720;
					} else if (n4 >= 720) {
						n4 -= 720;
					}
					n4 &= ~1;
					g_vars.calculated_rebound_angle = n4;
				}
				if (n1 <= 240) {
					n1 += 120;
					if (n1 > 240) {
						n1 = 240;
					}
				}
			}

			if (g_vars.phys_map_type == 4) {
				if (ABS(g_vars.ball_xvel) >= 20) {
					g_vars.ball_collision_flags |= (1 << 4);
					g_vars.bonus_data[1] = 1;
					const int x = 6400 / (n1 + 1) + 20;
					const int y = (g_vars.impact_angle_offset << 5) / x + g_vars.ball_angle;
					n4 = clip_s16(y);
					if (n4 < 0) {
						n4 += 720;
					} else if (n4 >= 720) {
						n4 -= 720;
					}
					n4 &= ~1;
					g_vars.calculated_rebound_angle = n4;
					if (n1 <= 320) {
						n1 += 200;
						if (n1 > 320) {
							n1 = 320;
						}
					}
				}
			}
		}

		g_vars.bounce_energy = n1;
		if ((g_vars.ball_collision_flags & (1 << 7)) != 0) {
			n1 = g_vars.base_bounce_energy;
			n2 = n1;
			n1 -= g_vars.bounce_energy;
			if (g_vars.ball_ypos2 >= 475) {
				n1 >>= 1;
			} else {
				n3 = ABS(g_vars.impact_angle_offset);
				if (n3 >= 60) {
					n1 >>= 1;
					if (n3 >= 80) {
						n1 >>= 1;
						if (n3 >= 100) {
							n1 >>= 1;
						}
					}
				}
			}
			n2 -= n1;
			g_vars.bounce_energy = n2;
		}

apply_rebound:
		n1 = g_vars.bounce_energy;
		{
			const int16_t *ptr = g_vars.normals_dst_table;
			const int16_t *normals_hi = &ptr[90];
			assert((n4 & 1) == 0);
			assert(n4 >= 0 && (n4 / sizeof(int16_t) + 90) < 500);
			n2 =  normals_hi[n4 / sizeof(int16_t)];
			n3 = -ptr[n4 / sizeof(int16_t)];
			{
				const int x = (n2 * n1) >> 7;
				n2 = clip_s16(x >> 7);
				const int y = (n3 * n1) >> 7;
				n3 = clip_s16(y >> 7);
			}
			g_vars.ball_xvel = n2;
			g_vars.ball_yvel = n3;
		}
		n1 = ABS(n3);
		if (n1 < 50) {
			n0 = (g_vars.ball_yvel_prev - n3) >> 1;
			n2 = n3 + n0;
			g_vars.ball_yvel = n2;
		}
		if (g_vars.tilt_state != 0) {
			g_vars.tilt_detection_counter -= 4;
			g_vars.tilt_confirmation_counter -= 4;

			ptr = Resource_GetData(g_vars.res, RES_TILT_BALL, 0);
			if (g_vars.tilt_direction == 2) {
				ptr += 40 * sizeof(int16_t);
			} else if (g_vars.tilt_direction == 3) {
				ptr += 80 * sizeof(int16_t);
			}
			n4 = g_vars.ball_angle - 360;
			if (n4 < 0) {
				n4 += 720;
			}
			n4 /= 36;
			n4 += n4;
			n4 += n4;
			n3 = (int16_t)(READ_BE_UINT32(ptr + n4) & 0xFFFF);
			g_vars.ball_yvel += n3;
			n3 = g_vars.ball_xvel >> 3;
			g_vars.ball_xvel -= n3;
		}
		n3 = g_vars.flipper_impulse_strength;
		if (n3 != 0) {

			g_vars.flipper_impulse_already_applied = 1;
			apply_flipper_impulse(n3);
		}
		g_vars.ball_xvel_acc = 0;
		g_vars.ball_yvel_acc = 0;
	}

clamp_velocities:
	if (g_vars.ball_xvel >= 511) {
		g_vars.ball_xvel = 510;
	}
	if (g_vars.ball_xvel <= -511) {
		g_vars.ball_xvel = -510;
	}
	if (g_vars.ball_yvel >= 511) {
		g_vars.ball_yvel = 510;
	}
	if (g_vars.ball_yvel <= -511) {
		g_vars.ball_yvel = -510;
	}

	if (g_vars.ball_collision_occurred != 0) {
		g_vars.ball_xpos4 = g_vars.ball_xpos2;
		g_vars.ball_ypos4 = g_vars.ball_ypos2;
	}
	if (g_vars.ball_collision_type == BALL_IDLE_ON_FLIPPER) {

		n0 = ABS(g_vars.ball_xvel);
		if (n0 <= 80) {
			n1 = ABS(g_vars.ball_yvel);
			if (n1 <= 30) {
				if ((retry & 1) != 0) {
					n2 = g_vars.ball_xvel >> 4;
					g_vars.ball_xvel -= n2;
					if (g_vars.ball_xvel < 0) {
						g_vars.ball_xvel += 2;
					}
					--g_vars.ball_xvel;
					n2 = g_vars.ball_yvel >> 4;
					g_vars.ball_yvel -= n2;
					if (g_vars.ball_yvel < 0) {
						g_vars.ball_yvel += 2;
					}
					--g_vars.ball_yvel;
					if (n0 <= 7 && n1 <= 7 && g_vars.tilt_state == 0) {
						g_vars.ball_trapped = 1;
						g_vars.ball_xvel = 0;
						g_vars.ball_yvel = 0;
					}
				}
			}
		}
	}

	g_vars.ball_xvel_acc += g_vars.ball_xvel;
	n0 = g_vars.ball_xvel_acc;
	if (g_vars.ball_xvel_acc >= 256) {

		if (g_vars.tilt_direction == 3) {
			n0 += 50;
		}
		if (g_vars.tilt_direction == 2) {
			n0 -= 50;
		}
		n1 = n0;
		n0 >>= 8;
		if (n0 > 2) {
			n0 = 2;
		}
		assert(n0 >= 0);
		n1 &= 255;
		g_vars.ball_xvel_acc = n1;
		if (g_vars.ball_trapped == 0) {
			g_vars.ball_xpos2 += n0;
		}
	} else if (g_vars.ball_xvel_acc <= -256) {

		if (g_vars.tilt_direction == 3) {
			n0 += 50;
		}
		if (g_vars.tilt_direction == 2) {
			n0 -= 50;
		}
		const int d0_hi = n0 % 256;
		n0 /= 256;
		if (n0 < -2) {
			n0 = -2;
		}
		assert(n0 <= 0);
		if (g_vars.ball_trapped == 0) {
			g_vars.ball_xpos2 += n0;
		}
		g_vars.ball_xvel_acc = d0_hi;
	}

	if (g_vars.ball_xpos2 >= 308 && g_vars.ball_trapped && g_vars.ball_collision_occurred && g_vars.rebound_direction_flag == 0) {
		n0 = ABS(g_vars.ball_yvel);
		if (n0 <= 20) {
			n0 = g_vars.ball_yvel >> 3;
			g_vars.ball_yvel = n0;
			g_vars.ball_ypos2 = 443;
		}
	}

	if (g_vars.flipper_collision_type != 0 && g_vars.ball_flipper_activation_timer >= 2) {
		n0 = ABS(g_vars.flipper_yvel);
		n1 = ABS(g_vars.flipper_impulse_strength);
		n1 += n1;
		if (n0 <= n1) {
			if (g_vars.flipper_impulse_already_applied == 0) {
				n3 = g_vars.flipper_impulse_strength;
				if (n3 != 0) {
					apply_flipper_impulse(n3);
				}
			}

			n0 = g_vars.ball_flipper_xoffset;
			n1 = g_vars.flipper_contact_point_y1;
			n0 -= 14;
			if (n0 < 0) {
				n0 = 0;
			}
			n0 >>= 3;
			if (g_vars.ball_phys_flipper_frame_num == 2 || g_vars.ball_phys_flipper_frame_num == 3) {
				n0 >>= 1;
			}

			n1 -= n0;
			if (g_vars.was_ball_on_flipper_area != 0) {
				n1 += n0;
				n1 += n0;
			}
			g_vars.ball_ypos2 = n1;
			goto clamp_position;
		}
	}

	g_vars.ball_yvel_acc += g_vars.ball_yvel;
	n0 = g_vars.ball_yvel_acc;
	if (g_vars.ball_yvel_acc >= 256) {

		if (g_vars.tilt_state == 1) {
			n0 += 53;
		}
		n1 = n0;
		n0 >>= 8;
		if (n0 > 2) {
			n0 = 2;
		}
		assert(n0 >= 0);
		n1 &= 255;
		g_vars.ball_yvel_acc = n1;
		if (g_vars.ball_xpos2 <= 308 && g_vars.ball_trapped != 0) {
			n1 = ABS(g_vars.ball_yvel);
			if (n1 <= 5) {
				goto clamp_position;
			}
		}
		g_vars.ball_ypos2 += n0;
	} else if (g_vars.ball_yvel_acc <= -256) {

		if (g_vars.tilt_state == 1) {
			n0 += 53;
		}
		const int d0_hi = n0 % 256;
		assert(d0_hi <= 0);
		n0 /= 256;
		if (n0 < -2) {
			n0 = -2;
		}
		assert(n0 <= 0);
		if (g_vars.ball_trapped == 0) {
			g_vars.ball_ypos2 += n0;
		} else {
			n1 = ABS(g_vars.ball_yvel);
			if (n1 > 5) {
				g_vars.ball_ypos2 += n0;
			}
		}
		g_vars.ball_yvel_acc = d0_hi;
	}

clamp_position:
	if (g_vars.ball_xpos2 < 0) {
		g_vars.ball_xpos2 = 0;
	}
	if (g_vars.ball_xpos2 >= 320) {
		g_vars.ball_xpos2 = 320;
	}
	if (g_vars.ball_ypos2 < 0) {
		g_vars.ball_ypos2 = 0;
	}
	if (g_vars.ball_ypos2 >= 499) {
		g_vars.ball_ypos2 = 500;
		reset_vars();
	}
	if (g_vars.physics_recovery_flag) {
		g_vars.physics_recovery_flag = 0;
		return;
	}

	n0 = g_vars.ball_xpos2 - _ball_prev_xpos;
	n1 = g_vars.ball_ypos2 - _ball_prev_ypos;

	g_vars.ball_xpos3 = g_vars.ball_xpos2;
	g_vars.ball_ypos3 = g_vars.ball_ypos2;
	g_vars.ball_xvel_prev = g_vars.ball_xvel;
	g_vars.ball_yvel_prev = g_vars.ball_yvel;
	if (--retry != -1) goto ball_update_retry;

	if (g_vars.flipper_hit_counter < 9) {
		return;
	}

	{
		n0 = g_vars.ball_phys_flipper_frame_num * 4;
		const uint8_t *prev_frame = Resource_GetFlipperCollisionData(g_vars.res, n0 / sizeof(uint32_t));
		const uint8_t *next_frame = Resource_GetFlipperCollisionData(g_vars.res, (n0 + 20) / sizeof(uint32_t));
		n0 = g_vars.ball_flipper_xoffset * 2;
		n1 = (int16_t)READ_BE_UINT16(prev_frame + n0);
		n2 = (int16_t)READ_BE_UINT16(next_frame + n0);
		n3 = g_vars.ball_ypos2 - 300;
		n4 = n2 - n3;
		if (n4 >= 0) {
			n3 -= n1;
			if (n3 >= 0) {
				if (n3 >= n4) {
					n1 = n2;
				}
				n1 += 300;
				g_vars.ball_ypos2 = n1;
			}
		}
	}
	return;

collision_recovery:

	if (g_vars.phys_map_type != 5) {
		if (((g_vars.random_num >> 8) & (1 << 2)) == 0) {
			g_vars.ball_xpos2 = g_vars.ball_xpos1;
			g_vars.ball_ypos2 = g_vars.ball_ypos1;
		} else {
			const int x = (g_vars.ball_angle / 90) << 1;
			const int8_t *p = &_ball_directions_xy_table[x];
			g_vars.ball_xpos2 += (int16_t)p[0];
			g_vars.ball_ypos2 += (int16_t)p[1];
		}
	}

	g_vars.physics_recovery_flag = 1;
	n1 = g_vars.bounce_energy;
	n4 = g_vars.calculated_rebound_angle;
	goto apply_rebound;
}

void update_left_flipper() {
	if (g_vars.left_flipper_state == 0) {
		++g_vars.left_flipper_frame_num;
		if (g_vars.left_flipper_frame_num == 4) {
			g_vars.left_flipper_pushed = 0;
		}
	} else {
		--g_vars.left_flipper_frame_num;
		if (g_vars.left_flipper_frame_num == 0) {
			g_vars.left_flipper_pushed = 0;
		}
	}
}

void update_right_flipper() {
	if (g_vars.right_flipper_state == 0) {
		++g_vars.right_flipper_frame_num;
		if (g_vars.right_flipper_frame_num == 4) {
			g_vars.right_flipper_pushed = 0;
		}
	} else {
		--g_vars.right_flipper_frame_num;
		if (g_vars.right_flipper_frame_num == 0) {
			g_vars.right_flipper_pushed = 0;
		}
	}
}

