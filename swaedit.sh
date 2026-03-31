#!/bin/sh
# Launcher script for swaedit on Linux.
# Uses system OpenGL if available; falls back to bundled Mesa software renderer.

DIR="$(cd "$(dirname "$0")" && pwd)"

if ldconfig -p 2>/dev/null | grep -q 'libGL\.so\.1'; then
    exec "$DIR/swaedit" "$@"
else
    echo "No system OpenGL found — using software renderer"
    LD_LIBRARY_PATH="$DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
        exec "$DIR/swaedit" "$@"
fi
