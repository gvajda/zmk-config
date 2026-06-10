# Piantor Pro BT Keymap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a 5-layer ZMK keymap for the 36-key Keebart Piantor Pro BT that replicates the user's kanata config (home-row mods, numbers/symbols, brackets/nav) plus thumb-enabled function/media and keyboard-hardware layers.

**Architecture:** Consume Keebart's board definition as a west module. Write `config/piantor_pro_bt.keymap` (42 bindings/layer — outer columns `&none`, since the 36-key unit is the 42-position board with the outer column unpopulated) and `config/piantor_pro_bt.conf` (RGB off at boot). Custom hold-tap behaviors provide home-row mods and the desktop-switch hold; a combo provides the `kbd` layer. Verification is a green firmware build per task.

**Tech Stack:** ZMK firmware (Zephyr devicetree `.keymap`/`.conf`), west manifest, GitHub Actions build, `gh` CLI.

**Spec:** `docs/superpowers/specs/2026-06-10-piantor-pro-bt-keymap-design.md`

---

## Reference: build verification

There is no unit-test harness for a keymap; the gate for every task is **a successful firmware build**. Two ways to run it:

- **Local (fast, preferred if set up):** the ZMK CLI used for `init` — `zmk build` from the repo root. If the local toolchain isn't installed, use GitHub Actions.
- **GitHub Actions (always available):** `git push -u origin feat/piantor-keymap` then watch the run:
  ```bash
  gh run watch --exit-status   # or: gh run list --branch feat/piantor-keymap
  ```
  Green = pass. Download artifacts with `gh run download` when you want `.uf2` files.

When a step says "**Verify: build green**", run one of the above and confirm success before moving on.

## Reference: matrix position numbers (42-key grid)

Used by the home-row-mod positional config and to place `&none` outer keys. Bindings are written in this order, 12 per main row + 6 thumbs:

```
Row0:  0   1   2   3   4   5      6   7   8   9  10  11
Row1: 12  13  14  15  16  17     18  19  20  21  22  23
Row2: 24  25  26  27  28  29     30  31  32  33  34  35
Thumb:        36  37  38         39  40  41
```

- Outer columns (no switch on 36-key unit): **0, 11, 12, 23, 24, 35**.
- Left hand: `0 1 2 3 4 5 12 13 14 15 16 17 24 25 26 27 28 29` + left thumbs `36 37 38`.
- Right hand: `6 7 8 9 10 11 18 19 20 21 22 23 30 31 32 33 34 35` + right thumbs `39 40 41`.
- Thumbs: 36=L-outer(esc), 37=L-mid(space), 38=L-inner(tab), 39=R-inner(bspc), 40=R-mid(enter), 41=R-outer(del).

---

## File structure

| File | Responsibility |
|------|----------------|
| `config/west.yml` | Add Keebart remote + project so `piantor_pro_bt_*` boards resolve |
| `build.yaml` | Declare left/right board build targets (with ZMK Studio) |
| `config/piantor_pro_bt.conf` | RGB underglow off at boot |
| `config/piantor_pro_bt.keymap` | Behaviors, combo, and all 5 layers |
| `docs/keymap.md` | Generated visual documentation of every layer |

---

## Task 1: Wire the Keebart board as a west module

**Files:**
- Modify: `config/west.yml`

- [ ] **Step 1: Get the current Keebart commit SHA to pin**

Run:
```bash
gh api repos/Keebart/zmk-config/commits/main --jq .sha
```
Expected: a 40-char SHA (e.g. `a1b2c3...`). Copy it; it replaces `PIN_SHA` below.

- [ ] **Step 2: Replace `config/west.yml` with the module-enabled manifest**

Write `config/west.yml` (substitute the real SHA for `PIN_SHA`):
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
      revision: PIN_SHA
  self:
    path: config
```

- [ ] **Step 3: Refresh west so the new project is fetched**

Run (local toolchain): `zmk update` (or `west update`).
Expected: clones `zmk-config` from Keebart without error. If you have no local toolchain, skip — GitHub Actions runs `west update` itself.

- [ ] **Step 4: Commit**

```bash
git add config/west.yml
git commit -m "build: add Keebart board definitions as a west module"
```

---

## Task 2: Declare the build targets

**Files:**
- Modify: `build.yaml`

- [ ] **Step 1: Replace `build.yaml` with both board halves**

Write `build.yaml`:
```yaml
include:
  - board: piantor_pro_bt_left
    snippet: studio-rpc-usb-uart
    cmake-args: -DCONFIG_ZMK_STUDIO=y
  - board: piantor_pro_bt_right
    snippet: studio-rpc-usb-uart
    cmake-args: -DCONFIG_ZMK_STUDIO=y
