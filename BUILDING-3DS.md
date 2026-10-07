# Building Exult for the Nintendo 3DS

Technical notes for the `n3ds` branch. The result is `exult.3dsx`, run from
the Homebrew Launcher on a New 3DS / New 2DS XL.

## Toolchain

- devkitARM (arm-none-eabi-gcc), libctru, citro3d — devkitPro
- SDL 3.4 built for the 3DS (`-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/3DS.cmake`),
  installed into `$DEVKITPRO/portlibs/3ds`
- zlib, libogg, libvorbis built the same way into `portlibs/3ds`
- a native build of Exult's `expack` tool on the host (it generates the data
  flex files during the cross build)
- `3dsxtool` and `smdhtool` from devkitPro's general-tools

## Build

```sh
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export PATH=$DEVKITPRO/tools/bin:$DEVKITARM/bin:/path/to/host/expack:$PATH

./autogen.sh
mkdir build-3ds && cd build-3ds
../configure --host=arm-none-eabi \
  CXXFLAGS="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -O2 -mword-relocations -ffunction-sections -fdata-sections" \
  LDFLAGS="-specs=3dsx.specs -L$DEVKITPRO/portlibs/3ds/lib -L$DEVKITPRO/libctru/lib" \
  SDL3_CFLAGS="-I$DEVKITPRO/portlibs/3ds/include" SDL3_LIBS="-lSDL3 -lctru -lm" \
  --disable-tools --disable-exult-studio --disable-timidity-midi --disable-alsa \
  --disable-fluidsynth --disable-mt32emu --with-usecode-debugger=no
make -j4

# package: the data directory goes into romfs
mkdir -p romfs && cp -r data romfs/data
smdhtool --create "Exult (New 3DS only)" "Ultima VII engine - needs a New 3DS / New 2DS XL" "cherygarcia77" icon.png exult.smdh
3dsxtool exult exult.3dsx --smdh=exult.smdh --romfs=romfs
```

## Where the 3DS code lives

- `n3ds_kbd.cc/.h` — the second screen: touch keyboard, or a mirror of the game
- `n3ds_heap.cc` — memory set-up (stack, linear heap, application heap)
- `n3ds_fs.cc/.h` — game files are read into memory (the 3DS has a small
  limit on open file handles)
- `exult.cc` — start-up (SD card folders, logs, New 3DS check), gamepad to
  mouse/keyboard mapping, screen swap
- `imagewin/imagewin.cc` — window creation on either screen

Runtime layout on the SD card is described in `README-3DS.txt`.
