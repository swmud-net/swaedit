#!/bin/bash
#
# Build ALL dependencies as static libraries, then build Qt (static)
# and Mesa (softpipe fallback shared lib).
#
# Key libraries are built from source to avoid transitive dep bloat:
#   - libxml2 without ICU/LZMA  (eliminates libicu, liblzma)
#   - freetype without brotli   (eliminates libbrotli)
#   - harfbuzz without glib/ICU/graphite2  (eliminates libglib, libgraphite2)
#   - fontconfig without glib   (eliminates libglib)
#
# System -dev packages provide .a files for X11/xcb stack.
# CMAKE_FIND_LIBRARY_SUFFIXES=".a;.so" ensures static linking throughout.
#
set -euo pipefail

PREFIX="${STATIC_DEPS_PREFIX:-/opt/static-deps}"
SRC="/tmp/static-deps-build"
JOBS="$(nproc)"

# --- Versions ---
ZLIB_VER=1.3.2
BZIP2_VER=1.0.8
EXPAT_VER=2.6.2
PCRE2_VER=10.43
LIBPNG_VER=1.6.43
LIBXML2_VER=2.12.7
FREETYPE_VER=2.13.2
HARFBUZZ_VER=8.3.0
FONTCONFIG_VER=2.14.2
QT_VER="${QT_VERSION:-6.8.1}"
QT_MAJOR_MINOR="$(echo "$QT_VER" | cut -d. -f1-2)"
MESA_VER="${MESA_VERSION:-23.3.6}"

mkdir -p "$PREFIX"/{lib/pkgconfig,include,share/pkgconfig} "$SRC"

export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PREFIX/share/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig"
export CFLAGS="-fPIC -O2"
export CXXFLAGS="-fPIC -O2"

# --- Helpers ---
fetch() {
    local url="$1" file dir
    file="$(basename "$url")"
    if [ ! -f "$SRC/$file" ]; then
        echo "Downloading $file ..." >&2
        wget --tries=3 --timeout=30 -nv -P "$SRC" "$url"
    fi
    dir="$(tar tf "$SRC/$file" | head -1 | cut -d/ -f1)"
    if [ ! -d "$SRC/$dir" ]; then
        tar xf "$SRC/$file" -C "$SRC"
    fi
    echo "$SRC/$dir"
}

step() { printf '\n========================================\n %s\n========================================\n' "$1"; }

# ===================================================================
# Tier 1 — leaf libraries
# ===================================================================

step "zlib ${ZLIB_VER}"
d=$(fetch "https://github.com/madler/zlib/releases/download/v${ZLIB_VER}/zlib-${ZLIB_VER}.tar.gz")
cd "$d" && rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DZLIB_BUILD_EXAMPLES=OFF
cmake --build build -j"$JOBS" && cmake --install build

step "bzip2 ${BZIP2_VER}"
d=$(fetch "https://sourceware.org/pub/bzip2/bzip2-${BZIP2_VER}.tar.gz")
cd "$d" && make clean 2>/dev/null || true
make -j"$JOBS" CC=gcc "CFLAGS=-fPIC -O2" libbz2.a
install -Dm644 libbz2.a "$PREFIX/lib/libbz2.a"
install -Dm644 bzlib.h "$PREFIX/include/bzlib.h"

step "expat ${EXPAT_VER}"
d=$(fetch "https://github.com/libexpat/libexpat/releases/download/R_2_6_2/expat-${EXPAT_VER}.tar.xz")
cd "$d" && rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_TOOLS=OFF
cmake --build build -j"$JOBS" && cmake --install build

step "pcre2 ${PCRE2_VER}"
d=$(fetch "https://github.com/PCRE2Project/pcre2/releases/download/pcre2-${PCRE2_VER}/pcre2-${PCRE2_VER}.tar.bz2")
cd "$d" && rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DPCRE2_BUILD_TESTS=OFF -DPCRE2_BUILD_PCRE2GREP=OFF
cmake --build build -j"$JOBS" && cmake --install build

# ===================================================================
# Tier 2 — depends on Tier 1
# ===================================================================

step "libpng ${LIBPNG_VER}"
d=$(fetch "https://download.sourceforge.net/libpng/libpng-${LIBPNG_VER}.tar.xz")
cd "$d" && rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF
cmake --build build -j"$JOBS" && cmake --install build

step "libxml2 ${LIBXML2_VER} (no ICU, no LZMA)"
d=$(fetch "https://download.gnome.org/sources/libxml2/2.12/libxml2-${LIBXML2_VER}.tar.xz")
cd "$d" && rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_LZMA=OFF \
    -DLIBXML2_WITH_ICU=OFF -DLIBXML2_WITH_ZLIB=ON \
    -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_TESTS=OFF \
    -DLIBXML2_WITH_PROGRAMS=OFF
cmake --build build -j"$JOBS" && cmake --install build

