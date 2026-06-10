# Piantor Pro BT — Keymap

36-key Piantor Pro BT. Five layers, all reached from the thumb cluster.
`/M` after a key = hold for modifier M (home-row mods). `·` = no key (blocked).

## Layer activation

| Layer | Hold |
|-------|------|
| SYM   | tab or bspc (inner thumbs) |
| BRC   | space or enter (middle thumbs) |
| FN    | esc or del (outer thumbs) |
| KBD   | esc **and** del together |

Home-row mods (pinky→index): Ctrl, Alt, Cmd, Shift. Top-row holds = Ctrl+1..5 (macOS desktop switch).

## BASE
```
 q/⌃1  w/⌃2  e/⌃3  r/⌃4  t/⌃5  │  y     u     i     o     p
 a/⌃   s/⌥   d/⌘   f/⇧   g     │  h     j/⇧   k/⌘   l/⌥   ;/⌃
 z     x     c     v     b     │  n     m     ,     .     /
             esc   spc   tab   │  bspc  ent   del
```

## SYM (hold tab / bspc)
```
 1     2     3     4     5     │  6     7     8     9     0
 !/⌃   @/⌥   #/⌘   $/⇧   %     │  ^     &/⇧   */⌘   -/⌥   +/⌃
 `     ~     ·     ·     ·     │  ·     ·     ·     _     =
```

## BRC (hold space / enter)
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
⌘Z/X/C/V most comfortable held with **del** (right thumb, opposite hand).

## KBD (squeeze esc + del)
```
 boot  reset ·     ·     ·     │  ·     ·     ·     ·     ·
 RGB   RGB-  RGB+  studio ·    │  BT0   BT1   BT2   BT3   BT4
 ·     ·     ·     ·     ·     │  BTclr ·     ·     ·     ·
```
RGB = toggle lights, RGB-/RGB+ = brightness, studio = ZMK Studio unlock.

> Firmware note: this is the 42-position board with the outer column unpopulated; the keymap pads
> each row with `&none` outer keys (matrix positions 0, 11, 12, 23, 24, 35).
