
#include "host.h"
#include "resource.h"
#include "unpack.h"

static void SetColor(int color, uint16_t rgb444, ResourcesData *rd) {

	for (int i = 0; i < COLORS_COUNT; i += 32) {
		rd->colors[i + color] = rgb444;
	}
}

static const uint32_t COLOR00 = 0x180;
static const uint32_t COLOR31 = 0x1BE;

static void LoadCopper(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	for (uint32_t offset = 0; offset < size; ) {
		const uint16_t reg = READ_BE_UINT16(data + offset); offset += 2;
		const uint16_t val = READ_BE_UINT16(data + offset); offset += 2;

		if (reg >= COLOR00 && reg <= COLOR31) {
			assert((reg & 1) == 0);
			const int color_num = (reg - COLOR00) / 2;
			SetColor(color_num, val, rd);
		}
	}
}

static void SetPaletteColor(int palette, int color, uint16_t rgb444, ResourcesData *rd) {

	assert(palette >= 0 && palette < PALETTES_COUNT);
	rd->colors[palette * 32 + color] = rgb444;
}

static const uint8_t COLORS[] = { 1, 2, 3, 4, 5, 6, 7, 8, 16, 17, 18, 19, 20, 21, 22 };

static void LoadColors(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	static const int count = sizeof(COLORS);
	int palette_num = 0;
	int color_num = 0;
	for (uint32_t offset = 0; offset < size; ) {
		const uint16_t rgb = READ_BE_UINT16(data + offset); offset += 2;
		SetPaletteColor(palette_num, COLORS[color_num], rgb, rd);
		++color_num;
		if (color_num == count) {
			++palette_num;
			color_num = 0;
		}
	}
}

static void LoadGfxOfs(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	assert((size % 4) == 0);
	const int count = size / 4;
	uint16_t *ofs = (uint16_t *)malloc(count * sizeof(uint16_t));
	if (ofs) {
		for (int i = 0; i < count; ++i) {
			const uint32_t addr = READ_BE_UINT32(data); data += 4;

			assert(addr >= 0xe158);
			const int offset = addr - 0xe158;
			assert((offset % 12) == 0);
			ofs[i] = offset / 12;
		}
		rd->gfx_ofs = ofs;
		rd->gfx_ofs_count = count;
	}
}

static void LoadGfxDat(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	assert((size % 12) == 0);
	const int count = size / 12;
	struct gfx_t *gfx = (struct gfx_t *)malloc(count * sizeof(struct gfx_t));
	if (gfx) {
		for (int i = 0; i < count; ++i) {
			gfx[i].src = READ_BE_UINT32(data); data += 4;
			gfx[i].dst = READ_BE_UINT32(data); data += 4;
			gfx[i].w = READ_BE_UINT16(data); data += 2;
			gfx[i].h = READ_BE_UINT16(data); data += 2;
		}
		rd->gfx_dat = gfx;
		rd->gfx_dat_count = count;
	}
}

static void DecodePlanar(const uint8_t *src, int w, int h, int depth, uint8_t *dst) {
	w >>= 3;
	const int plane_size = h * w;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x, ++src) {
			for (int b = 0; b < 8; ++b) {
				const uint8_t mask = 1 << (7 - b);
				uint8_t color = 0;
				for (int i = 0; i < depth; ++i) {
					if (src[i * plane_size] & mask) {
						color |= 1 << i;
					}
				}
				*dst++ = color;
			}
		}
	}
}

static void LoadTablePic(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	assert(size == TABLE_W * TABLE_H * 5 / 8);
	rd->table_pic = (uint8_t *)malloc(TABLE_W * TABLE_H);
	if (rd->table_pic) {
		DecodePlanar(data, TABLE_W, TABLE_H, 5, rd->table_pic);
	}
}

