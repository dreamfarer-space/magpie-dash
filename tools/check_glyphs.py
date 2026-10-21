"""Fail when a firmware string uses a glyph the bundled font does not have.

The fonts are subsets (tools/gen_fonts.py), so a character missing from ark12
renders as nothing on the panel, with no warning at build time. This checks
every string literal in the sketch (comments excluded) against the glyphs
actually present in firmware/magepie_dash/ark12.c. Runs in CI; no font files
or Node needed.

    python tools/check_glyphs.py
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FW = os.path.join(ROOT, "firmware", "magepie_dash")
FONT = os.path.join(FW, "ark12.c")
# Text a phone browser renders, not the panel (the WiFi setup page).
WEB_ONLY = {"netcfg.cpp"}


def literals(src):
    """String literals of a C/C++ source, skipping // and /* */ comments."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if src.startswith("//", i):
            i = src.find("\n", i)
            i = n if i < 0 else i
        elif src.startswith("/*", i):
            i = src.find("*/", i + 2)
            i = n if i < 0 else i + 2
        elif c == "'":  # char literal: '"' must not open a string
            j = i + 1
            while j < n and src[j] != "'":
                j += 2 if src[j] == "\\" else 1
            i = j + 1
        elif c == '"':
            j, buf = i + 1, []
            while j < n and src[j] != '"':
                if src[j] == "\\":
                    buf.append(src[j:j + 2])
                    j += 2
                else:
                    buf.append(src[j])
                    j += 1
            out.append("".join(buf))
            i = j + 1
        else:
            i += 1
    return out


def font_glyphs(src):
    lists = {m.group(1): [int(x, 16) for x in re.findall(r"0x([0-9a-fA-F]+)", m.group(2))]
             for m in re.finditer(r"unicode_list_(\d+)\[\]\s*=\s*\{(.*?)\};", src, re.S)}
    got = set()
    for m in re.finditer(r"\.range_start = (\d+), \.range_length = (\d+),.*?"
                         r"\.unicode_list = (NULL|unicode_list_(\d+))", src, re.S):
        start, length = int(m.group(1)), int(m.group(2))
        got |= {start + o for o in lists[m.group(4)]} if m.group(4) else set(range(start, start + length))
    return got


def main():
    glyphs = font_glyphs(open(FONT, encoding="utf-8").read())
    if not glyphs:
        sys.exit(f"could not read any glyphs from {FONT}")
    missing = {}
    for name in sorted(os.listdir(FW)):
        if not name.endswith((".cpp", ".h", ".ino")) or name.startswith("ark") or name in WEB_ONLY:
            continue
        src = open(os.path.join(FW, name), encoding="utf-8").read()
        src = "\n".join(l for l in src.splitlines() if "_map[] = {" not in l)  # sprite data
        for s in literals(src):
            for ch in s:
                if ord(ch) > 0x7E and ord(ch) not in glyphs:
                    missing.setdefault(ch, set()).add(f"{name}: {s!r}")
    if missing:
        for ch, where in sorted(missing.items()):
            print(f"missing {ch!r} (U+{ord(ch):04X}) used in:")
            for w in sorted(where)[:5]:
                print("   ", w)
        print("Add the text to tools/gen_fonts.py and regenerate, or reword it "
              "if Ark Pixel has no such glyph.")
        sys.exit(1)
    print(f"ok: every firmware string renders with ark12 ({len(glyphs)} glyphs)")


if __name__ == "__main__":
    main()
