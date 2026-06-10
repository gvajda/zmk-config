# Piantor Pro BT — ZMK Keymap Design

**Date:** 2026-06-10
**Board:** Keebart Piantor Pro BT, 36-key version (5 columns × 3 rows + 3 thumbs, per hand)
**Goal:** Replicate the user's kanata config (`~/.config/kanata/kanata.kbd`) in ZMK, adapted to the
physical keyboard — home-row mods, a numbers/symbols layer, a brackets/nav layer, plus two new
layers made possible by the thumb cluster (function/media and keyboard-hardware control).

---

## 1. Overview

Five layers, all reachable from the thumb cluster:

| # | Layer | Activation (hold) | Purpose |
|---|-------|-------------------|---------|
| 0 | `base` | — | Letters, home-row mods, desktop switching |
| 1 | `sym` | `space` or `enter` (middle thumbs) | Numbers + symbols |
| 2 | `brc` | `tab` or `bspc` (inner thumbs) | Brackets + arrow/word navigation |
| 3 | `fn`  | `esc` or `del` (outer thumbs) | F-keys, media, ⌘ clipboard |
| 4 | `kbd` | `esc` **and** `del` together (combo) | Bluetooth, RGB, Studio, bootloader/reset |

Target OS: **macOS** (⌘ = `LGUI`, ⌥ = `LALT`).

### Thumb cluster

```
left  (outer→inner):  esc   space   tab
right (inner→outer):  bspc  enter   del
```

Each thumb is a layer-tap: tap = the key, hold = its layer. Mirror-symmetric pairing —
inner = `brc`, middle = `sym`, outer = `fn`. Squeezing both outer thumbs (`esc`+`del`) fires a
combo → `kbd`.

---

## 2. Board wiring

Piantor Pro BT is **not** an upstream ZMK shield. Keebart ships the board definition inside their
config repo at `boards/arm/piantor_pro_bt/`, exposed via `zephyr/module.yml` (`board_root: .`).
Consume it as a **west module** (no vendoring; updates with a revision bump).

`config/west.yml`:
```yaml
manifest:
  defaults:
    revision: v0.3
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: keebart
      url-base: https://github.com/Keebart
  projects:
    - name: zmk
      remote: zmkfirmware
      import: app/west.yml
    - name: zmk-config
      remote: keebart
      revision: <pinned-commit-sha>   # pin for reproducible builds
  self:
    path: config
```

`build.yaml`:
```yaml
include:
  - board: piantor_pro_bt_left
    snippet: studio-rpc-usb-uart
    cmake-args: -DCONFIG_ZMK_STUDIO=y
  - board: piantor_pro_bt_right
    snippet: studio-rpc-usb-uart
    cmake-args: -DCONFIG_ZMK_STUDIO=y
```

Our keymap and config live in `config/piantor_pro_bt.keymap` and `config/piantor_pro_bt.conf`,
overriding Keebart's defaults.

### Physical layout / binding count

The board's matrix is **42 positions** (21 per half, direct-scan, 6 thumbs). The board defines two
physical-layout nodes: `default_layout` (42-key, 6 columns) and `five_col_layout` (36-key, 5
columns). The 36-key unit is the same firmware with the two outer columns unpopulated (no switches
there). We keep `chosen { zmk,physical-layout = &default_layout; }` and write **42 bindings per
layer**: the 30 alphas + 6 thumbs carry the design below; the **6 outer-column positions**
(left + right pinky-outer column, matrix positions 0, 11, 12, 23, 24, 35) are `&none` — physically
absent on this unit, so harmless. The §4 maps show the logical 10-wide layout; the firmware file
pads each row with `&none` outer keys.

**Fallback (not chosen):** copy the 13 board files into this repo's `boards/arm/piantor_pro_bt/`
for a zero-dependency, fully self-contained repo. Switch to this only if depending on Keebart's
repo becomes a problem.

---

## 3. Behaviors

### 3.1 Home-row mods (HRM)

Order, pinky → index: **Ctrl, Alt, Gui, Shift** (GACS, from kanata). Left hand uses left-side mods,
right hand uses right-side mods.

| Key | Hold | Key | Hold |
|-----|------|-----|------|
| `a` | LCTRL | `;` | RCTRL |
| `s` | LALT  | `l` | RALT  |
| `d` | LGUI  | `k` | RGUI  |
| `f` | LSHFT | `j` | RSHFT |

