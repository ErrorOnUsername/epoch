#pragma once
#include <cstdint>

#include "util.hh"


#define GLYPH_COUNT 128

struct GlyphMetric
{
	Vec2 advance;
	Vec2 bitmap_size;
	Vec2 bitmap_top_left;

	float tex_off_x;
};

struct FontAtlas
{
	uint32_t width;
	uint32_t height;
	uint32_t texture;

	GlyphMetric metrics[GLYPH_COUNT];
};


bool init_font_atlas( FontAtlas* atlas, char const* path );

