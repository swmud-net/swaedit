#!/bin/bash
#
# Build Qt 6.8.x (static) and Mesa (softpipe fallback) for the
# maximally-static swaedit Linux binary.
#
# Prerequisites: system -dev packages must be installed (see workflow).
# Output: /opt/static-deps/qt6  — static Qt6 libraries
#         /opt/static-deps/mesa — Mesa libGL.so (softpipe software renderer)
#
set -euo pipefail

PREFIX="${STATIC_DEPS_PREFIX:-/opt/static-deps}"
QT_VER="${QT_VERSION:-6.8.1}"
QT_MAJOR_MINOR="$(echo "$QT_VER" | cut -d. -f1-2)"
MESA_VER="${MESA_VERSION:-23.3.6}"
JOBS="$(nproc)"
SRC_DIR="/tmp/static-deps-build"

mkdir -p "$PREFIX" "$SRC_DIR"

# ===================================================================
# Qt 6.8.x — static build
# ===================================================================
echo ""
echo "========================================"
echo " Building Qt ${QT_VER} (static)"
echo "========================================"
echo ""

cd "$SRC_DIR"

QT_SRC="qtbase-everywhere-src-${QT_VER}"
QT_TAR="${QT_SRC}.tar.xz"
QT_URL="https://download.qt.io/official_releases/qt/${QT_MAJOR_MINOR}/${QT_VER}/submodules/${QT_TAR}"

if [ ! -f "$QT_TAR" ]; then
    echo "Downloading Qt ${QT_VER}..."
    wget -q --show-progress "$QT_URL" -O "$QT_TAR"
fi

if [ ! -d "$QT_SRC" ]; then
    echo "Extracting Qt sources..."
    tar xf "$QT_TAR"
fi

cd "$QT_SRC"

# Clean any previous build
rm -rf build

echo "Configuring Qt (static, xcb, desktop OpenGL)..."
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
    -DCMAKE_C_FLAGS="-fPIC" \
    -DCMAKE_CXX_FLAGS="-fPIC" \
    -DFEATURE_eglfs=OFF \
    -DFEATURE_cups=OFF \
    -DFEATURE_dbus=OFF

echo "Building Qt (this takes 20-40 minutes)..."
cmake --build . -j"$JOBS"

echo "Installing Qt to ${PREFIX}/qt6..."
cmake --install .

echo "Qt build complete."

# ===================================================================
# Mesa — softpipe software renderer (for bundled libGL.so fallback)
# ===================================================================
echo ""
echo "========================================"
echo " Building Mesa ${MESA_VER} (softpipe)"
echo "========================================"
echo ""

cd "$SRC_DIR"

MESA_SRC="mesa-${MESA_VER}"
MESA_TAR="${MESA_SRC}.tar.xz"
MESA_URL="https://archive.mesa3d.org/${MESA_TAR}"

if [ ! -f "$MESA_TAR" ]; then
    echo "Downloading Mesa ${MESA_VER}..."
    wget -q --show-progress "$MESA_URL" -O "$MESA_TAR"
fi

if [ ! -d "$MESA_SRC" ]; then
    echo "Extracting Mesa sources..."
    tar xf "$MESA_TAR"
fi

cd "$MESA_SRC"

# Clean any previous build
rm -rf builddir

# Build Mesa with gallium-xlib GLX + softpipe driver.
# Produces a standalone libGL.so (no GLVND) with software rendering.
echo "Configuring Mesa (softpipe, gallium-xlib GLX)..."
meson setup builddir \
    --prefix="$PREFIX/mesa" \
    -Dgallium-drivers=softpipe \
    -Dvulkan-drivers= \
    -Dglx=gallium-xlib \
    -Degl=disabled \
    -Dgbm=disabled \
    -Dllvm=disabled \
    -Dplatforms=x11 \
    -Dbuildtype=release

echo "Building Mesa..."
ninja -C builddir -j"$JOBS"

echo "Installing Mesa to ${PREFIX}/mesa..."
ninja -C builddir install

echo "Mesa build complete."

# ===================================================================
echo ""
echo "========================================"
echo " All dependencies built successfully"
echo "========================================"
echo " Qt:   ${PREFIX}/qt6"
echo " Mesa: ${PREFIX}/mesa"
echo ""