```

- [ ] **Step 2: Verify the board resolves — build green**

At this point there is no `config/piantor_pro_bt.keymap` yet, so the build uses Keebart's default keymap. Run a build (see "Reference: build verification"). The point of this task is to confirm `piantor_pro_bt_left`/`_right` are found and the default firmware compiles.
Expected: build succeeds for both boards. A "board not found" error here means the west module in Task 1 is misconfigured — fix before continuing.

- [ ] **Step 3: Commit**

```bash
git add build.yaml
git commit -m "build: add piantor_pro_bt left/right targets with ZMK Studio"
```

---

## Task 3: RGB-off config

**Files:**
- Create: `config/piantor_pro_bt.conf`

- [ ] **Step 1: Create the config file**

Write `config/piantor_pro_bt.conf`:
```
# RGB underglow is available but OFF at boot — RGB drains the battery to a few
# hours (per the keyboard guide). Toggle it on from the kbd layer (RGBtog).
CONFIG_ZMK_RGB_UNDERGLOW=y
CONFIG_ZMK_RGB_UNDERGLOW_ON_START=n
```

- [ ] **Step 2: Verify build green**

Run a build. Expected: compiles. (Behaviour-wise the lights now stay off until toggled; confirmed on-device in Task 9.)

- [ ] **Step 3: Commit**

```bash
git add config/piantor_pro_bt.conf
git commit -m "config: keep RGB underglow off at boot"
```

---

## Task 4: Keymap scaffold — headers, behaviors, combo, BASE layer

This creates the keymap file with all behaviors and the base layer. Layers 1–4 are temporary all-`&trans` stubs so the file compiles; later tasks replace each stub.

**Files:**
- Create: `config/piantor_pro_bt.keymap`

- [ ] **Step 1: Create the keymap with behaviors, combo, and BASE + four stub layers**

Write `config/piantor_pro_bt.keymap`:
```c
#include <behaviors.dtsi>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/bt.h>
#include <dt-bindings/zmk/rgb.h>

