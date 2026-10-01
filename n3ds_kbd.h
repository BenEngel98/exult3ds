/*
 *  n3ds_kbd.h - On-screen keyboard on the Nintendo 3DS bottom screen.
 *
 *  Copyright (C) 2026  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef N3DS_KBD_H
#define N3DS_KBD_H

#ifdef __3DS__

#	include <SDL3/SDL.h>

// Create / destroy the keyboard window on the bottom (touch) screen.
void n3ds_kbd_create();
void n3ds_kbd_destroy();
bool n3ds_kbd_exists();

// Redraw if needed and present. Call from the main thread regularly
// (Image_window::show() does).
void n3ds_kbd_present();

// Event filter helper: returns true if the event was aimed at the keyboard
// window and has been consumed (the caller should drop it).
bool n3ds_kbd_handle_event(SDL_Event* event);

// The window that game input should be delivered to (set by Image_window).
void        n3ds_set_game_window(SDL_Window* w);
SDL_Window* n3ds_get_game_window();

// Which screen the game is on: false = top (default), true = bottom.
bool n3ds_game_on_bottom();
void n3ds_request_screen_swap();
bool n3ds_take_screen_swap_request();

// Re-create the game window on the screen chosen by n3ds_game_on_bottom()
// and the keyboard / info panel on the other one (defined in exult.cc).
void n3ds_apply_screen();

#endif    // __3DS__
#endif    // N3DS_KBD_H
