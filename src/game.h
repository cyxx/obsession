
#ifndef TABLE_H__
#define TABLE_H__

#include "intern.h"
#include "resource.h"

enum {
	AMIGA_KEY_Y      = 0x15,
	AMIGA_KEY_S      = 0x21,
	AMIGA_KEY_J      = 0x26,
	AMIGA_KEY_L      = 0x28,
	AMIGA_KEY_N      = 0x36,
	AMIGA_KEY_M      = 0x37,
	AMIGA_KEY_SPACE  = 0x40,
	AMIGA_KEY_ESCAPE = 0x45,
	AMIGA_KEY_DOWN   = 0x4d,
	AMIGA_KEY_LSHIFT = 0x64,
	AMIGA_KEY_RSHIFT = 0x65,
	AMIGA_KEY_LAMIGA = 0x66,
	AMIGA_KEY_RAMIGA = 0x67,
};

enum {
	GAME_STATE_ATTRACT_MODE,
	GAME_STATE_PLUNGER_READY,
	GAME_STATE_BALL_ACTIVE,
	GAME_STATE_TILT_OR_DRAIN,
	GAME_STATE_MATCH_OVER,
	GAME_STATE_CONFIRM_QUIT,
};

enum {
	LIGHT_STATE_OFF = 0,
	LIGHT_STATE_ON = 1,
	LIGHT_STATE_BLINKING = 2,
};

enum {
	LAYER_GROUND = 0,
	LAYER_OVERHEAD = 1,
};

enum {
	BCD_150 = 150,
	BCD_320 = 320,
	BCD_500 = 500,
	BCD_640 = 640,
	BCD_750 = 750,
	BCD_1_000 = 1000,
	BCD_1_280 = 1280,
	BCD_2_000 = 2000,
	BCD_3_000 = 3000,
	BCD_5_000 = 5000,
	BCD_9_000 = 9000,
	BCD_10_000 = 10000,
	BCD_20_000 = 20000,
	BCD_25_000 = 25000,
	BCD_30_000 = 30000,
	BCD_50_000 = 50000,
	BCD_75_000 = 75000,
	BCD_100_000 = 100000,
	BCD_150_000 = 150000,
	BCD_200_000 = 200000,
	BCD_250_000 = 250000,
	BCD_300_000 = 300000,
	BCD_400_000 = 400000,
	BCD_500_000 = 500000,
	BCD_600_000 = 600000,
	BCD_700_000 = 700000,
	BCD_750_000 = 750000,
	BCD_800_000 = 800000,
	BCD_900_000 = 900000,
	BCD_1_000_000 = 1000000,
	BCD_1_500_000 = 1500000,
	BCD_2_000_000 = 2000000,
	BCD_2_500_000 = 2500000,
	BCD_3_000_000 = 3000000,
	BCD_4_000_000 = 4000000,
	BCD_5_000_000 = 5000000,
	BCD_6_000_000 = 6000000,
	BCD_7_000_000 = 7000000,
	BCD_8_000_000 = 8000000,
	BCD_10_000_000 = 10000000,
};

enum {
	LED_MODE_0_DISPLAYING_PLAYER_SCORE = 0,
	LED_MODE_1_SCROLLING_TEXT = 1,
	LED_MODE_2_DISPLAYING_BONUS_SCORE = 2,
	LED_MODE_3_BLANKING = 3,
	LED_MODE_4_PLAYING_ANIMATION = 4,
	LED_MODE_5_HIGH_SCORE = 5,
};

enum {
	BONUS_SEA = 4,
	BONUS_DIVE = 5,
	BONUS_STARFISH_BALL_TRAP = 13,
	BONUS_PEARL = 14,
};

enum {
	BALLS_COUNT = 5,
};

enum {
	BALL_COLLISION_NORMAL = 0,
	BALL_COLLISION_BUMPER = 1,
	BALL_COLLISION_FLIPPER = 6,
	BALL_COLLISION_SPRING = 7,
	BALL_IDLE_ON_FLIPPER = 19
};

enum {
	MAX_BALL_VELOCITY = 511,
	PATH_FOLLOW_ITERATIONS = 8,
	HINT_DISPLAY_THRESHOLD = 100,
	BALL_LOCK_VELOCITY_THRESHOLD = 7
};

struct vars_t {
	uint8_t keys[0x80];
	int16_t last_key_code;
	int16_t esc_pressed_flag;
	bool key_M_pressed;
	bool key_J_pressed;
	bool key_S_pressed;
	int16_t game_over_flag;
	int16_t random_num;

	const uint8_t *vscroll_table;
	int16_t vscroll;
	int16_t prev_vscroll;

	int16_t f_keys_enabled_flag;
	uint8_t f_keys_disabled;
	int16_t draw_ball_flag;
	int16_t is_screen_scrolling_following_ball;
	int16_t current_player;

	int16_t ball_xpos1;
	int16_t ball_ypos1;
	int16_t ball_xpos2;
	int16_t ball_ypos2;
	int16_t ball_xpos3;
	int16_t ball_ypos3;
	int16_t ball_xpos4;
	int16_t ball_ypos4;
	int16_t ball_angle;
	int16_t ball_layer;

