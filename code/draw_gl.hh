#pragma once

void renderer_init();
void renderer_deinit();

void renderer_clear( float r, float g, float b );

void immediate_begin();
void immediate_flush();
