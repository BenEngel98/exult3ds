/*
 *  n3ds_file.h - Memory-backed input streams for the Nintendo 3DS port.
 *
 *  Copyright (C) 2026  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your version) any later version.
 */

#ifndef N3DS_FILE_H
#define N3DS_FILE_H

#ifdef __3DS__

#	include <ios>
#	include <istream>
#	include <memory>

// Opens 'path' and, if it is at most 'max_bytes' long, reads the whole file
// into memory and returns a seekable stream over that buffer (the SD card
// file handle is released at once). Returns nullptr when the file is bigger
// than that or cannot be opened; the caller should fall back to a normal
// stream.
std::unique_ptr<std::istream> n3ds_open_in_memory(const char* path, std::ios_base::openmode mode, std::size_t max_bytes);

#endif    // __3DS__
#endif    // N3DS_FILE_H
