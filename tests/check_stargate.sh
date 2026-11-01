#!/bin/sh
# Compare Comskip EDL to Stargate ground truth. Skips if the TS is absent.
# Usage: tests/check_stargate.sh [--run]

set -e
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ts="$root/Test Video/Stargate Atlantis S01E01 Rising.ts"
edl="$root/Test Video/Stargate Atlantis S01E01 Rising.edl"
expected="$root/tests/stargate_rising.expected.edl"
ini="$root/ini/sky.ini"

if [ ! -f "$ts" ]; then
    echo "SKIP: $ts not present"
    exit 77
fi

if [ "${1-}" = "--run" ]; then
    "$root/comskip" --ini "$ini" "$ts"
fi

if [ ! -f "$edl" ]; then
    echo "FAIL: no EDL at $edl (run with --run)"
    exit 1
fi

python3 - "$edl" "$expected" <<'PY'
import sys

def load(path):
    rows = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            rows.append((float(parts[0]), float(parts[1])))
    return rows

edl, expected = load(sys.argv[1]), load(sys.argv[2])
if len(edl) != len(expected):
    print(f"FAIL: {len(edl)} cuts, expected {len(expected)}")
    print("got:", edl)
    sys.exit(1)

# Mid-show pods: 2.5s. Credits start: 3s. Credits end is EOF padding.
tol = [(2.5, 2.5), (2.5, 2.5), (3.0, 30.0)]
failed = 0
for i, ((gs, ge), (ws, we), (ts, te)) in enumerate(zip(edl, expected, tol), 1):
    if abs(gs - ws) > ts or abs(ge - we) > te:
        print(f"FAIL break {i}: got {gs:.2f}-{ge:.2f} want {ws:.2f}-{we:.2f}")
        failed += 1
    else:
        print(f"ok   break {i}: {gs:.2f}-{ge:.2f}")
sys.exit(1 if failed else 0)
PY