static void LoadBallGfx(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	uint32_t offset = 0;
	const int h = BALL_H;
	const int w = BALL_W >> 3;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			for (int b = 0; b < 8; ++b) {
				const uint8_t mask = 1 << (7 - b);
				uint8_t color = 0;
				for (int i = 0; i < 4; ++i) {
					if (data[i * w + offset] & mask) {
						color |= 1 << i;
					}
				}
				rd->ball_gfx[y * BALL_H + x * 8 + b] = color;
			}
			++offset;
		}
		offset = y * 8;
	}
}

static void LoadBallMsk(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	assert(size == BALL_W * BALL_H / 8);
	memcpy(rd->ball_msk, data, size);
}

static void LoadTrigo(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	assert(size == 360 * sizeof(int16_t));
	for (int i = 0; i < 360; ++i) {
		rd->trigo[i] = READ_BE_UINT16(data); data += 2;
	}
}

static void LoadPhysMap(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	rd->phys_mat = (uint8_t *)malloc(size);
	if (rd->phys_mat) {
		memcpy(rd->phys_mat, data, size);
		assert(data[0] == 0x20);
		assert(data[0x17] == 0xF9);
	}
}

static void LoadBallCollision(const uint8_t *data, uint32_t size, ResourcesData *rd) {
	assert(size == 240 * sizeof(int16_t));
	for (int i = 0; i < 30; ++i) {
		rd->ball_collision[i].behaviour_type = READ_BE_UINT16(data); data += 2;
		rd->ball_collision[i].param1 = READ_BE_UINT16(data); data += 2;
		rd->ball_collision[i].param2 = READ_BE_UINT16(data); data += 2;
		rd->ball_collision[i].param3 = READ_BE_UINT16(data); data += 2;
		rd->ball_collision[i].param4 = READ_BE_UINT16(data); data += 2;
		rd->ball_collision[i].param5 = READ_BE_UINT16(data); data += 2;
		rd->ball_collision[i].unused0 = READ_BE_UINT16(data); data += 2;
		rd->ball_collision[i].unused1 = READ_BE_UINT16(data); data += 2;
	}
}

static const struct {
	uint8_t entry_num;
	uint32_t offset;
	uint32_t size;
	void (*load)(const uint8_t *, uint32_t, ResourcesData *rd);
} _resources_aqua[] = {
	{ 4, 0x1e190,    136, LoadCopper },
	{ 0, 0x24cae,    300, LoadColors },
	{ 2,  0xdafe,   1210, 0 },
	{ 2,  0xdfb8,    416, LoadGfxOfs },
	{ 2,  0xe158,   1248, LoadGfxDat },
	{ 2, 0x14776,  40160, 0 },
	{ 4,   0xc88, 105000, LoadTablePic },
	{ 4, 0x1a950,   7200, 0 },
	{ 4, 0x1c570,   7200, 0 },
	{ 2,  0xf4ce,    128, LoadBallGfx },
	{ 2,  0xf54e,     32, LoadBallMsk },
	{ 0,  0xedee,    720, LoadTrigo },
	{ 0,  0xf0c2,  67368, LoadPhysMap },
	{ 0,  0xbb8e,  12800, 0 },
	{ 0,  0x915e,  10000, 0 },
	{ 0,  0x8dc4,    480, LoadBallCollision },
	{ 2,  0xf56e,  21000, 0 },
	{ 0, 0x1fd2a,   1757, 0 },
	{ 0, 0x206f0,   2464, 0 },
	{ 0, 0x21090,   2464, 0 },
	{ 0, 0x23d66,   2730, 0 },
	{ 2, 0x20b8e,   1128, 0 },
	{ 2, 0x20ff6,  33280, 0 },
	{ 2, 0x2219e,    244, 0 },
	{ 2, 0x22a0a,    120, 0 },
	{ 2,  0xdafe,   1019, 0 },
	{ 2,  0xdefa,     38, 0 },
	{ 2,  0xdf20,     37, 0 },
	{ 2,  0xdf46,     37, 0 },
	{ 2,  0xdf6c,     33, 0 },
	{ 2,  0xdf8e,     42, 0 },
	{ 0,  0x554C,   3200, 0 },
	{ 0,  0x6730,    100, 0 },
	{ 2,  0xeb46,   2288, 0 },
	{ 0,  0x20f8,    630, 0 },
	{ 2, 0x217c6,    630, 0 },
	{ 2, 0x21a3c,    630, 0 },
	{ 2, 0x21f28,    630, 0 },
	{ 0,   0xee4,      7, 0 },
	{ 2,   0xc20,    627, 0 },
	{ 2,   0xa46,    472, 0 },
	{ 2,   0xe94,    398, 0 },
	{ 2,  0x1024,    491, 0 },
	{ 2,  0x1210,     14, 0 },
	{ 2,  0x17c0,    546, 0 },
	{ 2,  0x248e,    284, 0 },
	{ 2,  0x3be2,    453, 0 },
	{ 2,  0x4ece,    362, 0 },
	{ 2,  0x5894,    451, 0 },
	{ 2,  0x5f60,    400, 0 },
	{ 2,  0x60f2,    527, 0 },
	{ 2,  0x6302,     20, 0 },
	{ 2,  0x66c2,    217, 0 },
	{ 2,  0x6956,    132, 0 },
	{ 2,  0x71b4,    394, 0 },
	{ 2,  0x7340,    530, 0 },
	{ 2,  0x7554,    632, 0 },
	{ 2,  0xb14e,     16, 0 },
	{ 2,  0xbba8,    272, 0 },
	{ 2,  0xbcba,     75, 0 },
	{ 2,  0xd2d8,    622, 0 },
	{ 2,  0xd548,    541, 0 },
	{ 2,  0xd766,    252, 0 },
	{ 2,  0xdada,      7, 0 },
	{ 4, 0x1e47c, 140068, 0 },
	{ 2, 0x22afa,  26260, 0 },

	{ 0, 0, 0, 0 }
};

