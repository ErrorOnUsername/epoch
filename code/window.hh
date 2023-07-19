#pragma once


void window_init( int width, int height, char const* title );
void window_deinit();

bool window_should_close();
void window_set_title( char const* title );
