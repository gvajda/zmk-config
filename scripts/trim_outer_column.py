#!/usr/bin/env python3
"""Trim the unpopulated outer-pinky column from a keymap-drawer YAML.

The Piantor Pro BT is a 42-position matrix wired as a 36-key board: the outer
column (matrix positions 0, 11, 12, 23, 24, 35) is unpopulated and padded with
`&none` in the keymap. keymap-drawer parses all 42 positions, which draws as a
3x6 grid with an empty column. Removing those 6 positions lets the keymap be
drawn against the board's real 36-key `five_col_layout`, i.e. a true 3x5+3.

Reads YAML on stdin, writes trimmed YAML to stdout. Also renumbers combo key
positions, which are indices into the (now shorter) key list.
"""
import sys

import yaml

# Matrix positions of the unpopulated outer column (see CLAUDE.md / board DTS).
REMOVE = {0, 11, 12, 23, 24, 35}


def reindex(pos: int) -> int:
    """Map an old 42-key index to its index after REMOVE positions are dropped."""
    if pos in REMOVE:
        raise ValueError(f"combo references removed position {pos}")
    return pos - sum(1 for r in REMOVE if r < pos)


def main() -> None:
    data = yaml.safe_load(sys.stdin)

    for name, layer in data.get("layers", {}).items():
        if len(layer) != 42:
            raise SystemExit(f"layer {name!r} has {len(layer)} keys, expected 42")
        data["layers"][name] = [k for i, k in enumerate(layer) if i not in REMOVE]

    for combo in data.get("combos", []):
        combo["p"] = [reindex(p) for p in combo["p"]]

    yaml.safe_dump(data, sys.stdout, allow_unicode=True, sort_keys=False)


if __name__ == "__main__":
    main()
