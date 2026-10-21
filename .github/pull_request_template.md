## What changes

<!-- one or two sentences: what the screen / tool does differently -->

## How it was checked

- [ ] CI is green (firmware compiles, tool tests, glyph check)
- [ ] Flashed to a JC4827W543 (`bash flash.sh flash`), if firmware changed
- [ ] On-device suites, if UI changed: `A` taps, `Z` settings, `B` agent sheet (all `failed=0`)
- [ ] Screenshots attached for visible changes (`py -3.11 tools/board_shot.py --demo --all`)
- [ ] New on-screen text added to `tools/gen_fonts.py` and fonts regenerated, if any
