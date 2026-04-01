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

export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig:${PREFIX}/share/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig"

# Override the link rule so ALL libraries (cmake-managed + explicit) are
# wrapped in --start-group/--end-group.  This resolves circular deps
# (freetype↔harfbuzz) and ensures transitive static deps (fontconfig→expat,
# fontconfig→freetype BDF) are found regardless of link order.
LINK_RULE="<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -Wl,--start-group <LINK_LIBRARIES> -L${PREFIX}/lib -lfontconfig -lfreetype -lharfbuzz -lexpat -lpng16 -lz -lbz2 -lpcre2-8 -lpcre2-16 -Wl,--end-group"

cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${PREFIX}/qt6;${PREFIX}" \
    -DCMAKE_FIND_LIBRARY_SUFFIXES=".a;.so" \
    -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++" \
    -DCMAKE_CXX_LINK_EXECUTABLE="$LINK_RULE"

cmake --build "$BUILD_DIR" -j"$JOBS"

echo "Stripping binary..."
strip "$BUILD_DIR/swaedit"

echo ""
echo "Binary:  ${BUILD_DIR}/swaedit"
echo "Size:    $(du -h "${BUILD_DIR}/swaedit" | cut -f1)"
echo ""
echo "Dynamic dependencies:"
ldd "$BUILD_DIR/swaedit" 2>&1 || echo "(ldd not available)"