Implemented as a `hold-tap` behavior tuned to avoid mis-holds during fast rolls — this replaces
kanata's 24 `defchordsv2` rolling-shift entries (no 1:1 port exists; positional config achieves the
same feel):

```
flavor = "balanced"
tapping-term-ms = 200
quick-tap-ms = 175               # tap-then-hold repeats the tap
require-prior-idle-ms = 150      # rolls into a HRM produce the letter, not the mod
hold-trigger-key-positions = <opposite hand + thumb keys>
hold-trigger-on-release;
```

These same HRM hold-tap behaviors are **replicated on the `sym` and `brc` layers** (per kanata):
on those layers the home-row keys are tap = the layer symbol/nav key, hold = the same modifier.

### 3.2 Desktop switching (base, top row)

`q w e r t` are tap = letter, hold = `Ctrl`+number (`⌃1`…`⌃5`) — placed where the number row used
to be on a full keyboard. Custom `hold-tap` with both bindings as `&kp`:

```
flavor = "tap-preferred"         # standalone hold (no second key); favor the letter on quick taps
tapping-term-ms = 200
require-prior-idle-ms = 150
bindings = <&kp>, <&kp>;         # hold = &kp LC(N1..N5), tap = &kp Q..T
```

**Known risk:** top-row keys with a hold action can mis-fire on fast same-row rolls (e.g. "we",
"rt"). Mitigated by `require-prior-idle-ms` + `tapping-term-ms`; tunable after real-world use.

### 3.3 Thumb layer-taps

`&lt` for each thumb: `esc/fn`, `space/sym`, `tab/brc`, `bspc/brc`, `enter/sym`, `del/fn`.
Balanced flavor with `quick-tap-ms` so a fast double of space/enter repeats the character.

### 3.4 KBD combo

Combo over the two outer-thumb key positions (`esc` + `del`) → `&mo kbd` (layer 4):
```
timeout-ms = 40                  # deliberate squeeze, not sequential presses
require-prior-idle-ms = 150
```
While both thumbs are held the `kbd` layer is active; releasing either exits.

### 3.5 Empty keys

All unassigned positions on non-base layers are **hard-blocked** with `&none` (no fall-through to
base). Exception: the **thumb positions on layers 1–4 stay `&trans`**, so the held layer-key and the
other thumbs keep functioning while a layer is active.

---

## 4. Layer maps

Legend: `X/M` = tap `X`, hold modifier `M`. `∅` = `&none` (hard-blocked). Thumb row `&trans` on
layers 1–4.

### Layer 0 — BASE
```
 q/⌃1   w/⌃2   e/⌃3   r/⌃4   t/⌃5   │   y      u      i      o      p
 a/⌃    s/⌥    d/⌘    f/⇧    g      │   h      j/⇧    k/⌘    l/⌥    ;/⌃
 z      x      c      v      b      │   n      m      ,      .      /
              esc    spc    tab     │   bspc   ent    del
```
Notes:
- `g` and `h` are plain letters. kanata used them as extra `sym` triggers; the thumb cluster makes
  that redundant, so they stay fast.

### Layer 1 — SYM (numbers + symbols; HRM replicated)
```
 1      2      3      4      5      │   6      7      8      9      0
 !/⌃    @/⌥    #/⌘    $/⇧    %      │   ^      &/⇧    */⌘    -/⌥    +/⌃
 `      ~      ∅      ∅      ∅      │   ∅      ∅      ∅      _      =
              ⋯      ⋯      ⋯      │   ⋯      ⋯      ⋯
```
Symbol sources: `! @ # $ % ^ & *` = shifted `1`–`8`; `-` = `MINUS`; `+` = shifted `EQUAL`;
`` ` `` = `GRAVE`; `~` = shifted `GRAVE`; `_` = shifted `MINUS`; `=` = `EQUAL`.

### Layer 2 — BRC (brackets + nav; HRM replicated; redundant keys blocked)
```
 ∅      ⌥←     ↑      ⌥→     ∅      │   ∅      [      ]      ∅      ∅
 ⌘←/⌃   ←/⌥    ↓/⌘    →/⇧    ⌘→     │   ∅      (/⇧    )/⌘    '/⌥    "/⌃
 ∅      ∅      ∅      ∅      ⌃b     │   ∅      {      }      \      |
              ⋯      ⋯      ⋯      │   ⋯      ⋯      ⋯