	int16_t tilt_direction;
	int16_t tilt_state;
	int16_t tilt_table_middle;
	int16_t tilt_table_left;
	int16_t tilt_table_right;
	int16_t tilt_screen_index;
	int16_t tilt_block_until_launch;
	int16_t tilt_launch_cooldown;
	int16_t tilt_detection_counter;
	int16_t tilt_confirmation_counter;

	int16_t ball_out_of_play;
	int16_t reset_tilt;
	int16_t ball_xvel;
	int16_t ball_xvel_prev;
	int16_t ball_xvel_acc;
	int16_t ball_yvel;
	int16_t ball_yvel_prev;
	int16_t ball_yvel_acc;

	int16_t state_counter;
	int16_t state_num;

	int16_t sound_num;
	int16_t sound_volume;
	int16_t music_order_num;
	int16_t jingle_order_num;
	int16_t game_over_tilt;
	uint8_t left_flipper_state;
	uint8_t left_flipper_pushed;
	uint8_t left_flipper_on;
	int16_t left_flipper_frame_num;
	uint8_t right_flipper_state;
	uint8_t right_flipper_pushed;
	uint8_t right_flipper_on;
	int16_t right_flipper_frame_num;

	uint16_t lights_queue[128];
	uint16_t lights_queue_wr_index;
	uint16_t lights_queue_rd_index;

	uint16_t lights_blinking_list_index;
	uint16_t lights_blinking_list[34];
	uint8_t lights_state_saved[64];
	uint8_t lights_state_table[64];
	uint8_t lights_blink_phase;

	int16_t spring_force;
	int16_t spring_super_charge_flag;
	int16_t spring_down_flag;
	int16_t spring_pull_initiated;

	uint8_t led_screen[(TABLE_W / 8) * 15];
	uint8_t led_temp_bitmap[(TABLE_W / 8) * 15];
	const uint8_t *led_panel_text;
	const uint8_t *led_panel_text_ptr;
	int16_t led_panel_text_index;
	uint8_t led_panel_text_speed;
	uint8_t led_panel_text_counter;
	const uint8_t *led_animation_ptr;
	int16_t led_panel_mode;
	int16_t led_panel_mode_state;
	int16_t led_panel_animation_style_flag;
	int16_t led_change_color_flag;
	int16_t led_unused_flag;
	int16_t led_panel_counter;
	int16_t led_animation_counter;
	const uint8_t *led_animation_current_ptr;
	int16_t led_animation_size;
	int16_t led_color_index;
	uint16_t led_color_rgb444;
	uint16_t led_color_blinking_speed;
	int16_t led_panel_text_loop_flag;
	const uint8_t *led_panel_text_letter_mask;
	int16_t led_quit_flag;
	int16_t led_blink_counter;

	int16_t hall_of_fame;
	int16_t *cos;
	uint8_t *phys_mat_cur;
	const uint8_t *angle_conversion_table;
	uint8_t bounce_energy_lookup_table[8600];
	int16_t collision_response_angle_table[32];
	uint8_t *normals_src_table;
	int16_t normals_dst_table[500];
	uint16_t ball_collision_type;
	const struct collide_t *ball_collision_table;
	int16_t light_pearl_letter;
	int16_t light_sea_letter;
	int16_t flipper_contact_point_y1;
	int16_t flipper_contact_point_y2;
	int16_t ball_flipper_collision_angle;
	int16_t ball_phys_flipper_frame_num;
	int16_t flipper_angular_velocity_factor;
	int16_t ball_flipper_collision_state;
	int16_t ball_flipper_activation_timer;
	int16_t flipper_yvel;
	int16_t flipper_xvel;
	int16_t score_multiplier;
	int16_t led_bonus_shown;
	int16_t base_bounce_energy;
	int16_t left_flipper_change_counter;
	int16_t right_flipper_change_counter;
	int16_t flipper_impulse_strength;
	int16_t is_ball_on_flipper_area;
	int16_t was_ball_on_flipper_area;
	int16_t ball_collision_occurred;
	int16_t flipper_collision_type;
	uint8_t ball_collision_flags;
	int16_t flipper_hit_counter;
	int16_t collision_retry_counter;
	int16_t ball_hit_cooldown;
	int16_t collision_match_list[20];
	int16_t gravity_accumulator;
	int16_t vertical_impulse_buffer;
	int16_t phys_map_type;
	int16_t ball_friction_timer;
	uint16_t bounce_energy;
	int16_t calculated_rebound_angle;
	int16_t physics_recovery_flag;
	int16_t previous_velocity_magnitude;
	int16_t physics_loop_counter;
	int16_t is_ball_on_path;
	int16_t table_transition_flag;
	int16_t draw_ball_masked_flag;
	uint16_t path_traversal_progress;
	int16_t path_diagonal_move;
	int16_t path_moving_upward;
	int16_t flipper_impulse_already_applied;
	int16_t ball_near_flipper_side;
	const uint8_t *ball_path_data_ptr;
	int16_t collision_response_id;
	int16_t is_ball_path_reversed;
	int16_t ball_exit_locked_counter;
	int16_t ball_exit_locked_xpos;
	int16_t ball_exit_locked_ypos;
	int16_t ball_exit_locked_xvel;
	int16_t ball_exit_locked_yvel;
	int16_t impact_angle_offset;
	int16_t ball_flipper_xoffset;
	uint8_t bonus_data[24];
	uint32_t collision_processed_flags;
	int16_t left_ramp_state;
	int16_t pearl_lane_state;
	int16_t center_ramp_state;
	int16_t right_ramp_state;
	int16_t ball_exit_cooldown;
	int16_t spring_plunger_released;
	int16_t is_ball_pushed_on_spring;
	int16_t tilt_activation_flag;
	int16_t ball_trapped;
	int16_t hint_counter;
	int16_t spring_release_flag;
	int16_t rebound_direction_flag;

