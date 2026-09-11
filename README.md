# Piantor Pro BT — ZMK config

ZMK firmware config for a 36-key **Piantor Pro BT** split keyboard (macOS modifiers, ZMK v0.3). Five layers, all reached from the thumb cluster.

![Piantor Pro BT keymap](docs/img/piantor_pro_bt.svg)

> Auto-generated from [`config/piantor_pro_bt.keymap`](config/piantor_pro_bt.keymap)
> by [keymap-drawer](https://github.com/caksoylar/keymap-drawer) on every push
> (see [`.github/workflows/draw-keymap.yml`](.github/workflows/draw-keymap.yml)).
> **Edit the keymap, not this image.** In each key the large legend is the tap and
> the small legend is the hold — a modifier (⌃ ⌥ ⌘ ⇧) or a layer name. A highlighted
> thumb marks the key whose hold activates that layer.

## Layer activation

| Layer | Hold                        |
|-------|-----------------------------|
| SYM   | tab or bspc (inner thumbs)  |
| BRC   | space (both middle thumbs)  |
| FN    | esc or del (outer thumbs)   |
| KBD   | esc **and** del together    |

Home-row mods (pinky→index): Ctrl, Alt, Cmd, Shift. Top-row holds = Ctrl+1..5 (macOS desktop switch).
SYM puts numbers on the home row (with the same mods) and their shifted symbols on the top row.

## Notes

- **BRC enter:** hold a space thumb, then tap either outer thumb → enter (base gives the
  outer thumbs to esc/del; BRC remaps both to enter so it is reachable from either hand).
- **Clipboard (FN):** ⌘A/Z/X/C/V and ^⇧Z (clipboard manager) sit on the left hand — hold
  **del** (right thumb) for two-handed use, or **esc** (left thumb) for one-handed.
- **Sleep:** deep sleep after 2 h idle (ZMK default 15 min) — waking from deep sleep
  needs a BLE reconnect that takes seconds; see [`config/piantor_pro_bt.conf`](config/piantor_pro_bt.conf).

## Build

No local toolchain — firmware builds on **GitHub Actions**. Push to `main` or open a PR,
or trigger the build manually via `workflow_dispatch`. See [CLAUDE.md](CLAUDE.md) for the
repo layout and build details.
