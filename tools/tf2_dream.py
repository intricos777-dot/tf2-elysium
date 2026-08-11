#!/usr/bin/env python3
"""
TF2 Elysium Dream Launcher
============================
Boots the Twilight Elysium dream engine with Team Fortress 2-themed data and scripts.
Supports casual/legend difficulties. Sheldon pops up at most 3 times, ever, per player.
"""
from __future__ import annotations

import json
import random
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
ENGINE_TOOLS = REPO_ROOT / "tools" / "twilight-elysium" / "tools"
GAME_CONTENT = REPO_ROOT / "Content"
STATE_FILE = Path.home() / ".local" / "share" / "tf2-elysium" / "sheldon_state.json"
MAX_SHELDON_POPS = 3

if str(ENGINE_TOOLS) not in sys.path:
    sys.path.insert(0, str(ENGINE_TOOLS))

from dream import ContentDreamEngine
from dream_engine import DreamGenerator


def load_state() -> dict:
    if STATE_FILE.exists():
        try:
            return json.loads(STATE_FILE.read_text())
        except Exception:
            pass
    return {"maxPops": MAX_SHELDON_POPS, "players": {}}


def save_state(state: dict) -> None:
    STATE_FILE.parent.mkdir(parents=True, exist_ok=True)
    STATE_FILE.write_text(json.dumps(state, indent=2))


def pop_sheldon(player: str = "local") -> dict:
    """Increment Sheldon's pop count for a player; returns the event."""
    state = load_state()
    players = state.setdefault("players", {})
    entry = players.setdefault(player, {"pops": 0})
    entry["pops"] += 1
    state["maxPops"] = MAX_SHELDON_POPS
    save_state(state)
    return {"player": player, "totalPops": entry["pops"], "remaining": MAX_SHELDON_POPS - entry["pops"]}


class TF2DreamEngine(ContentDreamEngine):
    """Dream engine preset for the Team Fortress 2 theme."""

    def __init__(self, difficulty: str = "casual"):
        super().__init__()
        self.npcs: list[dict] = []
        self.dream_generator = DreamGenerator()
        self.dream_generator.script_state.weapons = []
        self.dream_generator.script_state.enemies = []
        self.dream_generator.script_state.locations = []
        self.dream_generator.load_scripts()
        if not self.dream_generator.script_state.weapons and not self.dream_generator.script_state.enemies:
            print("[TF2] No script data loaded; Content/Data JSONs are optional.")
        self._load_game_data()
        self.difficulty = self._load_difficulty(difficulty)

    def _load_difficulty(self, name: str) -> dict:
        f = GAME_CONTENT / "Data" / "Difficulties.json"
        if not f.exists():
            return {"profile": name, "settings": {}}
        data = json.loads(f.read_text())
        profiles = data.get("profiles", {})
        name = name if name in profiles else data.get("default", "casual")
        return {"profile": name, "settings": profiles.get(name, {})}


    def _load_game_data(self) -> None:
        """Load this game's own Content/Data JSONs into the dream state."""
        data_dir = GAME_CONTENT / "Data"
        for f in sorted(data_dir.glob("*.json")):
            if f.name == "Difficulties.json":
                continue
            try:
                items = json.loads(f.read_text())
            except Exception:
                continue
            if not isinstance(items, list):
                continue
            low = f.stem.lower()
            if "weapon" in low:
                self.dream_generator.script_state.weapons.extend(items)
            elif "enem" in low or "class" in low or "mode" in low:
                self.dream_generator.script_state.enemies.extend(items)
            elif "map" in low:
                self.dream_generator.script_state.locations.extend(
                    {"name": m.get("name", f.stem), "data": m} for m in items
                )
            elif "npc" in low:
                self.npcs.extend(items)


def sheldon_popup(sheldon: dict, player: str) -> None:
    """Randomly pop Sheldon into the dream run (at most 3 times ever)."""
    state = load_state()
    pops = state.setdefault("players", {}).get(player, {}).get("pops", 0)
    if pops >= MAX_SHELDON_POPS:
        print("[TF2] Sheldon does not appear today. He has said all he had to say. Three times. Ever.")
        return

    popup = sheldon.get("popup", {})
    lo, hi = popup.get("minIntervalSeconds", 30), popup.get("maxIntervalSeconds", 120)
    when = random.randint(lo, hi)
    quote = random.choice(sheldon.get("quotes", ["I like turtles"]))
    print(f"\n[t={when}s] SHELDON POPS UP OUT OF NOWHERE:")
    print(f"  \"{quote}\"")
    print("  (he looks at you. then he is gone, as if never there.)\n")
    ev = pop_sheldon(player)
    if ev["remaining"] > 0:
        print(f"[TF2] Sheldon appearances remaining for {ev['player']}: {ev['remaining']} (ever)")
    else:
        print(f"[TF2] That was Sheldon's last appearance. Ever. For anyone.")


def main() -> int:
    print("[TF2] Starting Team Fortress 2 Elysium Dream Engine...")
    difficulty = "casual"
    if len(sys.argv) > 1 and sys.argv[1] in ("casual", "legend"):
        difficulty = sys.argv[1]
    player = sys.argv[2] if len(sys.argv) > 2 else "local"
    engine = TF2DreamEngine(difficulty=difficulty)
    diff = engine.difficulty
    print(f"[TF2] Difficulty: {diff['profile']}")
    if diff["settings"]:
        print(f"[TF2]   player  -> {diff['settings'].get('player')}")
        print(f"[TF2]   combat  -> {diff['settings'].get('combat')}")

    scene = engine.dream()
    print(f"[TF2] Generated scene with {len(scene.objects)} objects")

    sheldon = next((n for n in engine.npcs if n.get("name") == "Sheldon"), None)
    if sheldon:
        sheldon_popup(sheldon, player)
    else:
        print("[TF2] Sheldon is not in this build. This is wrong and should be investigated.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
