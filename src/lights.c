
#include "host.h"
#include "game.h"

static void Light_TurnOff(uint32_t light) {
	uint16_t lo = light & 0xFFFF;
	light = (light >> 16) | ((light & 0xFFFF) << 16);
	light = (light & 0xFFFF0000) | lo;
	uint16_t *list = g_vars.lights_blinking_list;
	for (int i = 0; i < 16; ++i) {
		if (list[i * 2] == (light >> 16) && list[i * 2 + 1] == (light & 0xFFFF)) {
			list[i * 2] = 0xFFFF;
			list[i * 2 + 1] = 0xFFFF;
			break;
		}
	}
}

void light_set0(uint32_t light) {
	assert((light & 1) == 0);
	g_host.Debug("light_set0 light:0x%x", light);
	const uint16_t id = light;
	uint8_t state = g_vars.lights_state_table[light >> 1];
	if (state == 2) {
		Light_TurnOff(id);
	}

	light = (id << 16) | ((id + 1) & 0xFFFF);

	int wr = g_vars.lights_queue_wr_index;
	g_vars.lights_queue[ wr           / sizeof(uint16_t)] = light >> 16;
	g_vars.lights_queue[(wr + 2) / sizeof(uint16_t)] = light & 0xFFFF;
	wr = (wr + 4) & 0xFF;
	g_vars.lights_queue_wr_index = wr;

	g_vars.lights_state_table[id >> 1] = 0;
}

void light_set1(uint32_t light) {
	assert((light & 1) == 0);
	g_host.Debug("light_set1 light:0x%x", light);
	const uint16_t id = light;
	int state = g_vars.lights_state_table[light >> 1];
	if (state == 2) {
		Light_TurnOff(id);
	}

	light = ((id + 1) << 16) | ((id + 1) & 0xFFFF);

	int wr = g_vars.lights_queue_wr_index;
	g_vars.lights_queue[ wr           / sizeof(uint16_t)] = light >> 16;
	g_vars.lights_queue[(wr + 2) / sizeof(uint16_t)] = light & 0xFFFF;
	wr = (wr + 4) & 0xFF;
	g_vars.lights_queue_wr_index = wr;

	g_vars.lights_state_table[id >> 1] = 1;
}

void light_set2(uint32_t light) {
	assert((light & 1) == 0);
	g_host.Debug("light_set2 light:0x%x", light);

	uint16_t lo = light;
	light = (light >> 16) | ((light & 0xFFFF) << 16);
	light = (light & 0xFFFF0000) | lo;

	uint16_t *list = g_vars.lights_blinking_list;
	for (int i = 0; i < 16; ++i) {
		if (list[i * 2] == (light >> 16) && list[i * 2 + 1] == (light & 0xFFFF)) {
			return;
		}
	}
	for (int i = 0; i < 16; ++i) {
		if (list[i * 2] & 0x8000) {
			list[i * 2] = light >> 16;
			list[i * 2 + 1] = light & 0xFFFF;
			g_vars.lights_state_table[(light & 0xFFFF) >> 1] = 2;
			break;
		}
	}
}

void light_set5(uint32_t light) {
	assert((light & 1) == 0);
	g_host.Debug("light_set5 light:0x%x", light);
	const uint16_t id = light;
	uint8_t *states = g_vars.lights_state_table;
	light &= 0xFFFF;
	light >>= 1;
	int state = states[light];
	states[light] = 0xFF;
	if (state == 2) {
		light = id;
		Light_TurnOff(light);
	}

	light = (id << 16) | id;

	int wr = g_vars.lights_queue_wr_index;
	g_vars.lights_queue[ wr           / sizeof(uint16_t)] = light >> 16;
	g_vars.lights_queue[(wr + 2) / sizeof(uint16_t)] = light & 0xFFFF;
	wr = (wr + 4) & 0xFF;
	g_vars.lights_queue_wr_index = wr;
}

void light_set6(uint32_t light) {
	assert((light & 1) == 0);
	g_host.Debug("light_set6 light:0x%x", light);
	const uint32_t id = light & 0xFFFF;
	uint8_t *states = g_vars.lights_state_table;
	light &= 0xFFFF;
	light >>= 1;
	uint16_t state = states[light];
	states[light] = 0xFF;
	if (state == 2) {
		light = id;
		Light_TurnOff(light);
	}

	light = ((id ^ 1) << 16) | (id ^ 1);

	int wr = g_vars.lights_queue_wr_index;
	g_vars.lights_queue[ wr           / sizeof(uint16_t)] = light >> 16;
	g_vars.lights_queue[(wr + 2) / sizeof(uint16_t)] = light & 0xFFFF;
	wr = (wr + 4) & 0xFF;
	g_vars.lights_queue_wr_index = wr;

	g_vars.lights_state_table[(light & 0xFFFF) >> 1] = 6;
}

