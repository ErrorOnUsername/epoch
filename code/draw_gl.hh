#pragma once

void renderer_init();
void renderer_deinit();

void renderer_clear( float r, float g, float b );

void immediate_begin( float aspect_ratio );
void immediate_flush();

void immediate_push_rect( float width, float height, float r, float g, float b );
