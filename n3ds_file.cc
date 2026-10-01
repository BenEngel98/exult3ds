/*
 *  n3ds_file.cc - Memory-backed input streams for the Nintendo 3DS port.
 *
 *  Copyright (C) 2026  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifdef __3DS__

#	include "n3ds_file.h"

#	include <cstdio>
#	include <streambuf>
#	include <vector>

/*
 *  The 3DS kernel allows a process only a few hundred open handles, and
 *  every open SD card file costs one. Exult keeps every game data file it
 *  has touched open (144 U7IFIX files alone), so the limit is hit a few
 *  seconds into a game and every later open fails. Small files are
 *  therefore slurped into memory and served from there.
 */

namespace {

	class Membuf : public std::streambuf {
	public:
		explicit Membuf(std::vector<char>&& d) : data(std::move(d)) {
			char* b = data.data();
			setg(b, b, b + data.size());
		}

	protected:
		pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
			if (!(which & std::ios_base::in)) {
				return pos_type(off_type(-1));
			}
			const off_type size = static_cast<off_type>(data.size());
			off_type       base;
			if (dir == std::ios_base::beg) {
				base = 0;
			} else if (dir == std::ios_base::cur) {
				base = gptr() - eback();
			} else {
				base = size;
			}
			const off_type np = base + off;
			if (np < 0 || np > size) {
				return pos_type(off_type(-1));
			}
			setg(eback(), eback() + np, egptr());
			return pos_type(np);
		}

		pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
			return seekoff(off_type(pos), std::ios_base::beg, which);
		}

	private:
		std::vector<char> data;
	};

	class MemIstream : public std::istream {
	public:
		explicit MemIstream(std::vector<char>&& d) : std::istream(nullptr), buf(std::move(d)) {
			rdbuf(&buf);
		}

	private:
		Membuf buf;
	};

}    // namespace

std::unique_ptr<std::istream> n3ds_open_in_memory(const char* path, std::ios_base::openmode mode, std::size_t max_bytes) {
	if (mode & std::ios_base::out) {
		return nullptr;
	}
	FILE* f = std::fopen(path, "rb");
	if (f == nullptr) {
		return nullptr;
	}
	if (std::fseek(f, 0, SEEK_END) != 0) {
		std::fclose(f);
		return nullptr;
	}
	const long len = std::ftell(f);
	if (len < 0 || static_cast<std::size_t>(len) > max_bytes) {
		std::fclose(f);
		return nullptr;
	}
	std::rewind(f);
	std::vector<char> data(static_cast<std::size_t>(len));
	std::size_t       got = 0;
	while (got < data.size()) {
		const std::size_t n = std::fread(data.data() + got, 1, data.size() - got, f);
		if (n == 0) {
			break;
		}
		got += n;
	}
	std::fclose(f);
	if (got != data.size()) {
		return nullptr;
	}
	return std::make_unique<MemIstream>(std::move(data));
}

#endif    // __3DS__
