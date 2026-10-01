/*
 *  n3ds_kbd.cc - On-screen keyboard on the Nintendo 3DS bottom screen.
 *
 *  Copyright (C) 2026  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifdef __3DS__

#	include "n3ds_kbd.h"

#	include <cctype>
#	include <cstring>
#	include <string>
#	include <vector>

namespace {

	// ---------------------------------------------------------------------
	// A tiny 5x7 bitmap font (original, hand drawn). Rows top to bottom,
	// '#' = pixel on.
	// ---------------------------------------------------------------------
	struct Glyph {
		char        ch;
		const char* rows[7];
	};

	const Glyph glyphs[] = {
			{'A', {".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
			{'B', {"####.", "#...#", "#...#", "####.", "#...#", "#...#", "####."}},
			{'C', {".###.", "#...#", "#....", "#....", "#....", "#...#", ".###."}},
			{'D', {"####.", "#...#", "#...#", "#...#", "#...#", "#...#", "####."}},
			{'E', {"#####", "#....", "#....", "####.", "#....", "#....", "#####"}},
			{'F', {"#####", "#....", "#....", "####.", "#....", "#....", "#...."}},
			{'G', {".###.", "#...#", "#....", "#.###", "#...#", "#...#", ".####"}},
			{'H', {"#...#", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"}},
			{'I', {".###.", "..#..", "..#..", "..#..", "..#..", "..#..", ".###."}},
			{'J', {"..###", "...#.", "...#.", "...#.", "...#.", "#..#.", ".##.."}},
			{'K', {"#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"}},
			{'L', {"#....", "#....", "#....", "#....", "#....", "#....", "#####"}},
			{'M', {"#...#", "##.##", "#.#.#", "#.#.#", "#...#", "#...#", "#...#"}},
			{'N', {"#...#", "##..#", "#.#.#", "#..##", "#...#", "#...#", "#...#"}},
			{'O', {".###.", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
			{'P', {"####.", "#...#", "#...#", "####.", "#....", "#....", "#...."}},
			{'Q', {".###.", "#...#", "#...#", "#...#", "#.#.#", "#..#.", ".##.#"}},
			{'R', {"####.", "#...#", "#...#", "####.", "#.#..", "#..#.", "#...#"}},
			{'S', {".####", "#....", "#....", ".###.", "....#", "....#", "####."}},
			{'T', {"#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."}},
			{'U', {"#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."}},
			{'V', {"#...#", "#...#", "#...#", "#...#", "#...#", ".#.#.", "..#.."}},
			{'W', {"#...#", "#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"}},
			{'X', {"#...#", "#...#", ".#.#.", "..#..", ".#.#.", "#...#", "#...#"}},
			{'Y', {"#...#", "#...#", ".#.#.", "..#..", "..#..", "..#..", "..#.."}},
			{'Z', {"#####", "....#", "...#.", "..#..", ".#...", "#....", "#####"}},
			{'0', {".###.", "#...#", "#..##", "#.#.#", "##..#", "#...#", ".###."}},
			{'1', {"..#..", ".##..", "..#..", "..#..", "..#..", "..#..", ".###."}},
			{'2', {".###.", "#...#", "....#", "...#.", "..#..", ".#...", "#####"}},
			{'3', {"#####", "...#.", "..#..", "...#.", "....#", "#...#", ".###."}},
			{'4', {"...#.", "..##.", ".#.#.", "#..#.", "#####", "...#.", "...#."}},
			{'5', {"#####", "#....", "####.", "....#", "....#", "#...#", ".###."}},
			{'6', {"..##.", ".#...", "#....", "####.", "#...#", "#...#", ".###."}},
			{'7', {"#####", "....#", "...#.", "..#..", ".#...", ".#...", ".#..."}},
			{'8', {".###.", "#...#", "#...#", ".###.", "#...#", "#...#", ".###."}},
			{'9', {".###.", "#...#", "#...#", ".####", "....#", "...#.", ".##.."}},
			{'.', {".....", ".....", ".....", ".....", ".....", ".##..", ".##.."}},
			{',', {".....", ".....", ".....", ".....", ".##..", "..#..", ".#..."}},
			{'-', {".....", ".....", ".....", "#####", ".....", ".....", "....."}},
			{'\'', {".##..", "..#..", ".#...", ".....", ".....", ".....", "....."}},
			{'?', {".###.", "#...#", "....#", "...#.", "..#..", ".....", "..#.."}},
			{'!', {"..#..", "..#..", "..#..", "..#..", "..#..", ".....", "..#.."}},
			{'<', {"...#.", "..#..", ".#...", "#....", ".#...", "..#..", "...#."}},
			{'^', {"..#..", ".#.#.", "#...#", ".....", ".....", ".....", "....."}},
			{'_', {".....", ".....", ".....", ".....", ".....", ".....", "#####"}},
	};

	const Glyph* find_glyph(char c) {
		c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
		for (const Glyph& g : glyphs) {
			if (g.ch == c) {
				return &g;
			}
		}
		return nullptr;
	}

	// Draw text with the bitmap font, scale = pixel size of one font pixel.
	void draw_text(SDL_Renderer* r, int x, int y, const char* text, int scale) {
		for (const char* p = text; *p; ++p) {
			if (const Glyph* g = find_glyph(*p)) {
				for (int row = 0; row < 7; row++) {
					for (int col = 0; col < 5; col++) {
						if (g->rows[row][col] == '#') {
							SDL_FRect px = {static_cast<float>(x + col * scale), static_cast<float>(y + row * scale),
											static_cast<float>(scale), static_cast<float>(scale)};
							SDL_RenderFillRect(r, &px);
						}
					}
				}
			}
			x += 6 * scale;
		}
	}

	int text_width(const char* text, int scale) {
		return static_cast<int>(std::strlen(text)) * 6 * scale - scale;
	}

	// ---------------------------------------------------------------------
	// Keyboard layout
	// ---------------------------------------------------------------------
	struct Key {
		int         x, y, w, h;
		const char* label;
		SDL_Keycode key;
		char        text;    // character typed (0 = none)
		bool        is_shift;
	};

	constexpr int KW  = 28;    // key width
	constexpr int KH  = 36;    // key height
	constexpr int GAP = 4;
	constexpr int X0  = 4;
	constexpr int Y0  = 6;

	std::vector<Key> keys;

	void add_row(int row, const char* chars) {
		int x = X0;
		for (const char* p = chars; *p; ++p) {
			static char labels[64][2];
			static int  n = 0;
			char*       l = labels[n++ % 64];
			l[0]          = *p;
			l[1]          = 0;
			SDL_Keycode k = SDLK_UNKNOWN;
			if (*p >= '0' && *p <= '9') {
				k = SDLK_0 + (*p - '0');
			} else if (*p >= 'A' && *p <= 'Z') {
				k = SDLK_A + (*p - 'A');
			} else if (*p == '.') {
				k = SDLK_PERIOD;
			} else if (*p == ',') {
				k = SDLK_COMMA;
			} else if (*p == '-') {
				k = SDLK_MINUS;
			} else if (*p == '\'') {
				k = SDLK_APOSTROPHE;
			} else if (*p == '?') {
				k = SDLK_SLASH;
			}
			keys.push_back({x, Y0 + row * (KH + GAP), KW, KH, l, k, *p, false});
			x += KW + GAP;
		}
	}

	void build_layout() {
		keys.clear();
		add_row(0, "1234567890");
		add_row(1, "QWERTYUIOP");
		add_row(2, "ASDFGHJKL'");
		add_row(3, "ZXCVBNM,.?");
		// Bottom row: ESC | SHIFT | SPACE | BKSP | ENTER
		const int y = Y0 + 4 * (KH + GAP);
		keys.push_back({X0, y, 44, KH, "ESC", SDLK_ESCAPE, 0, false});
		keys.push_back({X0 + 48, y, 44, KH, "^", SDLK_LSHIFT, 0, true});
		keys.push_back({X0 + 96, y, 108, KH, "SPACE", SDLK_SPACE, ' ', false});
		keys.push_back({X0 + 208, y, 44, KH, "<", SDLK_BACKSPACE, 0, false});
		keys.push_back({X0 + 256, y, 56, KH, "ENTER", SDLK_RETURN, 0, false});
	}

	// ---------------------------------------------------------------------
	// State
	// ---------------------------------------------------------------------
	SDL_Window*   kbd_window   = nullptr;
	SDL_Renderer* kbd_renderer = nullptr;
	SDL_WindowID  kbd_id       = 0;
	SDL_Window*   game_window  = nullptr;
	bool          shifted      = false;
	int           pressed_key  = -1;
	bool          dirty        = true;
	Uint64        last_present = 0;
	bool          on_bottom    = false;
	bool          swap_request = false;
	bool          aux_is_info  = false;    // true: the aux window is the top-screen info panel

	int key_at(float px, float py) {
		for (size_t i = 0; i < keys.size(); i++) {
			const Key& k = keys[i];
			if (px >= k.x && px < k.x + k.w && py >= k.y && py < k.y + k.h) {
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	void push_key(const Key& k, bool down) {
		SDL_Event ev;
		SDL_zero(ev);
		ev.type          = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
		ev.key.timestamp = SDL_GetTicksNS();
		ev.key.windowID  = game_window ? SDL_GetWindowID(game_window) : 0;
		ev.key.key       = k.key;
		ev.key.scancode  = SDL_GetScancodeFromKey(k.key, nullptr);
		ev.key.mod       = shifted ? SDL_KMOD_SHIFT : SDL_KMOD_NONE;
		ev.key.down      = down;
		ev.key.repeat    = false;
		SDL_PushEvent(&ev);

		if (down && k.text && game_window && SDL_TextInputActive(game_window)) {
			// Typing into a text field (character name, save name...).
			static char bufs[16][2];
			static int  n   = 0;
			char*       buf = bufs[n++ % 16];
			buf[0] = shifted ? static_cast<char>(std::toupper(static_cast<unsigned char>(k.text)))
							 : static_cast<char>(std::tolower(static_cast<unsigned char>(k.text)));
			buf[1] = 0;
			SDL_Event te;
			SDL_zero(te);
			te.type           = SDL_EVENT_TEXT_INPUT;
			te.text.timestamp = SDL_GetTicksNS();
			te.text.windowID  = SDL_GetWindowID(game_window);
			te.text.text      = buf;
			SDL_PushEvent(&te);
		}
	}

	void draw_info() {
		SDL_SetRenderDrawColor(kbd_renderer, 10, 10, 18, 255);
		SDL_RenderClear(kbd_renderer);
		const char* lines[] = {"EXULT", "", "THE GAME IS ON THE", "BOTTOM SCREEN", "", "PRESS SELECT", "TO SWAP BACK"};
		int         y       = 60;
		for (const char* l : lines) {
			const int scale = (y == 60) ? 3 : 2;
			const int tw    = text_width(l, scale);
			SDL_SetRenderDrawColor(kbd_renderer, 200, 210, 240, 255);
			draw_text(kbd_renderer, (400 - tw) / 2, y, l, scale);
			y += 7 * scale + 6;
		}
		SDL_RenderPresent(kbd_renderer);
	}

	void draw() {
		if (!kbd_renderer) {
			return;
		}
		if (aux_is_info) {
			draw_info();
			return;
		}
		SDL_SetRenderDrawColor(kbd_renderer, 24, 22, 34, 255);
		SDL_RenderClear(kbd_renderer);
		for (size_t i = 0; i < keys.size(); i++) {
			const Key& k       = keys[i];
			const bool pressed = static_cast<int>(i) == pressed_key || (k.is_shift && shifted);
			SDL_FRect  rc      = {static_cast<float>(k.x), static_cast<float>(k.y), static_cast<float>(k.w),
								  static_cast<float>(k.h)};
			if (pressed) {
				SDL_SetRenderDrawColor(kbd_renderer, 120, 150, 230, 255);
			} else {
				SDL_SetRenderDrawColor(kbd_renderer, 72, 70, 88, 255);
			}
			SDL_RenderFillRect(kbd_renderer, &rc);
			SDL_SetRenderDrawColor(kbd_renderer, 110, 108, 128, 255);
			SDL_RenderRect(kbd_renderer, &rc);
			// label
			const int   scale = (std::strlen(k.label) == 1) ? 2 : 1;
			const int   tw    = text_width(k.label, scale);
			const int   th    = 7 * scale;
			SDL_SetRenderDrawColor(kbd_renderer, 235, 235, 245, 255);
			draw_text(kbd_renderer, k.x + (k.w - tw) / 2, k.y + (k.h - th) / 2, k.label, scale);
		}
		SDL_RenderPresent(kbd_renderer);
	}

}    // namespace

void n3ds_set_game_window(SDL_Window* w) {
	game_window = w;
}

SDL_Window* n3ds_get_game_window() {
	return game_window;
}

bool n3ds_game_on_bottom() {
	return on_bottom;
}

void n3ds_request_screen_swap() {
	swap_request = true;
}

bool n3ds_take_screen_swap_request() {
	const bool r = swap_request;
	swap_request = false;
	if (r) {
		on_bottom = !on_bottom;
	}
	return r;
}

bool n3ds_kbd_exists() {
	return kbd_window != nullptr;
}

void n3ds_kbd_create() {
	if (kbd_window) {
		return;
	}
	build_layout();
	int            count    = 0;
	SDL_DisplayID* displays = SDL_GetDisplays(&count);
	SDL_DisplayID  top      = (displays && count > 0) ? displays[0] : 0;
	SDL_DisplayID  bottom   = (displays && count > 1) ? displays[1] : 0;
	SDL_free(displays);
	// Game on top -> keyboard on the bottom screen; game on bottom -> info panel on top.
	aux_is_info                = on_bottom;
	const SDL_DisplayID target = aux_is_info ? top : bottom;
	const int           aw     = aux_is_info ? 400 : 320;
	if (!target) {
		return;
	}
	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, aux_is_info ? "info" : "keyboard");
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(target));
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(target));
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, aw);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 240);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
	kbd_window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);
	if (!kbd_window) {
		return;
	}
	kbd_id       = SDL_GetWindowID(kbd_window);
	kbd_renderer = SDL_CreateRenderer(kbd_window, nullptr);
	// Keep keyboard focus on the game window: the keyboard is touch only.
	if (game_window) {
		SDL_RaiseWindow(game_window);
	}
	dirty = true;
}

void n3ds_kbd_destroy() {
	if (kbd_renderer) {
		SDL_DestroyRenderer(kbd_renderer);
		kbd_renderer = nullptr;
	}
	if (kbd_window) {
		SDL_DestroyWindow(kbd_window);
		kbd_window = nullptr;
	}
	kbd_id      = 0;
	pressed_key = -1;
}

void n3ds_kbd_present() {
	if (!kbd_window) {
		return;
	}
	const Uint64 now = SDL_GetTicks();
	if (dirty || now - last_present > 500) {
		draw();
		dirty        = false;
		last_present = now;
	}
}

bool n3ds_kbd_handle_event(SDL_Event* event) {
	// A handheld never really loses focus; SDL hands the focus to whichever
	// window was created last, which would pause the game whenever the
	// keyboard / info window appears.
	if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST) {
		return true;
	}
	if (!kbd_window) {
		return false;
	}
	if (event->type >= SDL_EVENT_WINDOW_FIRST && event->type <= SDL_EVENT_WINDOW_LAST) {
		return event->window.windowID == kbd_id;
	}
	if (aux_is_info) {
		return false;
	}
	switch (event->type) {
	case SDL_EVENT_FINGER_DOWN:
	case SDL_EVENT_FINGER_UP:
	case SDL_EVENT_FINGER_MOTION:
	case SDL_EVENT_FINGER_CANCELED: {
		if (event->tfinger.windowID != kbd_id) {
			return false;
		}
		const float px = event->tfinger.x * 320.f;
		const float py = event->tfinger.y * 240.f;
		if (event->type == SDL_EVENT_FINGER_DOWN) {
			const int i = key_at(px, py);
			if (i >= 0) {
				const Key& k = keys[i];
				if (k.is_shift) {
					shifted = !shifted;
				} else {
					pressed_key = i;
					push_key(k, true);
				}
				dirty = true;
			}
		} else if (event->type == SDL_EVENT_FINGER_UP || event->type == SDL_EVENT_FINGER_CANCELED) {
			if (pressed_key >= 0) {
				push_key(keys[pressed_key], false);
				pressed_key = -1;
				dirty       = true;
			}
		}
		return true;    // consumed
	}
	case SDL_EVENT_MOUSE_MOTION:
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
		// Mouse events synthesised from touches on the keyboard window.
		return (event->type == SDL_EVENT_MOUSE_MOTION ? event->motion.windowID : event->button.windowID) == kbd_id;
	default:
		return false;
	}
}

#endif    // __3DS__
