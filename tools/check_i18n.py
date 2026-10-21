"""Fail when a translated label is missing or loses its printf specifiers.

Every user-visible string goes through T("中文") (or L("中文", "English") for the
rare word that needs two English forms). The English text lives in
firmware/magepie_dash/i18n.cpp, keyed by the Chinese source. This checks the two
mistakes that are invisible at build time and obvious on the panel:

  * a T("...") literal with no entry -> that one label stays Chinese in English mode
  * an entry whose %-specifiers differ from the source -> snprintf() reads the
    wrong argument type (or prints a literal "%d") on a 480x272 screen

    python tools/check_i18n.py
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FW = os.path.join(ROOT, "firmware", "magepie_dash")
TABLE = os.path.join(FW, "i18n.cpp")

ENTRY = re.compile(r'\{\s*"((?:[^"\\]|\\.)*)"\s*,\s*"((?:[^"\\]|\\.)*)"\s*\}')
USED = re.compile(r'\b(?:T|L)\(\s*"((?:[^"\\]|\\.)*)"')
SPEC = re.compile(r"%(?:%|[-+ 0#]*\d*(?:\.\d+)?(?:hh|h|ll|l|z)?[diouxXeEfgGcs])")


def unescape(s):
    return s.replace('\\"', '"').replace("\\\\", "\\")


def main():
    src = open(TABLE, encoding="utf-8").read()
    table = src.split("static const Say kSay")[0]  # kSay is id -> two lines, not zh -> en
    entries = {unescape(zh): unescape(en) for zh, en in ENTRY.findall(table)}
    if not entries:
        sys.exit(f"could not read any entry from {TABLE}")

    used = {}
    for name in sorted(os.listdir(FW)):
        if not name.endswith((".cpp", ".h", ".ino")) or name.startswith("ark") or name == "i18n.cpp":
            continue
        src = open(os.path.join(FW, name), encoding="utf-8").read()
        for lit in USED.findall(src):
            used.setdefault(unescape(lit), set()).add(name)

    problems = 0
    for zh in sorted(used):
        where = ", ".join(sorted(used[zh]))
        if zh not in entries:
            print(f"no English for {zh!r} (used in {where})")
            problems += 1
            continue
        en = entries[zh]
        want, got = sorted(SPEC.findall(zh)), sorted(SPEC.findall(en))
        if want != got:
            print(f"format specifiers differ for {zh!r} -> {en!r}: {want} vs {got} ({where})")
            problems += 1

    unused = sorted(set(entries) - set(used))
    if unused:
        print(f"note: {len(unused)} table entries are not referenced by T()/L(): "
              + ", ".join(unused[:6]) + (" ..." if len(unused) > 6 else ""))

    if problems:
        print(f"{problems} problem(s): fix i18n.cpp or the call site.")
        sys.exit(1)
    print(f"ok: {len(used)} panel strings, all translated, specifiers match ({len(entries)} table entries)")


if __name__ == "__main__":
    main()
