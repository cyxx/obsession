
#ifndef RENDER_H__
#define RENDER_H__

#include "intern.h"
#include "resource.h"

enum {
	DRAW_SURFACE_TABLE = 0,
	DRAW_SURFACE_LED,

	DRAW_SURFACE_COUNT
};

void InitPalette(const ResourcesData *rd, int surface);
void Draw(ResourcesData *data);

#endif