void light_set7(uint32_t light) {
	assert((light & 1) == 0);
	g_host.Debug("light_set7 light:0x%x", light);
	const uint32_t id = light & 0xFFFF;
	uint8_t *states = g_vars.lights_state_table;
	light &= 0xFFFF;
	light >>= 1;
	uint16_t state = states[light];
	states[light] = 0xFF;
	if (state == 2) {
		light = id;
		Light_TurnOff(id);
	}
	light = (light & 0xFFFF0000) | id;
	uint16_t lo = light;
	light = (light >> 16) | ((light & 0xFFFF) << 16);
	light = (light & 0xFFFF0000) | lo;

	int wr = g_vars.lights_queue_wr_index;
	g_vars.lights_queue[ wr           / sizeof(uint16_t)] = light >> 16;
	g_vars.lights_queue[(wr + 2) / sizeof(uint16_t)] = light & 0xFFFF;
	wr = (wr + 4) & 0xFF;
	g_vars.lights_queue_wr_index = wr;

	g_vars.lights_state_table[(light & 0xFFFF) >> 1] = 7;
}

void lights_update_state() {
	uint8_t *states = g_vars.lights_state_table;
	int index = -1;
	while (++index != 64) {
		const uint8_t state = states[index];
		if (state & 0x80) {
			continue;
		}
		if (state == 5) {
			continue;
		}
		assert(state < 8);

		const uint16_t light = index * 2;
		switch (state) {
		case 0:
			light_set0(light);
			break;
		case 1:
			light_set1(light);
			break;
		case 2:
		case 3:
		case 4:
			light_set2(light);
			break;
		case 6:
			light_set6(light);
			break;
		case 7:
			light_set7(light);
			break;
		}
	}
}

void lights_draw() {

	memset(g_vars.gfx_lights, 0, sizeof(g_vars.gfx_lights));

	if (g_vars.lights_queue_wr_index != g_vars.lights_queue_rd_index) {
		uint16_t rd = g_vars.lights_queue_rd_index;
		uint16_t entry = g_vars.lights_queue[rd / sizeof(uint16_t)];
		g_vars.lights_queue[rd / sizeof(uint16_t)] = 0xFFFF;
		rd = (rd + 2) & 0xFF;
		g_vars.lights_queue_rd_index = rd;

		if ((entry & 0x8000) == 0) {

			assert(entry >= 0 && entry < 256);
			g_vars.gfx_lights[entry] = 1;
		}
	}

	uint16_t index = g_vars.lights_blinking_list_index;
	g_vars.lights_blinking_list_index = (index + 2) & 0x3F;
	if (index == 0) {
		g_vars.lights_blink_phase ^= 1;
	}
	uint16_t entry = g_vars.lights_blinking_list[index / sizeof(uint16_t)];
	if ((entry & 0x8000) == 0) {
		if (g_vars.lights_blink_phase != 0) {
			++entry;
		}

		assert(entry >= 0 && entry < 256);
		g_vars.gfx_lights[entry] = 1;
	}
}

void reset_lights_pearl_shell() {
	light_set7(0x44);
}

void reset_lights_dive() {
	light_set5(0x1e);
	light_set5(0x20);
	light_set5(0x22);
	light_set5(0x24);
}

void reset_lights_arrows() {
	light_set5(0x3e);
	light_set5(0x40);
	light_set5(0x42);
	light_set5(0x60);
	light_set5(0x62);
	light_set5(0x46);
	light_set5(0x48);
	light_set5(0x4a);
	light_set5(0x50);
	light_set5(0x52);
	light_set5(0x66);
}

void reset_lights_sea() {
	light_set5(0x34);
	light_set5(0x36);
	light_set5(0x38);
}

void reset_lights_starfish() {
	light_set5(0x3c);
}

void reset_lights_hb() {
	light_set5(0x26);
	light_set5(0x28);
}

void reset_lights_pearl() {
	light_set5(0x2a);
	light_set5(0x2c);
	light_set5(0x2e);
	light_set5(0x30);
	light_set5(0x32);
}

void reset_lights_score_milestones() {
	light_set5(0x3a);
	light_set5(0x64);
	light_set5(0x4c);
	light_set5(0x4e);
}

void reset_lights_multipliers() {
	light_set5(0x54);
	light_set5(0x56);
	light_set5(0x58);
	light_set5(0x5a);
	light_set5(0x5c);
	light_set5(0x5e);
}
