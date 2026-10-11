
#include "unpack.h"

static const uint8_t TABLE[] = {
	0x0b,0x04,0x07,0x00,0x01,0x20,0x00,0x00,0x00,0x20,0x09,0x01,0x00,0xff,0xff,0x08,0x04,0x02,0x01,0x00,
	0x7f,0xff,0x00,0x0e,0x00,0xff,0x00,0x07,0x00,0x07,0x00,0x02,0x00,0x03,0x00,0x01,0x00,0x03,0x00,0x01,
	0x00,0x00,0x01,0x0d,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x01
};

struct unpack_t {
	const uint8_t *p;
	uint16_t bits;
};

static uint16_t next_word(struct unpack_t *u) {
	u->p -= 2;
	return READ_BE_UINT16(u->p);
}

static int next_bit(struct unpack_t *u) {
	int cf = (u->bits >> 15);
	u->bits <<= 1;
	if (u->bits == 0) {
		uint32_t bits = next_word(u);
		bits += bits + cf;
		cf = bits >> 16;
		assert(cf == 0 || cf == 1);
		u->bits = bits & 0xFFFF;
	}
	return cf;
}

static int get_bits(struct unpack_t *u, int count) {
	int bits = 0;
	do {
		bits += bits + next_bit(u);
	} while (--count != -1);
	return bits;
}

void Unpack(uint8_t *output, int uncompressed_size, const uint8_t *compressed_src, int compressed_size) {

	const uint8_t *len_table = &TABLE[0];
	const uint8_t *short_table = &TABLE[10];
	const uint8_t *match_table = &TABLE[40];

	struct unpack_t u;
	u.p = compressed_src + compressed_size;
	u.bits = 0;

	uint8_t *dst = output + uncompressed_size;

	if (uncompressed_size & 0x1000000) {

	}

#define copy_w(dst, src) \
	do { \
		dst -= 2; \
		src -= 2; \
		memcpy(dst, src, 2); \
	} while (0)

	copy_w(dst, u.p);
	u.bits = next_word(&u);

	while (1) {

		if (next_bit(&u) != 0) {

			if (next_bit(&u) == 0) {

				copy_w(dst, u.p);

			} else {

				const uint8_t *p = match_table;
				int tries = 4;
				int expected, actual;
				int count;
				do {
					p -= 4;
					const uint32_t entry = READ_BE_UINT32(p);
					expected = entry & 0xFFFF;
					actual = entry >> 16;
					count = get_bits(&u, expected);

				} while (actual == count && --tries != -1);
				count += (int16_t)READ_BE_UINT16(p + 22);
				do {
					copy_w(dst, u.p);
				} while (--count != -1);
			}
		}

		if (output >= dst) break;

		if (next_bit(&u) == 0) {

			int offset = 0;
			int bits = 5;
			int base = 0;
			if (next_bit(&u) != 0) {
				bits = 8;
				base = 64;
			}

			offset = get_bits(&u, bits);
			offset += base;
			offset += offset;
			const uint8_t *src = &dst[offset + 2];
			copy_w(dst, src);

		} else {

			int index = 2;
			while (next_bit(&u) != 0 && --index != -1);

			int count = 0;
			int offset = 0;
			int bits = (int8_t)short_table[index + 1];
			if (!(bits < 0)) {
				offset = get_bits(&u, bits);
			}

			count = short_table[index + 6];
			count += offset;

			index = 1;
			while (next_bit(&u) != 0 && --index != -1);

			offset = 0;
			bits = (int8_t)len_table[index + 1];
			offset = get_bits(&u, bits);

			index += index;
			offset += (int16_t)READ_BE_UINT16(len_table + index + 6);
			offset += offset;
			uint8_t *src = &dst[offset + 2];
			do {
				copy_w(dst, src);
			} while (--count != -1);

		}
	}

	assert((u.p - compressed_src) == 0);
	assert((dst - output) == 0);
}