/ {
    chosen {
        zmk,physical-layout = &default_layout;
    };

    behaviors {
        // Home-row mod, LEFT hand: hold = modifier, tap = letter.
        // Only triggers the hold if a RIGHT-hand or thumb key follows (positional),
        // which kills mis-holds during same-hand rolls. Replaces kanata's chord list.
        hml: home_row_mod_left {
            compatible = "zmk,behavior-hold-tap";
            #binding-cells = <2>;
            flavor = "balanced";
            tapping-term-ms = <200>;
            quick-tap-ms = <175>;
            require-prior-idle-ms = <150>;
            bindings = <&kp>, <&kp>;
            hold-trigger-key-positions = <6 7 8 9 10 11 18 19 20 21 22 23 30 31 32 33 34 35 36 37 38 39 40 41>;
            hold-trigger-on-release;
        };
        // Home-row mod, RIGHT hand: triggers only after a LEFT-hand or thumb key.
        hmr: home_row_mod_right {
            compatible = "zmk,behavior-hold-tap";
            #binding-cells = <2>;
            flavor = "balanced";
            tapping-term-ms = <200>;
            quick-tap-ms = <175>;
            require-prior-idle-ms = <150>;
            bindings = <&kp>, <&kp>;
            hold-trigger-key-positions = <0 1 2 3 4 5 12 13 14 15 16 17 24 25 26 27 28 29 36 37 38 39 40 41>;
            hold-trigger-on-release;
        };
        // Desktop switch on top row: tap = letter, hold = Ctrl+number.
        // Standalone hold (no second key needed), so no positional restriction;
        // tap-preferred favors the letter on quick taps.
        hd: hold_desktop {
            compatible = "zmk,behavior-hold-tap";
            #binding-cells = <2>;
            flavor = "tap-preferred";
            tapping-term-ms = <200>;
            require-prior-idle-ms = <150>;
            bindings = <&kp>, <&kp>;
        };
    };

    combos {
        compatible = "zmk,combos";
        // Squeeze both outer thumbs (esc + del) -> kbd layer (4).
        combo_kbd {
            timeout-ms = <40>;
            require-prior-idle-ms = <150>;
            key-positions = <36 41>;
            bindings = <&mo 4>;
            layers = <0>;
        };
    };

    keymap {
        compatible = "zmk,keymap";

        base_layer {
            display-name = "BASE";
            bindings = <
                &none  &hd LC(N1) Q  &hd LC(N2) W  &hd LC(N3) E  &hd LC(N4) R  &hd LC(N5) T      &kp Y  &kp U          &kp I          &kp O         &kp P           &none
                &none  &hml LCTRL A  &hml LALT S   &hml LGUI D   &hml LSHFT F  &kp G             &kp H  &hmr RSHFT J   &hmr RGUI K    &hmr RALT L   &hmr RCTRL SEMI &none
                &none  &kp Z         &kp X         &kp C         &kp V         &kp B             &kp N  &kp M          &kp COMMA      &kp DOT       &kp FSLH        &none
                                                   &lt 3 ESC     &lt 1 SPACE   &lt 2 TAB         &lt 2 BSPC &lt 1 RET  &lt 3 DEL
            >;
        };

        sym_layer {
            display-name = "SYM";
            bindings = <
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                                     &trans &trans &trans   &trans &trans &trans
            >;
        };

        brc_layer {
            display-name = "BRC";
            bindings = <
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                                     &trans &trans &trans   &trans &trans &trans
            >;
        };

        fn_layer {
            display-name = "FN";
            bindings = <
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                                     &trans &trans &trans   &trans &trans &trans
            >;
        };

        kbd_layer {
            display-name = "KBD";
            bindings = <
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                &trans &trans &trans &trans &trans &trans   &trans &trans &trans &trans &trans &trans
                                     &trans &trans &trans   &trans &trans &trans
            >;
        };
    };
};
```

- [ ] **Step 2: Verify build green**

Run a build. Expected: both boards compile. Common failures: a label typo, wrong binding count in a row (must be exactly 12 / 12 / 12 / 6), or an unknown keycode — read the compiler error, it names the line.

- [ ] **Step 3: Commit**

```bash
git add config/piantor_pro_bt.keymap
git commit -m "feat: keymap scaffold with behaviors, combo, and base layer"
```

---

## Task 5: SYM layer

**Files:**
- Modify: `config/piantor_pro_bt.keymap` (replace the `sym_layer` bindings)

- [ ] **Step 1: Replace the `sym_layer` bindings block**

Find the `sym_layer { ... bindings = < ... >; }` and replace its `bindings` with:
```c
                &none  &kp N1            &kp N2          &kp N3           &kp N4           &kp N5       &kp N6      &kp N7            &kp N8           &kp N9          &kp N0           &none
                &none  &hml LCTRL EXCL   &hml LALT AT    &hml LGUI HASH   &hml LSHFT DLLR  &kp PRCNT    &kp CARET   &hmr RSHFT AMPS  &hmr RGUI ASTRK  &hmr RALT MINUS &hmr RCTRL PLUS  &none
                &none  &kp GRAVE         &kp TILDE       &none            &none            &none        &none       &none            &none            &kp UNDER       &kp EQUAL        &none
                                                         &trans           &trans           &trans       &trans      &trans           &trans
```
This is: top row `1 2 3 4 5 6 7 8 9 0`; home row `! @ # $ % ^ & * - +` with HRM holds replicated; bottom row `` ` `` `~` then `_ =` on the right two used keys; everything else `&none`; thumbs `&trans`.

- [ ] **Step 2: Verify build green**

Run a build. Expected: compiles.

- [ ] **Step 3: Commit**

```bash
git add config/piantor_pro_bt.keymap
git commit -m "feat: sym layer (numbers + symbols with replicated home-row mods)"
```

---

## Task 6: BRC layer

**Files:**
- Modify: `config/piantor_pro_bt.keymap` (replace the `brc_layer` bindings)

- [ ] **Step 1: Replace the `brc_layer` bindings block**

