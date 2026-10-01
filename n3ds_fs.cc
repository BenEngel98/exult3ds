/*
 *  n3ds_fs.cc - File reading for the Nintendo 3DS port.
 *
 *  Copyright (C) 2026  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#ifdef __3DS__

#	include "n3ds_fs.h"

#	include <cerrno>
#	include <cstdio>
#	include <cstring>
#	include <iostream>
#	include <memory>
#	include <streambuf>
#	include <vector>

#	include <malloc.h>
#	include <sys/iosupport.h>

/*
 *  The 3DS file-system service only allows a small number of files to be
 *  open at the same time, and Exult likes to keep every data file it has
 *  ever touched open (shapes, fonts, the map, ...). Once the limit is hit,
 *  every further open fails and the game dies. So on the 3DS a file is read
 *  into memory in one go and closed immediately; what Exult gets back is a
 *  stream over that buffer. This also makes the SD card access pattern
 *  (one big read instead of thousands of tiny seeks) a lot friendlier.
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
			off_type base;
			switch (dir) {
			case std::ios_base::beg:
				base = 0;
				break;
			case std::ios_base::cur:
				base = gptr() - eback();
				break;
			case std::ios_base::end:
				base = egptr() - eback();
				break;
			default:
				return pos_type(off_type(-1));
			}
			const off_type np = base + off;
			if (np < 0 || np > egptr() - eback()) {
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

	class Memistream : public std::istream {
	public:
		explicit Memistream(std::vector<char>&& d) : std::istream(nullptr), buf(std::move(d)) {
			rdbuf(&buf);
		}

	private:
		Membuf buf;
	};

	int open_handles() {
		int n = 0;
		for (int fd = 0; fd < 1024; fd++) {
			if (__get_handle(fd) != nullptr) {
				n++;
			}
		}
		return n;
	}

}    // namespace

std::unique_ptr<std::istream> n3ds_open_in(const char* name, std::ios_base::openmode mode) {
	(void)mode;    // Always binary on the 3DS; there is no text translation.
	FILE* f = std::fopen(name, "rb");
	if (f == nullptr) {
		if (errno != ENOENT) {
			std::cerr << "3DS: open failed for " << name << ": " << std::strerror(errno) << " (errno " << errno
					  << "), open handles " << open_handles() << std::endl;
		}
		return nullptr;
	}
	if (std::fseek(f, 0, SEEK_END) != 0) {
		std::fclose(f);
		return nullptr;
	}
	const long size = std::ftell(f);
	if (size < 0) {
		std::fclose(f);
		return nullptr;
	}
	std::rewind(f);
	std::vector<char> data(static_cast<size_t>(size));
	size_t            got = 0;
	while (got < data.size()) {
		const size_t n = std::fread(data.data() + got, 1, data.size() - got, f);
		if (n == 0) {
			break;
		}
		got += n;
	}
	std::fclose(f);
	if (got != data.size()) {
		std::cerr << "3DS: short read on " << name << " (" << got << " of " << size << ")" << std::endl;
		return nullptr;
	}
	return std::make_unique<Memistream>(std::move(data));
}

void n3ds_fs_report(const char* when) {
	const struct mallinfo mi = mallinfo();
	std::cout << "3DS fs (" << when << "): " << open_handles() << " open handles, heap in use " << (mi.uordblks >> 10)
			  << " KB; devices:";
	for (int i = 0; i < STD_MAX; i++) {
		if (devoptab_list[i] != nullptr && devoptab_list[i]->name != nullptr) {
			std::cout << ' ' << devoptab_list[i]->name;
		}
	}
	std::cout << std::endl;
}

#endif    // __3DS__
