#!/bin/sh
# SessionStart hook: nudge when the "Last upstream check" date in CLAUDE.md is
# older than 90 days. Prints nothing otherwise. Wired up in .claude/settings.json.
cd "$(dirname "$0")/.." || exit 0
last=$(grep -o 'Last upstream check: [0-9-]*' CLAUDE.md | grep -o '[0-9-]*$')
[ -n "$last" ] || exit 0
python3 - "$last" <<'EOF'
import datetime as d, sys
last = d.date.fromisoformat(sys.argv[1])
age = (d.date.today() - last).days
if age > 90:
    print(f"UPSTREAM CHECK DUE: last done {last} ({age} days ago). "
          "Run the 'Upstream sync check' procedure in CLAUDE.md and bump the date.")
EOF