static const struct {
	uint32_t compressed;
	uint32_t uncompressed;
} _aqua_bin[6] = {
	{  0x8E06, 0x25ADC },
	{  0x2376,  0x2C60 },
	{  0xAD30, 0x2925C },
	{   0x228,   0x228 },
	{ 0x32332, 0x40868 },
	{    0x1C,    0x1C }
};

static void LoadBin(FILE *fp, ResourcesData *rd) {
	uint8_t header[12];
	for (int i = 0; i < 6 && fread(header, sizeof(header), 1, fp) == 1; ++i) {
		const int compressed_size = _aqua_bin[i].compressed - 12 ;
		const int uncompressed_size = _aqua_bin[i].uncompressed;
		const int header_compressed_size = READ_BE_UINT32(header + 4);
		const int header_uncompressed_size = READ_BE_UINT32(header + 8);
		g_host.Debug("Data %d size %d,%d %d,%d", i, compressed_size, uncompressed_size, header_compressed_size, header_uncompressed_size);
		if ((i & 1) != 0) {

			fseek(fp, compressed_size, SEEK_CUR);
		} else {
			assert(memcmp(header, "Ace?", 4) == 0);
			assert(header_compressed_size == compressed_size + 12);
			g_host.Debug("Entry %d size 0x%x", i, compressed_size);
			uint8_t *read_buffer = (uint8_t *)malloc(compressed_size);
			if (!read_buffer) {
				g_host.Error("Failed to allocate %d bytes (compressed buffer)", compressed_size);
			} else {
				const int count = fread(read_buffer, 1, compressed_size, fp);
				if (count != compressed_size) {
					g_host.Error("Failed to read %d bytes, count %d", compressed_size, count);
				}
				uint8_t *data = (uint8_t *)malloc(header_uncompressed_size);
				if (!data) {
					g_host.Error("Failed to allocate %d bytes (uncompressed buffer)", header_uncompressed_size);
				} else {
					Unpack(data, header_uncompressed_size, read_buffer, count);
					rd->data[i >> 1] = data;
					free(read_buffer);
				}
			}
		}
	}
}

