
#ifndef INTERN_H__
#define INTERN_H__

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifndef ABS
static inline int ABS(int x) {
	return (x < 0) ? -x : x;
}
#endif

#ifndef MIN
static inline int MIN(int x, int y) {
	return (x < y) ? x : y;
}
#endif

#ifndef MAX
static inline int MAX(int x, int y) {
	return (x > y) ? x : y;
}
#endif

static inline uint16_t READ_BE_UINT16(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[0] << 8) | b[1];
}

static inline uint32_t READ_BE_UINT32(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

static inline uint16_t READ_LE_UINT16(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[1] << 8) | b[0];
}

static inline uint32_t READ_LE_UINT32(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

#endif
