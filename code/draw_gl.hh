#pragma once

#include "util.hh"

enum FontStyle {
	FontStyle_Regular,
	FontStyle_Count,
};


void renderer_init();
void renderer_deinit();

void renderer_clear( float r, float g, float b );

void immediate_begin( float width, float height );
void immediate_flush();

void immediate_push_rect( Vec3 pos, Vec2 size, Vec3 color );
