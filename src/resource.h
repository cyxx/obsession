
#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "intern.h"

enum {
	PALETTES_COUNT = 10,
	COLORS_COUNT = 32 * 10,
	TABLE_W = 336,
	TABLE_H = 500,
	FLIPPER_W = 256,
	FLIPPER_H = 45,
	BALL_W = 16,
	BALL_H = 16,
	LED_W = 336,
	LED_H = 34,
	PHYSMAP_LAYER_OFFSET = 151 * 168,
	LED_PITCH = LED_W / 8,
	SCROLL_H = 226,
};

enum {
	RES_COPPER = 0,
	RES_COLORS,
	RES_TXT_ATTRACT,
	RES_GFX_OFS,
	RES_GFX_DAT,
	RES_TABLE_GFX,
	RES_TABLE_PIC,
	RES_LFLIPPER_GFX,
	RES_RFLIPPER_GFX,
	RES_BALL_GFX,
	RES_BALL_MSK,
	RES_COS,
	RES_PHYSMAP,
	RES_FRICTION,
	RES_NORMALS,
	RES_BALL_COLLISION,
	RES_TABLE_MSK,
	RES_BALL_PATHS,
	RES_LFLIPPER_PHYSMAP,
	RES_RFLIPPER_PHYSMAP,
	RES_FLIPPER_COLLISION,
	RES_VSCROLL,
	RES_FONT_LED_SCROLL_TEXT,
	RES_TABLE_LED_TEXT,
	RES_TILT_BALL,
	RES_TXT_COPYRIGHT,
	RES_TXT_EXTRA_BALL_LIT,
	RES_TXT_SAFE_BALL_LIT,
	RES_TXT_ADDING_BONUS,
	RES_TXT_SAFE_BALL,
	RES_TXT_INCREASE_BONUS_LIT,
	RES_FONT_8_16,
	RES_FONT_16_5,
	RES_FONT_LED_8_13,
	RES_LED_QUIT,
	RES_LED_PLAYER_BALL,
	RES_LED_PLAYER_GAME_OVER,
	RES_LED_BONUS,
	RES_LED_HALL_OF_FAME,
	RES_LED_200_000,
	RES_LED_100_000,
	RES_LED_500_000,
	RES_LED_50_000,
	RES_LED_GOING_DOWN,
	RES_LED_KICKBACK_ACTIVATED,
	RES_LED_ENTERING_MISSION,
	RES_LED_DIVE_MISSION_COMPLETED,
	RES_LED_NOT_TASTY,
	RES_LED_HAPPY_STARFISH,
	RES_LED_SHOOT_AGAIN,
	RES_LED_SHRIMP_ATTACK,
	RES_LED_TWO,
	RES_LED_THREE,
	RES_LED_SIX,
	RES_LED_SKILL_SHOT,
	RES_LED_UDS_IS_GOOD_FOR_YOU,
	RES_LED_EXTRA_BALL,
	RES_LED_MESSAGE_END,
	RES_LED_TILT,
	RES_LED_DIVING_DEEPER,
	RES_LED_BONUS_INCREASED,
	RES_LED_SUPER_CHARGE,
	RES_LED_PERCENTAGE,
	RES_LED_BLANK,
	RES_SOUND,
	RES_MUSIC,
};

struct gfx_t {
	uint32_t src;
	uint32_t dst;
	uint16_t w, h;
};

struct collide_t {
	int16_t behaviour_type;
	int16_t param1;
	int16_t param2;
	int16_t param3;
	int16_t param4;
	int16_t param5;
	int16_t unused0;
	int16_t unused1;
};

typedef struct {
	uint8_t *data[3];
	struct gfx_t *gfx_dat;
	int gfx_dat_count;
	uint16_t *gfx_ofs;
	int gfx_ofs_count;
	uint8_t *table_pic;
	uint16_t colors[COLORS_COUNT];
	int16_t trigo[360];
	struct collide_t ball_collision[30];
	uint8_t *phys_mat;
	uint8_t ball_gfx[BALL_W * BALL_H];
	uint8_t ball_msk[BALL_H * BALL_W / 8];
} ResourcesData;

int LoadResources(FILE *fp, const char *name, ResourcesData *rd);
uint8_t *Resource_GetData(ResourcesData *rd, int num, uint32_t *size);

const struct gfx_t *Resource_GetGfx(const ResourcesData *, int num);
const uint8_t *Resource_GetTablePic(ResourcesData *);
const uint8_t *Resource_LeftFlipperGfx(ResourcesData *);
const uint8_t *Resource_RightFlipperGfx(ResourcesData *);
int16_t *Resource_GetCosTable(ResourcesData *rd);
uint8_t *Resource_GetBallPath(ResourcesData *rd, int num);
uint8_t *Resource_GetPhysMap(ResourcesData *rd);
uint8_t *Resource_GetFlipperCollisionData(ResourcesData *rd, int num);
const uint8_t *Resource_GetHint(ResourcesData *rd, int num);

#endif