int LoadResources(FILE *fp, const char *name, ResourcesData *rd) {
	LoadBin(fp, rd);
	for (int i = 0; _resources_aqua[i].size != 0; ++i) {
		if (_resources_aqua[i].load) {
			g_host.Debug("Loading resource %d size %d", i, _resources_aqua[i].size);
			const int num = _resources_aqua[i].entry_num;
			assert((num & 1) == 0);
			const uint8_t *p = rd->data[num >> 1] + _resources_aqua[i].offset;
			_resources_aqua[i].load(p, _resources_aqua[i].size, rd);
		}
	}
	for (int i = 0; i < rd->gfx_ofs_count; ++i) {
		const int num = rd->gfx_ofs[i];
		assert(num < rd->gfx_dat_count);
	}
	return 0;
}

uint8_t *Resource_GetData(ResourcesData *rd, int num, uint32_t *size) {
	int data_num = _resources_aqua[num].entry_num;
	assert((data_num & 1) == 0);
	data_num >>= 1;
	if (size) {
		*size = _resources_aqua[num].size;
	}

	return rd->data[data_num] + _resources_aqua[num].offset;
}

const struct gfx_t *Resource_GetGfx(const ResourcesData *rd, int num) {
	assert(num < rd->gfx_ofs_count);
	const int ofs = rd->gfx_ofs[num];
	return &rd->gfx_dat[ofs];
}

const uint8_t *Resource_GetTablePic(ResourcesData *rd) {
	return rd->table_pic;
}

const uint8_t *Resource_LeftFlipperGfx(ResourcesData *rd) {
	return Resource_GetData(rd, RES_LFLIPPER_GFX, 0);
}

const uint8_t *Resource_RightFlipperGfx(ResourcesData *rd) {
	return Resource_GetData(rd, RES_RFLIPPER_GFX, 0);
}

int16_t *Resource_GetCosTable(ResourcesData *rd) {
	return rd->trigo;
}

uint8_t *Resource_GetBallPath(ResourcesData *rd, int num) {
	assert(num >= 0 && num < 9);
	static const int addr[] = {
		0xC2C5A6, 0xC2C6BC, 0xC2C7D2, 0xC2C8E8, 0xC2C946,
		0xC2C9A4, 0xC2CA22, 0xC2CAEC, 0xC2CBB6
	};
	const int offset = addr[num] - addr[0];
	assert(offset >= 0);
	uint8_t *p = Resource_GetData(rd, RES_BALL_PATHS, 0);
	assert(p[0] == 1 && p[1] == 0xD);
	return p + offset;
}

uint8_t *Resource_GetPhysMap(ResourcesData *rd) {
	return Resource_GetData(rd, RES_PHYSMAP, 0);
}

uint8_t *Resource_GetFlipperCollisionData(ResourcesData *rd, int num) {
	assert(num >= 0 && num < 20);
	static const int addr[] = {
		0xC305E2, 0xC3066E, 0xC306FA, 0xC30786, 0xC30786,
		0xC30C72, 0xC30CFE, 0xC30D8A, 0xC30E16, 0xC30E16,
		0xC3066E, 0xC306FA, 0xC30786, 0xC30786, 0xC30786,
		0xC30C72, 0xC30C72, 0xC30CFE, 0xC30D8A, 0xC30E16
	};
	const int offset = addr[num] - addr[0];
	assert(offset >= 0 && (offset & 1) == 0);
	uint8_t *p = Resource_GetData(rd, RES_FLIPPER_COLLISION, 0);
	assert(p[0] == 0 && p[1] == 0x90);
	return p + offset;
}

const uint8_t *Resource_GetHint(ResourcesData *rd, int num) {
	assert(num >= 0 && num < 8);
	static const int offsets[] = {
		0x24aa8, 0x24ad6, 0x24b10, 0x24b56,
		0x24ba5, 0x24c0b, 0x24c4d, 0x24c82
	};
	return rd->data[0] + offsets[num];
}
