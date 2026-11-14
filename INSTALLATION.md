# Installation

This port produces **one `.dol` per game** from one source tree. Each is a
standalone GameCube executable. None of them include the copyrighted game
data; you provide your own `.pk3` files, bought legally, and copy them to an
SD card.

> **Pre-alpha.** No release exists yet. This document describes the layout
> the port is built for.

## What each build is

| Build | What it is | Notable features |
|---|---|---|
| **ioQuake3** | Quake III Arena | Single player with bots, LAN/online multiplayer through the Broadband Adapter |
| **Open Arena** | Free/open Q3A-compatible game | Uses free OA game data — no retail purchase needed. Planned after Quake III Arena is done. |

## Release package

`make dist` builds the release layout in `dist/ioQuake3-GC/`:

```
dist/ioQuake3-GC/ioquake3_gc.dol      (ioquake3_oa_gc.dol for Open Arena)
dist/ioQuake3-GC/README.md
dist/ioQuake3-GC/INSTALLATION.md
dist/ioQuake3-GC/LICENSE
dist/ioQuake3-GC/THIRD_PARTY_NOTICES.md
```

## What you need

- A GameCube that can start homebrew: [Swiss](https://github.com/emukidid/swiss-gc)
  through PicoBoot, a GC Loader, a modchip, or an SD Media Launcher.
- An SD adapter: **SD2SP2** (Serial Port 2, recommended, the fastest) or an
  **SD Gecko** (memory card slot A or B).
- An SD card formatted as **FAT32**.
- A GameCube controller. A WaveBird works, without rumble.
- Optional: a Broadband Adapter for multiplayer.

## Where to get the required files

Buy the base game legally, then copy the `.pk3` files from your install/disc
into the paths below.

- **Quake III Arena** (needed for ioQuake3): [Here](https://www.gog.com/en/game/quake_iii_arena)
- **Open Arena** [Here](https://openarena.ws/)

---

## Common layout

All builds share one game-data root on the SD card, `quake3/`. It is the same
root the Wii port uses, so one card serves both consoles.

```
<dev>:/ioquake3_gc.dol
<dev>:/quake3/<game dir(s) below>
```

`<dev>` is `sd2sp2:` (SD2SP2), `carda:` (SD Gecko in slot A) or `cardb:`
(SD Gecko in slot B). The game uses the device it was started from, then
looks at the others.

---

## ioQuake3 (Q3A)

File: `ioquake3_gc.dol`

Copy your Quake III Arena `baseq3` paks:

```
<dev>:/quake3/baseq3/pak0.pk3 … pak8.pk3
```

## Open Arena

File: `ioquake3_oa_gc.dol`

Copy Open Arena's paks — no `baseq3` needed:

```
<dev>:/quake3/baseoa/pak0.pk3 …
```

---

## Starting the game

### Swiss

1. Copy the `.dol` to the SD card.
2. Copy the game data from the sections above into `<dev>:/quake3/`.
3. Start Swiss, browse to the `.dol` and start it.

Swiss passes the path the game was started from. The game uses that card
first, then the other cards.

### PicoBoot (autoboot)

Rename the `.dol` to `ipl.dol` and put it in the root of the SD card.

### SD Media Launcher

Rename the `.dol` to `autoexec.dol` and put it in the root of the SD card.

## Settings and progress

Settings and single-player progress are saved to
`<dev>:/quake3/baseq3/q3config.cfg` when they change and on quit. Nothing is
saved when the game runs from a disc image.

## Quitting

Z + START or the menu's Quit, or the console's Reset button, save and quit.
The game returns to the loader if the loader left a return stub; otherwise the
console resets. With PicoBoot a reset starts `/ipl.dol` again, so the game
restarts.

## Troubleshooting

- **"No storage" message at boot:** the card is not FAT32, or the adapter is
  not seated. Try the other memory card slot.
- **"Could not find pak0.pk3":** check the path `<dev>:/quake3/baseq3/pak0.pk3`.
- **Wrong settings after an update:** delete `q3config.cfg` in
  `<dev>:/quake3/baseq3/` — a stale config can override the correct defaults.
- **Picture problems:** the game takes the video mode from the console
  settings. 480p needs the component cable or a digital-port adapter.
