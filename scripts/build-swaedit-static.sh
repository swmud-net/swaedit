#!/bin/bash
#
# Build swaedit against the static Qt prefix.
# Produces a maximally-static binary (only glibc + libGL are dynamic).
#
set -euo pipefail

PREFIX="${STATIC_DEPS_PREFIX:-/opt/static-deps}"
BUILD_DIR="${1:-build-static}"
JOBS="$(nproc)"

echo "Building swaedit (static) against ${PREFIX}/qt6..."

cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${PREFIX}/qt6" \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"

cmake --build "$BUILD_DIR" -j"$JOBS"

echo "Stripping binary..."
strip "$BUILD_DIR/swaedit"

echo ""
echo "Binary:  ${BUILD_DIR}/swaedit"
echo "Size:    $(du -h "${BUILD_DIR}/swaedit" | cut -f1)"
echo ""
echo "Dynamic dependencies:"
ldd "$BUILD_DIR/swaedit" 2>&1 || echo "(ldd not available)"
