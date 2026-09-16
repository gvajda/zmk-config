# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

ZMK firmware config for a **Piantor Pro BT** — a 36-key split Bluetooth keyboard (3×5 + 3 thumbs per half). ZMK v0.3.

## Build

Firmware is built exclusively via **GitHub Actions** — there is no local build toolchain. Push to `main` or open a PR to trigger the CI build. The workflow delegates to the upstream reusable workflow:

```
.github/workflows/build.yml  →  zmkfirmware/zmk/.github/workflows/build-user-config.yml@v0.3
```

`build.yaml` at repo root defines the GitHub Actions matrix: left half (with ZMK Studio via the `studio-rpc-usb-uart` snippet — Studio only runs on the central half), right half, `settings_reset` images for both halves, and a left-half debug image with the `zmk-usb-logging` snippet (one snippet per target is all the reusable workflow passes, so the debug image has no Studio USB transport).

To force a build without a code change: use the **workflow_dispatch** trigger in the GitHub Actions UI.

## Key files

| File | Purpose |
|------|---------|
| `config/piantor_pro_bt.keymap` | All layers, behaviors, combos — the main edit target |
| `config/piantor_pro_bt.conf` | Kconfig overrides: keyboard name, sleep timeout, RGB defaults, per-half battery |
| `config/west.yml` | ZMK module manifest; pins ZMK to `v0.3` |
| `build.yaml` | CI matrix (left/right halves, settings_reset, left USB-logging debug image) |
| `docs/hrm-timing.html` | Local page logging key down/up timings, for tuning hold-taps (see README "Debugging misfires") |
| `boards/arm/piantor_pro_bt/` | Vendored board definition (DTS, Kconfig, layouts) — keep identical to upstream |
| `README.md` | Landing page: auto-generated keymap diagram + layer reference |
| `keymap_drawer.config.yaml` | keymap-drawer styling/glyph config for the diagram |
| `docs/img/piantor_pro_bt.svg` | Generated keymap diagram (do not hand-edit) |

## Keymap architecture

The keymap selects the board's 36-key `five_col_layout` (`chosen { zmk,physical-layout = &five_col_layout; }`), so every layer has exactly 36 bindings and key positions are:

```
row 0:  0- 4 left   5- 9 right
row 1: 10-14 left  15-19 right
row 2: 20-24 left  25-29 right
thumbs: 30 31 32 left  33 34 35 right
```

`KEYS_L`, `KEYS_R`, `THUMBS` macros at the top of the keymap hold these lists for positional hold-taps.

Five layers, all reached via the 6-key thumb cluster (positions 30–35):

| Layer | Activation |
|-------|------------|
| SYM (1) | Hold inner thumbs (TAB / BSPC) |
| BRC (2) | Hold middle thumbs (both SPACE) |
| FN (3) | Hold outer thumbs (ESC / DEL) |
| KBD (4) | Combo: squeeze ESC **and** DEL simultaneously (positions 30+35) |

**Home-row mods** use custom `hold-tap` behaviors (balanced, positional, `hold-trigger-on-release`):
- `hml` (left hand) — only fires hold when a right-hand or thumb key follows
- `hmr` (right hand) — only fires hold when a left-hand or thumb key follows
- `hsl` / `hsr` — shift-only variants: `tapping-term-ms` 175, `retro-tap`, no `require-prior-idle-ms`; values come from a logged typing session (comment in the keymap explains each)
- Order on both hands (pinky→index): Ctrl, Alt, Cmd (LGUI), Shift — i.e. CACS/SCAC

Top-row keys use `hd` (hold_desktop): tap = letter, hold = Ctrl+N for macOS desktop switching.

**KBD combo** activates the keyboard management layer (BT profile select, RGB toggle, bootloader, ZMK Studio unlock).

## Vendored board

The `piantor_pro_bt` board definition lives in `boards/arm/piantor_pro_bt/` instead of a west module. This is intentional — it was vendored (from `Keebart/zmk-config` @ `01556a8`) to avoid depending on a third-party west module. Keep it byte-identical to upstream; user preferences go in `config/piantor_pro_bt.conf`, not in the board defconfigs. Do not move it back to a west module without updating `config/west.yml`.

Note: ZMK after v0.3 moves to Zephyr hardware model v2 (`boards/<vendor>/` + `board.yml`); the vendored board will need converting when ZMK is bumped.

## Power / RGB

- Deep sleep after 2 h idle (`CONFIG_ZMK_IDLE_SLEEP_TIMEOUT=7200000`); default 15 min was too eager — wake from deep sleep needs a multi-second BLE reconnect.
- `CONFIG_ZMK_RGB_UNDERGLOW=y` but `CONFIG_ZMK_RGB_UNDERGLOW_ON_START=n` — lights are compiled in but off at boot to preserve battery. Toggle from the KBD layer.
- `CONFIG_ZMK_RGB_UNDERGLOW_AUTO_OFF_IDLE=y` is a battery guard, keep it: EXT_POWER (the 21-LED strip's supply) defaults ON at boot and after `settings_reset`; ZMK only switches it off inside `rgb_underglow_off()`, which AUTO_OFF_IDLE triggers at the first idle. Without it the dark strip draws ~15 mA per half (drained 500 mAh in ~4 days, Sep 2026).
- Per-half battery: `..._BATTERY_LEVEL_FETCHING` + `..._PROXY` expose the right half's level as a second BLE Battery Service for menu bar widgets. Keep these.

## Upstream sync check

Last upstream check: 2026-09-11

Do this every ~3 months. A SessionStart hook (`scripts/upstream-check-nudge.sh`, wired in `.claude/settings.json`) prints a nudge when the date above is older than 90 days. After the check, bump the date above — that is the only state.

1. **Vendor board vs Keebart.** Clone `https://github.com/Keebart/zmk-config`, then:
   - `git diff <pinned> HEAD -- boards/arm/piantor_pro_bt` in the clone to see what changed since the pinned commit (currently `01556a8`).
   - `diff -r <clone>/boards/arm/piantor_pro_bt boards/arm/piantor_pro_bt` must be empty at the pinned commit. If adopting a newer commit, copy the whole dir, update the pinned hash in "Vendored board" above, and keep prefs in `config/piantor_pro_bt.conf`.
   - Skim their `config/piantor_pro_bt*.conf`, `build.yaml`, README for new recommended Kconfig (they added e.g. `CONFIG_ZMK_IDLE_TIMEOUT`, `..._AUTO_OFF_IDLE`, `_5col` targets after we vendored).
2. **ZMK version.** Check `https://github.com/zmkfirmware/zmk/releases`. If a new tag exists: bump `revision` in `config/west.yml` and `@vX.Y` in `.github/workflows/build.yml` together, read the release notes for breaking changes (hardware model v2 will require converting `boards/arm/` — see "Vendored board"), and diff `https://github.com/zmkfirmware/zmk-config` template's `build.yml`/`west.yml` against ours.
3. **Behavior/config docs.** Re-read `https://zmk.dev/docs/keymaps/behaviors/hold-tap`, `/docs/features/studio`, `/docs/config/battery`, `/docs/config/underglow` for renamed or deprecated properties/Kconfig used here.
4. Push, confirm CI builds all four targets, flash both halves.

## ZMK Studio

Studio is enabled on the left half only. A keymap saved via Studio overrides the compiled keymap until "Restore Stock Settings" in Studio or a `settings_reset` flash — if a flashed keymap change does not show up, suspect this first.
