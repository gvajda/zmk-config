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

- **BRC enter:** hold a space thumb, then tap the right outer thumb → enter. (BRC remaps
  that thumb to enter — restoring it, since base gives the outer thumb to del.)
- **Clipboard (FN):** ⌘A/Z/X/C/V and ^⇧Z (clipboard manager) sit on the left hand — hold
  **del** (right thumb) for two-handed use, or **esc** (left thumb) for one-handed.
- **Firmware:** 42-position board with the outer pinky column unpopulated; the keymap
  pads each row with `&none` (matrix positions 0, 11, 12, 23, 24, 35). The drawing trims
  that column and renders the real 36-key 3×5+3 shape.

## Build

No local toolchain — firmware builds on **GitHub Actions**. Push to `main` or open a PR,
or trigger the build manually via `workflow_dispatch`. See [CLAUDE.md](CLAUDE.md) for the
repo layout and build details.
