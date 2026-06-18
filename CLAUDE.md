# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

ZMK firmware config for a **Piantor Pro BT** — a 36-key (42-position matrix, outer pinky column unpopulated) split Bluetooth keyboard. ZMK v0.3.

## Build

Firmware is built exclusively via **GitHub Actions** — there is no local build toolchain. Push to `main` or open a PR to trigger the CI build. The workflow delegates to the upstream reusable workflow:

```
.github/workflows/build.yml  →  zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3
```

`build.yaml` at repo root defines the GitHub Actions matrix (left half, right half, each with ZMK Studio enabled via the `studio-rpc-usb-uart` snippet).

To force a build without a code change: use the **workflow_dispatch** trigger in the GitHub Actions UI.

## Key files

| File | Purpose |
|------|---------|
| `config/piantor_pro_bt.keymap` | All layers, behaviors, combos — the main edit target |
| `config/piantor_pro_bt.conf` | Kconfig overrides (RGB off at boot) |
| `config/west.yml` | ZMK module manifest; pins ZMK to `v0.3` |
| `build.yaml` | CI matrix (left/right halves, cmake args) |
| `boards/arm/piantor_pro_bt/` | Vendored board definition (DTS, Kconfig, layouts) |
| `docs/keymap.md` | Human-readable visual layer reference — keep in sync with keymap |

## Keymap architecture

Five layers, all reached via the 6-key thumb cluster (positions 36–41):

| Layer | Activation |
|-------|------------|
| SYM (1) | Hold inner thumbs (TAB / BSPC) |
| BRC (2) | Hold middle thumbs (both SPACE) |
| FN (3) | Hold outer thumbs (ESC / RET) |
| KBD (4) | Combo: squeeze ESC **and** RET simultaneously |

**Home-row mods** use two custom `hold-tap` behaviors:
- `hml` (left hand) — positional: only fires hold when a right-hand or thumb key follows
- `hml` (right hand) — positional: only fires hold when a left-hand or thumb key follows
- Order on both hands (pinky→index): Ctrl, Alt, Cmd (LGUI), Shift — i.e. CACS/SCAC

Top-row keys use `hd` (hold_desktop): tap = letter, hold = Ctrl+N for macOS desktop switching.

**KBD combo** (positions 36+41, outer thumbs) activates the keyboard management layer (BT profile select, RGB toggle, bootloader, ZMK Studio unlock).

## Vendored board

The `piantor_pro_bt` board definition lives in `boards/arm/piantor_pro_bt/` instead of a west module. This is intentional — it was vendored to avoid depending on a third-party west module. Do not move it back to a west module without updating `config/west.yml`.

The matrix has 42 positions; 6 are unused (`&none` at positions 0, 11, 12, 23, 24, 35 — the outer pinky column). Every layer row must include these padding `&none` entries.

## RGB

`CONFIG_ZMK_RGB_UNDERGLOW=y` but `CONFIG_ZMK_RGB_UNDERGLOW_ON_START=n` — lights are compiled in but off at boot to preserve battery. Toggle from the KBD layer.
