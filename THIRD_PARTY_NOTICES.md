# Third-party notices

ioQuake3-GC is under the GNU General Public License, version 2 (`LICENSE`). It is built from the projects below. Each
keeps its own licence. This file ships beside every `.dol` in a release.

QGC-5 vendored the engine baseline into `code/` from old-mac-quake3 commit
`2f187c09c8a816bb62d5b7ccdbee24c65a1581fb`, unchanged. QGC-8 added `code/ui/ui_public.h` from the same commit,
unchanged. QGC-10 added `code/ui/ui_syscalls.c` from the same commit, unchanged, and moved `menudef.h` back to the
baseline path `ui/menudef.h`.

## Summary

| Component | Where | Licence | Linked into the `.dol` |
|---|---|---|---|
| ioquake3 (id Software, ioquake3 contributors), through old-mac-quake3 | `code/` (vendored) | GPL-2.0 | Yes |
| old-mac-quake3 changes (Matthew Deaves) | `code/` (vendored) | GPL-2.0 | Yes |
| ioQuake3-wii GX renderer, input and audio layers | `code/` (ported parts) | GPL-2.0 | Yes |
| Quake3e zone allocator | `code/qcommon/common.c` (`HW_DOL` only) | GPL-2.0-or-later | Yes |
| q3ce (Quake 3 Arena CE) | not vendored, reference for memory cuts | GPL-2.0 | Only if code is copied |
| zlib 1.2.3 (Jean-loup Gailly and Mark Adler) | `code/zlib/` | zlib License | Yes |
| minizip `unzip.c` (Gilles Vollant) | `code/qcommon/unzip.c` | zlib License | Yes |
| `puff.c` (Mark Adler) | `code/qcommon/puff.c` | zlib License | Yes |
| libjpeg 8c (Independent JPEG Group) | `code/jpeg-8c/` | IJG licence | Yes |
| MD4 (Andrew Tridgell) | `code/qcommon/md4.c` | GPL-2.0-or-later | Yes |
| MD5 (Colin Plumb) | `code/qcommon/md5.c` | Public domain | Yes |
| Intel/DVI ADPCM coder (Stichting Mathematisch Centrum) | `code/client/snd_adpcm.c` | CWI permissive notice | Yes |
| libogc 3.2.1 (Michael Wiedenbauer, Dave Murphy) | toolchain image | zlib-style licence | Yes (`libogc.a`, `libasnd.a`, `libbba.a`) |
| libfat-ogc 2.1.0 (devkitPro) | toolchain image | as stated in its headers | Yes (`libfat.a`) |
| GCC runtime (libgcc) | toolchain image | GPL-3.0 with the GCC Runtime Library Exception | Yes |
| newlib | toolchain image | `COPYING.NEWLIB` (permissive) | Yes |
| devkitPPC r51 (GCC 16.2, binutils 2.47), gamecube-tools | toolchain image | GPL-3.0-or-later | No (build tools) |

No Nintendo SDK material is used. No game data is included.

## ioquake3, old-mac-quake3, ioQuake3-wii and q3ce

id Software released the Quake III Arena source code under the GPL, version 2. ioquake3, old-mac-quake3, ioQuake3-wii
and q3ce carry that licence. The full text is in `LICENSE`. Their source files keep their own copyright headers.
The Quake III Arena and OpenArena **game data** is not covered and is not included.

- ioquake3: https://github.com/ioquake/ioq3. The baseline is commit `4432a80a` (2013-01-17), the last SDL 1.2 commit.
- old-mac-quake3: https://github.com/matthewdeaves/old-mac-quake3, commit
  `2f187c09c8a816bb62d5b7ccdbee24c65a1581fb`. This is the engine baseline of the port.
- ioQuake3-wii: https://github.com/Mayo1970/ioQuake3-wii, commit `9b9ad24bfae38a9951066906edf7a9942ec93d2d`.
  Source of the native GX renderer backend, the GameCube pad input and the audio backend.
- q3ce: http://q3ce.googlecode.com/svn (Quake 3 Arena CE 1.1, based on Quake III Arena 1.32b). Reference only.

## Quake3e

The `HW_DOL` zone allocator in `code/qcommon/common.c` is adapted from Quake3e,
https://github.com/ec-/Quake3e, commit `96a2e530`, licensed GPL-2.0-or-later.

## zlib, minizip and puff

zlib 1.2.3 comes with the ioquake3 baseline. The licence text is from `zlib.h`:

```
Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Jean-loup Gailly        Mark Adler
jloup@gzip.org          madler@alumni.caltech.edu
```

`unzip.c` is Copyright (C) 1998-2005 Gilles Vollant. `puff.c` is Copyright (C) 2002-2004 Mark Adler. Both use the
same terms.

## libjpeg 8c

This software is based in part on the work of the Independent JPEG Group.

The jpeg-8c sources come with the ioquake3 baseline. Copyright (C) 1991-2011, Thomas G. Lane, Guido Vollbeding.
The IJG `README` with its copyright and no-warranty notice stays in `code/jpeg-8c/README`, unchanged.

## ADPCM coder

`snd_adpcm.c` keeps this notice:

```
Copyright 1992 by Stichting Mathematisch Centrum, Amsterdam, The
Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.

STICHTING MATHEMATISCH CENTRUM DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH CENTRUM BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```

## libogc

libogc 3.2.1 comes from the `devkitpro/devkitppc` Docker image. The licence text is from `gccore.h`:

```
Copyright (C) 2004
Michael Wiedenbauer (shagkur)
Dave Murphy (WinterMute)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you
   must not claim that you wrote the original software. If you use
   this software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
   must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
```

## Toolchain runtime

The `.dol` links libgcc (GPL-3.0 with the GCC Runtime Library Exception) and newlib (`COPYING.NEWLIB`). The
exception permits distribution of the linked binary under GPL-2.0.