```
Nav: `⌥←/⌥→` = word left/right (`LA(LEFT/RIGHT)`); `⌘←/⌘→` = line start/end (`LG(LEFT/RIGHT)`);
`↑ ↓ ← →` = arrows. Brackets `[ ] ( ) { }`, quotes `' "`, `\ |`, and `⌃b` (`LC(B)`) retained from
kanata. Removed as redundant (now covered by thumbs / sym): `esc`, `tab`, `bspc`, `del`,
`` ` ``, `~`.

### Layer 3 — FN (F-keys + ⌘ clipboard + media)
```
 F1     F2     F3     F4     F5     │   F6     F7     F8     F9     F10
 ∅      ∅      ∅      ∅      ∅      │   vol-   vol+   🔇     F11    F12
 ⌘Z     ⌘X     ⌘C     ⌘V     ∅      │   ◀◀     ▶❚❚    ▶▶     ∅      ∅
              ⋯      ⋯      ⋯      │   ⋯      ⋯      ⋯
```
- Clipboard: `⌘Z/⌘X/⌘C/⌘V` = `LG(Z/X/C/V)`. Most comfortable held with `del` (right thumb,
  opposite hand from z/x/c/v). `⌘Z` = undo (not the redo of kanata's odd `C-S-z`).
- Media on the right per preference: `vol-`=`C_VOL_DN`, `vol+`=`C_VOL_UP`, `🔇`=`C_MUTE`;
  `◀◀`=`C_PREV`, `▶❚❚`=`C_PP`, `▶▶`=`C_NEXT` (transport placed where BT used to sit).

### Layer 4 — KBD (keyboard hardware; esc+del squeeze)
```
 boot   reset  ∅      ∅      ∅      │   ∅      ∅      ∅      ∅      ∅
 RGBtog RGB-   RGB+   studio ∅      │   BT0    BT1    BT2    BT3    BT4
 ∅      ∅      ∅      ∅      ∅      │   BTclr  ∅      ∅      ∅      ∅
              ⋯      ⋯      ⋯      │   ⋯      ⋯      ⋯
```
- `boot` = `&bootloader`, `reset` = `&sys_reset` (corner-tucked behind a two-thumb squeeze; the
  board also has a physical reset button per the guide).
- `RGBtog/RGB-/RGB+` = `&rgb_ug RGB_TOG / RGB_BRD / RGB_BRI`. `studio` = `&studio_unlock`.
- BT right (old muscle memory): `BT0`–`BT4` = `&bt BT_SEL 0..4`; `BTclr` = `&bt BT_CLR`.

---

## 5. Config (`config/piantor_pro_bt.conf`)

Lights **off by default** (the guide warns RGB drains battery to a few hours):
```
CONFIG_ZMK_RGB_UNDERGLOW=y
CONFIG_ZMK_RGB_UNDERGLOW_ON_START=n   # boots dark; RGBtog (kbd layer) turns on when wanted
```
Other config (sleep, etc.) inherited from the board defconfig; revisit if needed.

---

## 6. Build & flash

1. Push to GitHub → Actions builds two `.uf2` artifacts (left, right).
2. Per half: double-press the reset button (SIM tool) → board mounts as `KEEBART` → drag the
   matching `.uf2`.
3. Pair Bluetooth via the `kbd` layer `BT` keys; use a fresh profile per device.
4. Optional live tweaks: `kbd` → `studio` (unlock) → edit in the ZMK Studio app.

---

## 7. Deliverables

1. `config/west.yml` — Keebart module added.
2. `build.yaml` — left/right board targets with Studio.
3. `config/piantor_pro_bt.keymap` — five layers + behaviors + combo above.
4. `config/piantor_pro_bt.conf` — RGB off at boot.
5. **`docs/keymap.md`** — generated keymap documentation with a visual representation of every
   layer (the layer-map tables from §4, kept in sync with the keymap), plus a short legend and the
   activation table from §1.

---

## 8. Decisions & open items

**Decided:**
- 5 layers; thumb activation as in §1; `kbd` via `esc`+`del` combo.
- GACS home-row mods, replicated on `sym`/`brc`.
- Desktop switch on `q`–`t` hold.
- Empty keys hard-blocked (`&none`); thumbs `&trans` on layers 1–4.
- macOS modifiers; `⌘Z` = undo.
- Lights off by default.
- West-module board wiring (not vendored).

**To finalize during implementation:**
- Exact hold-tap timing numbers (tapping-term, prior-idle) — start from §3 values, tune after use.
- Pinned Keebart revision SHA.
- Whether `bootloader`/`reset` keymap keys are kept (physical button is sufficient) — kept for now.
