/*
 *  n3ds_fs.h - File reading for the Nintendo 3DS port.
 *
 *  Copyright (C) 2026  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifndef N3DS_FS_H
#define N3DS_FS_H

#ifdef __3DS__

#	include <ios>
#	include <istream>
#	include <memory>

// Reads the whole file into memory and returns a seekable stream over it
// (nullptr if the file cannot be read). The file handle is closed at once.
std::unique_ptr<std::istream> n3ds_open_in(const char* name, std::ios_base::openmode mode);

// Log the number of open file handles and the mounted devices.
void n3ds_fs_report(const char* when);

#endif    // __3DS__
#endif    // N3DS_FS_H
