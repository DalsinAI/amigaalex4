"""Print the lines of a C file an AmigaOS build compiles: #ifdef, #ifndef,
#if [!]defined(X) [&& ...], #elif defined(X), #else and #endif are taken
with __amigaos__ defined and __unix__, _WIN32 and the editor and shooter
switches not; any other #if is kept as if true. Enough for Alex 4's own
platform branches.
    python3 amiga_lines.py FILE
Copyright (c) 2026 AmiCradle Team. GPL-2.0-or-later, like Alex 4."""
import re, sys

DEFINED = {"__amigaos__"}

def cond(expr):
    expr = expr.strip()
    terms = re.split(r"\s*&&\s*", expr)
    val = True
    for t in terms:
        m = re.fullmatch(r"(!)?\s*defined\s*\(?\s*(\w+)\s*\)?", t)
        if not m:
            return True
        v = m.group(2) in DEFINED
        val = val and (not v if m.group(1) else v)
    return val

stack = []  # (taking, any_taken)
def live():
    return all(s[0] for s in stack)

for n, line in enumerate(open(sys.argv[1], encoding="latin-1"), 1):
    s = line.strip()
    m = re.match(r"#\s*(ifdef|ifndef|if|elif|else|endif)\b(.*)", s)
    if m:
        d, rest = m.group(1), m.group(2).split("//")[0]
        if d == "ifdef":
            v = cond("defined(%s)" % rest.strip()); stack.append([v, v])
        elif d == "ifndef":
            v = cond("!defined(%s)" % rest.strip()); stack.append([v, v])
        elif d == "if":
            v = cond(rest); stack.append([v, v])
        elif d == "elif":
            v = not stack[-1][1] and cond(rest); stack[-1] = [v, stack[-1][1] or v]
        elif d == "else":
            stack[-1] = [not stack[-1][1], True]
        else:
            stack.pop()
        continue
    if live():
        sys.stdout.write("%s:%d:%s" % (sys.argv[1], n, line))
