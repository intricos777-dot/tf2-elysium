#!/usr/bin/env python3
"""Smoke test: launcher boots, loads game data + difficulties, Sheldon limit enforced."""
import json
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]

def test_difficulties_present() -> None:
    diff = json.loads((REPO / "Content" / "Data" / "Difficulties.json").read_text())
    assert "casual" in diff["profiles"] and "legend" in diff["profiles"]
    assert diff["profiles"]["casual"]["player"]["health"] == 150
    assert diff["profiles"]["legend"]["player"]["health"] == 100

def test_sheldon_data_present() -> None:
    npcs = json.loads((REPO / "Content" / "Data" / "TF2_NPCs.json").read_text())
    sheldon = next(n for n in npcs if n["name"] == "Sheldon")
    assert "I like turtles" in sheldon["quotes"]

def test_launcher_boots() -> None:
    for diff in ("casual", "legend"):
        r = subprocess.run([sys.executable, str(REPO / "tools" / "tf2_dream.py"), diff],
                           capture_output=True, text=True)
        assert r.returncode == 0, r.stderr
        assert f"Difficulty: {diff}" in r.stdout

def test_sheldon_three_times_ever(tmp: Path) -> None:
    state = tmp / "sheldon_state.json"
    env = {"HOME": str(tmp)}
    script = str(REPO / "tools" / "tf2_dream.py")
    got_pop = 0
    for _ in range(6):
        r = subprocess.run([sys.executable, script, "casual"], capture_output=True, text=True, env=env)
        assert r.returncode == 0, r.stderr
        if "SHELDON POPS UP" in r.stdout:
            got_pop += 1
    assert got_pop == 3, f"expected exactly 3 pops, saw {got_pop}"
    assert (tmp / ".local" / "share" / "tf2-elysium" / "sheldon_state.json").exists()

if __name__ == "__main__":
    test_difficulties_present()
    test_sheldon_data_present()
    test_launcher_boots()
    with tempfile.TemporaryDirectory() as tmp:
        test_sheldon_three_times_ever(Path(tmp))
    print("tf2-elysium smoke tests pass")