Replace the `brc_layer` `bindings` with:
```c
                &none  &none              &kp LA(LEFT)   &kp UP          &kp LA(RIGHT)   &none           &none   &kp LBKT         &kp RBKT       &none          &none           &none
                &none  &hml LCTRL LG(LEFT) &hml LALT LEFT &hml LGUI DOWN  &hml LSHFT RIGHT &kp LG(RIGHT)  &none   &hmr RSHFT LPAR  &hmr RGUI RPAR &hmr RALT SQT  &hmr RCTRL DQT  &none
                &none  &none              &none          &none           &none           &kp LC(B)       &none   &kp LBRC         &kp RBRC       &kp BSLH       &kp PIPE        &none
                                                         &trans          &trans          &trans          &trans  &trans           &trans
```
This is: word-left/up/word-right and `[ ]` on top; line-start/←/↓/→/line-end (HRM holds) and `( ) ' "` (HRM holds) on home; `⌃b` and `{ } \ |` on bottom; removed keys (esc/tab/bspc/del/`` ` ``/~) are `&none`; thumbs `&trans`.

- [ ] **Step 2: Verify build green**

Run a build. Expected: compiles. Note: `LG(LEFT)` etc. are modified keycodes used as the *tap* side of a hold-tap — valid.

- [ ] **Step 3: Commit**

```bash
git add config/piantor_pro_bt.keymap
git commit -m "feat: brc layer (brackets + nav with replicated home-row mods)"
```

---

## Task 7: FN layer

**Files:**
- Modify: `config/piantor_pro_bt.keymap` (replace the `fn_layer` bindings)

- [ ] **Step 1: Replace the `fn_layer` bindings block**

Replace the `fn_layer` `bindings` with:
```c
                &none  &kp F1     &kp F2     &kp F3     &kp F4     &kp F5      &kp F6        &kp F7        &kp F8       &kp F9   &kp F10  &none
                &none  &none      &none      &none      &none      &none       &kp C_VOL_DN  &kp C_VOL_UP  &kp C_MUTE   &kp F11  &kp F12  &none
                &none  &kp LG(Z)  &kp LG(X)  &kp LG(C)  &kp LG(V)  &none       &kp C_PREV    &kp C_PP      &kp C_NEXT   &none    &none    &none
                                             &trans     &trans     &trans      &trans        &trans        &trans
```
This is: `F1–F10` full width on top; left home empty; right home `vol- vol+ mute F11 F12`; `⌘Z ⌘X ⌘C ⌘V` on left bottom; `prev play/pause next` on right bottom; thumbs `&trans`.

- [ ] **Step 2: Verify build green**

Run a build. Expected: compiles.

- [ ] **Step 3: Commit**

```bash
git add config/piantor_pro_bt.keymap
git commit -m "feat: fn layer (F-keys, cmd clipboard, media)"
```

---

## Task 8: KBD layer

**Files:**
- Modify: `config/piantor_pro_bt.keymap` (replace the `kbd_layer` bindings)

- [ ] **Step 1: Replace the `kbd_layer` bindings block**

Replace the `kbd_layer` `bindings` with:
```c
                &none  &bootloader      &sys_reset        &none             &none            &none   &none         &none         &none         &none         &none         &none
                &none  &rgb_ug RGB_TOG  &rgb_ug RGB_BRD   &rgb_ug RGB_BRI   &studio_unlock   &none   &bt BT_SEL 0  &bt BT_SEL 1  &bt BT_SEL 2  &bt BT_SEL 3  &bt BT_SEL 4  &none
                &none  &none            &none             &none             &none            &none   &bt BT_CLR    &none         &none         &none         &none         &none
                                                          &trans            &trans           &trans  &trans        &trans        &trans
```
This is: `bootloader reset` top-left; `RGB-toggle / brightness-down / brightness-up / studio-unlock` on left home; `BT0–BT4` on right home; `BT_CLR` on right bottom; thumbs `&trans`.

- [ ] **Step 2: Verify build green**

Run a build. Expected: compiles. The keymap is now complete.

- [ ] **Step 3: Commit**

```bash
git add config/piantor_pro_bt.keymap
git commit -m "feat: kbd layer (bluetooth, rgb, studio, bootloader/reset)"
```

---

## Task 9: Visual keymap documentation

**Files:**
- Create: `docs/keymap.md`

- [ ] **Step 1: Create the documentation file**

Write `docs/keymap.md`:
```markdown
# Piantor Pro BT — Keymap

36-key Piantor Pro BT. Five layers, all reached from the thumb cluster.
`/M` after a key = hold for modifier M (home-row mods). `·` = no key (blocked).

## Layer activation

| Layer | Hold |
|-------|------|
| SYM   | space or enter (middle thumbs) |
| BRC   | tab or bspc (inner thumbs) |
| FN    | esc or del (outer thumbs) |
| KBD   | esc **and** del together |

Home-row mods (pinky→index): Ctrl, Alt, Cmd, Shift. Top row holds = Ctrl+1..5 (macOS desktop switch).

## BASE
```
 q/⌃1  w/⌃2  e/⌃3  r/⌃4  t/⌃5  │  y     u     i     o     p
 a/⌃   s/⌥   d/⌘   f/⇧   g     │  h     j/⇧   k/⌘   l/⌥   ;/⌃
 z     x     c     v     b     │  n     m     ,     .     /
             esc   spc   tab   │  bspc  ent   del
```

## SYM (hold space / enter)
```
 1     2     3     4     5     │  6     7     8     9     0
 !/⌃   @/⌥   #/⌘   $/⇧   %     │  ^     &/⇧   */⌘   -/⌥   +/⌃
 `     ~     ·     ·     ·     │  ·     ·     ·     _     =
```

## BRC (hold tab / bspc)
```
 ·     ⌥←    ↑     ⌥→    ·     │  ·     [     ]     ·     ·
 ⌘←/⌃  ←/⌥   ↓/⌘   →/⇧   ⌘→    │  ·     (/⇧   )/⌘   '/⌥   "/⌃
 ·     ·     ·     ·     ⌃b    │  ·     {     }     \     |
```

## FN (hold esc / del)
```
 F1    F2    F3    F4    F5    │  F6    F7    F8    F9    F10
 ·     ·     ·     ·     ·     │  vol-  vol+  🔇    F11   F12
 ⌘Z    ⌘X    ⌘C    ⌘V    ·     │  ◀◀    ▶❚❚   ▶▶    ·     ·
```

## KBD (squeeze esc + del)
```
 boot  reset ·     ·     ·     │  ·     ·     ·     ·     ·
 RGB   RGB-  RGB+  studio ·    │  BT0   BT1   BT2   BT3   BT4
 ·     ·     ·     ·     ·     │  BTclr ·     ·     ·     ·
```

> Firmware note: this is the 42-position board with the outer column unpopulated; the keymap pads
> each row with `&none` outer keys (matrix positions 0, 11, 12, 23, 24, 35).
```

- [ ] **Step 2: Verify the doc matches the keymap**

Read `config/piantor_pro_bt.keymap` and confirm every non-`&none`/`&trans` binding appears in the matching layer table above (and vice versa). Fix any drift.

- [ ] **Step 3: Commit**

```bash
git add docs/keymap.md
git commit -m "docs: add visual keymap reference"
```

---

## Task 10: Flash and verify on device

**Files:** none (manual hardware verification)

- [ ] **Step 1: Get the firmware artifacts**

Push and download the built `.uf2` files:
```bash
git push -u origin feat/piantor-keymap
gh run watch --exit-status
gh run download   # pulls piantor_pro_bt_left and piantor_pro_bt_right .uf2 artifacts
```

- [ ] **Step 2: Flash each half**

For each half: double-press the reset button (SIM-card tool) on the bottom → board mounts as drive `KEEBART` → drag the matching `.uf2` (`...left.uf2` to the left half, `...right.uf2` to the right).

- [ ] **Step 3: Verify behavior on device**

Confirm each, fixing the keymap and re-flashing if any fails:
- Lights are **off** at boot.
- Letters type normally; home-row holds give Ctrl/Alt/Cmd/Shift (e.g. hold `d` + tap `c` = ⌘C... actually that is Cmd; test hold `f`=Shift + a letter = capital).
- Top row hold `q`–`t` switches macOS desktops (requires Ctrl+number shortcuts enabled in System Settings → Keyboard → Keyboard Shortcuts → Mission Control).
- Hold space/enter → numbers + symbols. Hold tab/bspc → brackets + arrows.
- Hold esc or del → F-keys/media; ⌘C/⌘X/⌘V work (hold **del**, opposite hand, tap z/x/c/v).
- Squeeze esc+del → kbd layer: BT0–4 pair (use a fresh profile per device), RGBtog turns lights on/off, BT_CLR clears a profile.

- [ ] **Step 4: Finish the branch**

Use the superpowers:finishing-a-development-branch skill to decide merge/PR/cleanup.

---

## Self-review notes

- **Spec coverage:** board wiring (T1–2), RGB-off (T3), behaviors+base (T4), sym (T5), brc (T6), fn (T7), kbd (T8), docs (T9), flash/verify (T10) — every spec section maps to a task.
- **Binding counts:** every layer block is 12+12+12+6 = 42. Outer columns (`&none`) and thumb `&trans` are consistent across all five layers.
- **Behavior names** (`hml`, `hmr`, `hd`) and layer indices (`&lt 1`=sym, `&lt 2`=brc, `&lt 3`=fn, `&mo 4`=kbd) are consistent between the base layer, the combo, and all references.
