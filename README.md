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

## Debugging misfires

Two data sources, best used together while typing a drill like `I find it. I fix it.`:

1. **Firmware decisions.** Flash `piantor_pro_bt_left_usb_logging.uf2` (built by CI) to the left half, plug it in over USB, then in a terminal:
   ```
   script -q ~/Downloads/zmk.log sudo cu -l /dev/cu.usbmodem*   # records the whole session to the file
   ```
   Type the drill, then quit cu with Enter, `~`, `.` (Ctrl-C is sent to the keyboard, not to cu; and piping cu's output through tee makes it exit at once). cu needs sudo for its lock directory in /var/spool/uucp. Then:
   ```
   grep decided ~/Downloads/zmk.log                          # just the hold-tap verdicts
   ```
   The raw stream is very chatty (every position event, BLE, battery). Every hold-tap logs `<position> decided hold/tap (<flavor> decision moment <timer|key-up|other-key-up>)` with a timestamp. Position 13 is F, 16 is J. Flash the normal image back afterwards; logging costs battery.
2. **Your timing.** Open `docs/hrm-timing.html` in a browser and type in the box. It logs every key's down/up time, hold length, gap from the previous key and overlap with it. Yellow rows are modifiers, meaning ZMK decided hold. Copy the log as TSV.

Feed both logs plus `config/piantor_pro_bt.keymap` to an assistant, or eyeball them: a wanted capital that came out as `fi` shows as `f` released before `i` in the timing page and `13 decided tap (balanced decision moment key-up)` in the firmware log.

