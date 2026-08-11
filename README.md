# TF2 Elysium

Fan-engine recreation project targeting the feel, structure, and class-based mayhem of the original title, built on the Twilight Elysium engine.

> This is an independent fan project. It is not affiliated with, endorsed by, or connected to Valve or Team Fortress 2 in any way.

## Goals
- Rebuild the 9 classes and core game modes (CTF, payload, arena, King of the Hill) in Elysium
- Tool-assisted asset generation and audio-reactive rendering
- Local-only engine runtime with optional editor tooling

## Layout
- `Content/` — maps, data scripts, generated assets
- `Source/` — engine + game modules
- `tools/` — asset pipeline, dream engine, launcher
- `tests/` — smoke and regression tests
- `docs/` — design notes

## Difficulties
Identical casual/legend profiles to every other Elysium build (`Content/Data/Difficulties.json`):
- `casual` — forgiving damage, extra ammo, fast respawns
- `legend` — lethal enemies, scarce supplies, no mercy

```bash
python3 tools/tf2_dream.py casual   # relaxed dream run
python3 tools/tf2_dream.py legend   # brutal dream run
```

## Secrets
There is a coconut somewhere. Every map. Carry it, throw it, or taunt with it.
Nobody knows why it's there. That's the point. (See `Content/Data/EasterEggs.json`.)

## Build
TBD based on engine language choice; current prototype uses Python renderer + optional C++ backend.