	struct {
		int16_t dive_mission_counter;
		int16_t special_num;
		int16_t special_timer;
		int16_t led_message_timer;
		int16_t underwater_progress;
		int16_t shrimp_attack_timer;
		int16_t starfish_bonus_timer;
		uint16_t dive_letters_mask;
		int16_t sea_s_letter_on;
		int16_t sea_e_letter_on;
		int16_t sea_a_letter_on;
		int16_t sea_lane_s_on;
		int16_t sea_lane_e_on;
		int16_t sea_lane_a_on;
		int16_t lane_a_lit;
		int16_t lane_b_lit;
		int16_t secret_passage_hits;
		int16_t pearl_letter_timer;
		int16_t pearl_bonus_timer;
		int16_t sea_letters_cooldown;
		int16_t extra_ball_lit_counter;
		int16_t lanes_complete_flag;
		int16_t extra_ball_flag1;
		int16_t bonus_increase_available;
		int16_t bonus_increase_completed;
		int16_t dive_mission_available;
		int16_t dive_mission_completed;
		int16_t million_point_bonus_flag;
		int16_t sea_score_index;
		int16_t special_bonus_data_num;
		int16_t safe_ball_lit;
		int16_t special_mode;
		int16_t bonus_multiplier_hold;
	} state;

	int16_t bonus_activation_flags[10];
	int16_t deep_ramp_counter;
	int16_t pearl_double_bonus;
	int16_t pearl_letter_count;
	int16_t shoot_again_flag;
	int16_t skill_shot_score_index;
	uint32_t score_bcd;
	uint32_t bonus_score1_bcd;
	uint32_t bonus_score2_bcd;
	int16_t bonus_multiplier;
	int16_t balls_count;
	int16_t special_next;
	int16_t kickback_enabled;

	uint32_t special_input_mask;

	bool in_attract_mode;
	bool attract_mode_vscroll_down;
	uint8_t gfx_lights[256];
	ResourcesData *res;
	uint8_t *offscreen;
};

extern struct vars_t g_vars;

extern void table_init();
extern void table_update();

extern int table_get_state();

extern void update_game_state();
extern void process_bonus();
void clear_mission_state();

extern void sound_init();
extern void sound_update();

void led_update();
void led_reset_text();

void light_set0(uint32_t light);
void light_set1(uint32_t light);
void light_set2(uint32_t light);
void light_set5(uint32_t light);
void light_set6(uint32_t light);
void light_set7(uint32_t light);
void lights_update_state();
void lights_draw();
void reset_lights_pearl_shell();
void reset_lights_dive();
void reset_lights_arrows();
void reset_lights_sea();
void reset_lights_starfish();
void reset_lights_hb();
void reset_lights_pearl();
void reset_lights_score_milestones();
void reset_lights_multipliers();

void patch_phys_map_open();
void patch_phys_map_close();
void update_physmap(int frame, const uint8_t *src, uint8_t *dst);
int16_t clip_s16(int16_t x);
void reset_vars();
void init_physics_lut();

void ball_update();
void update_left_flipper();
void update_right_flipper();

void add_u32(uint32_t *p, uint32_t x);
void score_bcd(uint32_t score);
void bonus_score1_bcd(uint32_t score);
void bonus_score2_bcd(uint32_t score);

void special_award_events(uint32_t bonus_active, uint32_t zero);
void dive_mission_150k_score(uint32_t bonus_active, uint32_t zero);
void dive_mission_250k_score(uint32_t bonus_active, uint32_t zero);
void dive_mission_500k_score(uint32_t bonus_active, uint32_t zero);
void dive_mission_pearl_500k(uint32_t bonus_active, uint32_t zero);
void sea_lane_scoring(uint32_t bonus_active, uint32_t zero);

void do_key_M_pressed();
void do_key_M_released();
void do_key_J_pressed();
void do_key_J_released();
void do_key_S_pressed();
void do_key_S_released();
void do_key_ALT_LEFT_pressed();
void do_key_ALT_LEFT_released();
void do_key_ALT_RIGHT_pressed();
void do_key_ALT_RIGHT_released();
void do_key_SPACE_pressed();
void do_key_ESC_pressed();
void do_key_DOWN_pressed();
void do_key_DOWN_released();
void do_key_AMIGA_LEFT_pressed();
void do_key_AMIGA_RIGHT_pressed();

#endif
