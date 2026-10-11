# Obsession — Aquatic Adventure

A C rewrite of the "Aquatic Adventure" pinball table from *Obsession*, released
on Atari ST and Amiga in 1994 and 1995 (Unique Development Studios AB).

The original game data is required and is **not** included in this repository.
*Obsession*, the table artwork, sounds and music remain property of their
original copyright holders.

## Building

Requires a C compiler and SDL2.

```
cd src
make
```

## Running

The original data file from the Amiga release is required:

```
AQUA.BIN - 57c22fbaf4eacd99e662fe8c9b4089726e0e47ad
```

Place `AQUA.BIN` in the working directory (or pass `--filepath`):

```
./aqua [OPTIONS]

  --filepath=PATH   Path to .BIN file (default is 'AQUA.BIN')
  --scrolling       Enable vertical scrolling
  --fullscreen      Stretch window resolution to desktop
```

## Controls

| Key | Action |
|---|---|
| Left Shift | Left flipper |
| Right Shift | Right flipper |
| Down (hold/release) | Pull and release the plunger |
| Space | Nudge the table |
| Esc, then Y | Quit |
| M | Toggle music |
| J | Toggle jingles |
| S | Toggle sound effects |

## Links

* <https://www.mobygames.com/game/12645/obsession/>