# ===================================================================
# Tier 3 — font stack (freetype ↔ harfbuzz circular dep)
# ===================================================================

step "freetype ${FREETYPE_VER} (pass 1: no harfbuzz, no brotli)"
d=$(fetch "https://download.savannah.gnu.org/releases/freetype/freetype-${FREETYPE_VER}.tar.xz")
cd "$d" && rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON
cmake --build build -j"$JOBS" && cmake --install build

step "harfbuzz ${HARFBUZZ_VER} (no glib, no ICU, no graphite2)"
d=$(fetch "https://github.com/harfbuzz/harfbuzz/releases/download/${HARFBUZZ_VER}/harfbuzz-${HARFBUZZ_VER}.tar.xz")
cd "$d" && rm -rf builddir
meson setup builddir --prefix="$PREFIX" --default-library=static \
    -Dfreetype=enabled -Dglib=disabled -Dgobject=disabled \
    -Dcairo=disabled -Dicu=disabled -Dgraphite2=disabled \
    -Dtests=disabled -Ddocs=disabled -Dbenchmark=disabled
ninja -C builddir -j"$JOBS" && ninja -C builddir install

step "freetype ${FREETYPE_VER} (pass 2: with harfbuzz)"
cd "$d/../freetype-${FREETYPE_VER}" && rm -rf build
cmake -B build -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DFT_DISABLE_HARFBUZZ=OFF -DFT_DISABLE_BROTLI=ON
cmake --build build -j"$JOBS" && cmake --install build

step "fontconfig ${FONTCONFIG_VER} (no glib)"
d=$(fetch "https://www.freedesktop.org/software/fontconfig/release/fontconfig-${FONTCONFIG_VER}.tar.xz")
cd "$d"
./configure --prefix="$PREFIX" --enable-static --disable-shared \
    --disable-docs --sysconfdir=/etc \
    PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
make -j"$JOBS" && make install

# ===================================================================
# Qt 6.8.x — static build
# ===================================================================

step "Qt ${QT_VER} (static)"

cd "$SRC"
QT_SRC="qtbase-everywhere-src-${QT_VER}"
QT_TAR="${QT_SRC}.tar.xz"
QT_URL="https://download.qt.io/official_releases/qt/${QT_MAJOR_MINOR}/${QT_VER}/submodules/${QT_TAR}"
[ -f "$QT_TAR" ] || wget -q --show-progress "$QT_URL" -O "$QT_TAR"
[ -d "$QT_SRC" ] || tar xf "$QT_TAR"
cd "$QT_SRC"
rm -rf build

echo "Configuring Qt..."
./configure \
    -prefix "$PREFIX/qt6" \
    -release \
    -static \
    -opensource -confirm-license \
    -no-icu \
    -fontconfig \
    -system-zlib \
    -system-libpng \
    -system-freetype \
    -system-harfbuzz \
    -system-pcre \
    -xcb \
    -bundled-xcb-xinput \
    -opengl desktop \
    -nomake examples \
    -nomake tests \
    -- \
    -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DCMAKE_FIND_LIBRARY_SUFFIXES=".a;.so" \
    -DCMAKE_C_FLAGS="-fPIC" \
    -DCMAKE_CXX_FLAGS="-fPIC" \
    -DFEATURE_eglfs=OFF \
    -DFEATURE_cups=OFF \
    -DFEATURE_dbus=OFF \
    -DFEATURE_zstd=OFF

echo "Building Qt (this takes 20-40 minutes)..."
cmake --build . -j"$JOBS"

echo "Installing Qt to ${PREFIX}/qt6..."
cmake --install .
echo "Qt build complete."

# ===================================================================
# Mesa — softpipe software renderer (bundled libGL.so fallback)
# ===================================================================

step "Mesa ${MESA_VER} (softpipe, xlib GLX)"

cd "$SRC"
MESA_SRC="mesa-${MESA_VER}"
MESA_TAR="${MESA_SRC}.tar.xz"
MESA_URL="https://archive.mesa3d.org/${MESA_TAR}"
[ -f "$MESA_TAR" ] || wget -q --show-progress "$MESA_URL" -O "$MESA_TAR"
[ -d "$MESA_SRC" ] || tar xf "$MESA_TAR"
cd "$MESA_SRC"
rm -rf builddir

meson setup builddir \
    --prefix="$PREFIX/mesa" \
    -Dgallium-drivers=swrast \
    -Dvulkan-drivers= \
    -Dglx=xlib \
    -Degl=disabled \
    -Dgbm=disabled \
    -Dllvm=disabled \
    -Dplatforms=x11 \
    -Dbuildtype=release

echo "Building Mesa..."
ninja -C builddir -j"$JOBS"
ninja -C builddir install
echo "Mesa build complete."

# ===================================================================
step "All dependencies built"
echo " Deps: ${PREFIX}/lib"
echo " Qt:   ${PREFIX}/qt6"
echo " Mesa: ${PREFIX}/mesa"
