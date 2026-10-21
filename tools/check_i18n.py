"""Fail when a translated label is missing or loses its printf specifiers.

Every user-visible string goes through T("中文") (or L("中文", "English") for the
rare word that needs two English forms). The English text lives in
firmware/magepie_dash/i18n.cpp, keyed by the Chinese source. This checks the
mistakes that are invisible at build time, and obvious — or fatal — on the panel:

  * a T("...") literal with no entry -> that one label stays Chinese in English mode
  * an entry whose %-specifiers differ from the source in TYPE OR ORDER ->
    snprintf() reads the wrong argument (a "%s" fed an int can crash the board)
  * the same Chinese key listed twice with different English -> T() returns the
    first match, which is not necessarily the line a translator edited
  * an L(zh, en) pair whose two sides disagree
  * a call this tool cannot parse, or not a single call found -> the check would
    pass while checking nothing

    python tools/check_i18n.py
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FW = os.path.join(ROOT, "firmware", "magepie_dash")
TABLE = os.path.join(FW, "i18n.cpp")

ENTRY = re.compile(r'\{\s*"((?:[^"\\]|\\.)*)"\s*,\s*"((?:[^"\\]|\\.)*)"\s*\}')
CALL_START = re.compile(r'\b(T|L)\s*\(')
T_ARGS = re.compile(r'\s*"((?:[^"\\]|\\.)*)"\s*\)')
L_ARGS = re.compile(r'\s*"((?:[^"\\]|\\.)*)"\s*,\s*"((?:[^"\\]|\\.)*)"\s*\)')
SPEC = re.compile(r"%(?:%|[-+ 0#]*\d*(?:\.\d+)?(?:hh|h|ll|l|z)?[diouxXeEfgGcs])")


def unescape(s):
    return s.replace('\\"', '"').replace("\\\\", "\\")


def strip_comments(src):
    """Drop // and /* */ so doc comments that mention T() are not parsed as calls."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            i = n if j < 0 else j + 2
            out.append(" ")
        elif c == '"':
            j, buf = i + 1, ['"']
            while j < n and src[j] != '"':
                step = 2 if src[j] == "\\" else 1
                buf.append(src[j:j + step])
                j += step
            buf.append('"')
            out.append("".join(buf))
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def main():
    src = open(TABLE, encoding="utf-8").read()
    table = src.split("static const Say kSay")[0]  # kSay is id -> two lines, not zh -> en
    entries, dupes = {}, []
    for zh, en in ENTRY.findall(table):
        zh, en = unescape(zh), unescape(en)
        if zh in entries and entries[zh] != en:
            dupes.append((zh, entries[zh], en))
        entries.setdefault(zh, en)
    if not entries:
        sys.exit(f"could not read any entry from {TABLE}")

    t_used, l_used, unparsed = {}, {}, []
    for name in sorted(os.listdir(FW)):
        if not name.endswith((".cpp", ".h", ".ino")) or name.startswith("ark") or name == "i18n.cpp":
            continue
        text = strip_comments(open(os.path.join(FW, name), encoding="utf-8").read())
        for m in CALL_START.finditer(text):
            rest = text[m.end():]
            if rest.lstrip().startswith("const char"):  # declaration in a header
                continue
            if m.group(1) == "T":
                args = T_ARGS.match(rest)
                if not args:
                    unparsed.append(f"{name}: T({rest[:48]!r}")
                    continue
                t_used.setdefault(unescape(args.group(1)), set()).add(name)
            else:
                args = L_ARGS.match(rest)
                if not args:
                    unparsed.append(f"{name}: L({rest[:48]!r}")
                    continue
                pair = (unescape(args.group(1)), unescape(args.group(2)))
                l_used.setdefault(pair, set()).add(name)

    problems = 0

    def bad(msg):
        nonlocal problems
        print(msg)
        problems += 1

    for zh, first, second in dupes:
        bad(f"duplicate key {zh!r}: {first!r} vs {second!r} (T() returns the first)")

    for zh in sorted(t_used):
        where = ", ".join(sorted(t_used[zh]))
        if zh not in entries:
            bad(f"no English for {zh!r} (used in {where})")
            continue
        en = entries[zh]
        want, got = SPEC.findall(zh), SPEC.findall(en)
        if want != got:
            bad(f"format specifiers differ for {zh!r} -> {en!r}: {want} vs {got} ({where})")

    for (zh, en), where in sorted(l_used.items()):
        want, got = SPEC.findall(zh), SPEC.findall(en)
        if want != got:
            bad(f"L() specifiers differ for {zh!r} / {en!r}: {want} vs {got} "
                f"({', '.join(sorted(where))})")

    for line in unparsed:
        bad(f"cannot parse call: {line}")

    if not t_used:
        bad("no T() calls found - the scan is broken, not the firmware")

    unused = sorted(set(entries) - set(t_used))
    if unused:
        print(f"note: {len(unused)} table entries are not referenced by T(): "
              + ", ".join(unused[:6]) + (" ..." if len(unused) > 6 else ""))

    if problems:
        print(f"{problems} problem(s): fix i18n.cpp or the call site.")
        sys.exit(1)
    print(f"ok: {len(t_used)} panel strings translated, {len(l_used)} L() pair(s) checked, "
          f"specifiers match in order ({len(entries)} table entries)")


if __name__ == "__main__":
    main()
