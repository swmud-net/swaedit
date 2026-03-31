# Maximally Static Linux Build Plan

## Goal

Produce a portable x86_64 Linux binary of swaedit with minimal runtime dependencies, packaged as `.7z`. Built in GitHub Actions on `ubuntu-24.04`.

**Runtime dependencies of the final binary:**
- `libc.so.6` + `ld-linux-x86-64.so.2` (glibc — present on every Linux distro)
- `libGL.so.1` (optional — bundled Mesa software renderer as fallback)

Everything else (Qt, libxml2, X11/xcb, freetype, fontconfig, harfbuzz, zlib, pcre2, libstdc++, libgcc) is **statically linked**.

---

## Why Not Fully Static glibc?

Static glibc + dynamic `libGL.so` is architecturally impossible:

1. `libGL.so` (whether Mesa or NVIDIA) is linked against `libc.so.6` (dynamic glibc).
2. If the main binary has glibc statically linked, loading `libGL.so` via `dlopen()` brings a **second copy** of glibc into the process.
3. Two copies of glibc = dual `malloc`/`free` heaps, dual TLS, dual `errno`, dual `pthread` internals. This corrupts memory and crashes.

This is not theoretical — it is the documented reason that **no production Linux GUI binary** (Chrome, Steam, Firefox, Electron) statically links glibc. glibc's own maintainers call this "completely unsupported."

NSS is a non-issue here: with dynamic glibc, NSS works normally (glibc `dlopen()`s NSS modules from the system as usual). The app itself does not perform DNS lookups or user/group resolution, so NSS is not exercised in practice.

**Bottom line:** glibc stays dynamic. It is on every Linux system. This is not a portability concern.

---

## Approach: Maximally Static

| Component | Linking | Why |
|---|---|---|
| glibc (libc, libm, libpthread, libdl) | **Dynamic** | Cannot be static with dlopen; dual-glibc problem |
| libGL.so (OpenGL) | **Dynamic** | Hardware-specific, must match user's GPU driver |
| libstdc++, libgcc | **Static** | Avoids C++ ABI version mismatches across distros |
| Qt 6.8.x (Widgets, OpenGLWidgets, Xml) | **Static** | Full control over version |
| libxml2 | **Static** | App dependency |
| X11/xcb stack (libxcb, libX11, etc.) | **Static** | X11 protocol is stable; safe to statically link |
| Font stack (freetype, fontconfig, harfbuzz) | **Static** | Avoids version mismatches |
| Compression (zlib, libpng) | **Static** | Leaf dependencies |
| pcre2 | **Static** | Qt dependency |
| Mesa (libGL.so, softpipe) | **Bundled .so** | Fallback for systems without OpenGL |

### OpenGL Fallback Mechanism

1. Binary starts, Qt creates a GLX context using the system's `libGL.so.1`.
2. If system has a GPU driver (Mesa, NVIDIA, AMD) → hardware-accelerated GL.
3. If system has no `libGL.so.1` → Qt fails to create GL context.
4. Fallback: user runs via the included launcher script, which sets `LD_LIBRARY_PATH` to the bundled Mesa directory.

Launcher script (`swaedit.sh`):
```bash
#!/bin/sh
DIR="$(cd "$(dirname "$0")" && pwd)"
if ldconfig -p 2>/dev/null | grep -q 'libGL\.so\.1'; then
    exec "$DIR/swaedit" "$@"
else
    echo "No system OpenGL found — using software renderer"
    LD_LIBRARY_PATH="$DIR/lib" exec "$DIR/swaedit" "$@"
fi
```

### Font Fallback

Bundle **DejaVu Sans** (Regular + Bold, ~1.4 MB total):
- Qt's own source code recommends DejaVu as the fallback font.
- Default sans-serif on most Linux distros — consistent rendering.
- 6,253 glyphs including box-drawing and symbols (useful for MUD content).

Ship a minimal `fonts.conf` that adds the bundled font directory as a weak fallback:
```xml
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<fontconfig>
  <!-- Standard system font dirs (checked first) -->
  <dir>/usr/share/fonts</dir>
  <dir>/usr/local/share/fonts</dir>
  <dir>~/.fonts</dir>
  <!-- Bundled fallback font -->
  <dir prefix="relative">fonts</dir>
  <cachedir>/tmp/swaedit-fontcache</cachedir>
</fontconfig>
```

In `main.cpp`, set `FONTCONFIG_PATH` before `QApplication` creation:
```cpp
qputenv("FONTCONFIG_PATH", QCoreApplication::applicationDirPath().toUtf8());
```

---

## Dependencies — Build Order

All built from source as static archives (`.a`) with `-fPIC`. No cross-compilation needed (native x86_64 build on ubuntu-24.04).

### Tier 1 — Leaf libraries (no deps on each other)

| Library | Purpose | Build system |
|---|---|---|
| zlib | Compression | cmake |
| bzip2 | Compression (freetype) | make |
| expat | XML parser (fontconfig) | cmake |
| pcre2 | Regex (Qt Core) | cmake |
| util-macros | X11 build macros | autotools |
| xcb-proto | XCB protocol definitions (data only) | autotools |
| xorgproto | X11 protocol headers | meson |
| xtrans | X11 transport abstraction | autotools |

### Tier 2 — Mid-level

| Library | Depends on | Build system |
|---|---|---|
| libpng | zlib | cmake |
| libxml2 | zlib | cmake |
| libXau | xorgproto | autotools |
| libXdmcp | xorgproto | autotools |
| libICE | xorgproto, xtrans | autotools |
| libSM | libICE | autotools |

### Tier 3 — X11/XCB stack

| Library | Depends on | Build system |
|---|---|---|
| libxcb | xcb-proto, libXau, libXdmcp | autotools |
| xcb-util | libxcb | autotools |
| xcb-util-wm | libxcb | autotools |
| xcb-util-image | libxcb, xcb-util | autotools |
| xcb-util-keysyms | libxcb | autotools |
| xcb-util-renderutil | libxcb | autotools |
| xcb-util-cursor | libxcb, xcb-util-image, xcb-util-renderutil | autotools |
| libX11 | libxcb, xtrans, xorgproto | autotools |
| libXext | libX11 | autotools |
| libxshmfence | libxcb | autotools |
| libxkbcommon | libxcb | meson |

### Tier 4 — Font & text rendering

| Library | Depends on | Notes |
|---|---|---|
| freetype (pass 1) | zlib, libpng, bzip2 | Without harfbuzz |
| fontconfig | freetype, expat | |
| harfbuzz | freetype | |
| freetype (pass 2) | zlib, libpng, bzip2, harfbuzz | Rebuild with harfbuzz (circular dep) |

### Tier 5 — Qt 6.8.x

Qt is the largest dependency. Only `qtbase` is needed (Widgets, OpenGLWidgets, Xml are all in qtbase).

Qt configure:
```
./configure \
  -prefix $PREFIX/qt6 \
  -release \
  -static \
  -no-icu \
  -system-zlib \
  -system-libpng \
  -system-freetype \
  -fontconfig \
  -system-harfbuzz \
  -system-pcre \
  -xcb \
  -bundled-xcb-xinput \
  -opengl desktop \
  -no-eglfs \
  -no-wayland \
  -no-cups \
  -no-dbus \
  -nomake examples \
  -nomake tests \
  -- \
  -DCMAKE_PREFIX_PATH=$PREFIX \
  -DCMAKE_C_FLAGS="-fPIC" \
  -DCMAKE_CXX_FLAGS="-fPIC"
```

### Mesa (for bundled fallback .so only)

Mesa is built as a **shared library** (not static — it's the bundled fallback `libGL.so.1`):

```
meson setup build \
  --prefix=$PREFIX/mesa \
  --default-library=shared \
  -Dgallium-drivers=softpipe \
  -Dvulkan-drivers= \
  -Dglx=xlib \
  -Degl=disabled \
  -Dgbm=disabled \
  -Dllvm=disabled \
  -Dplatforms=x11
ninja -C build
ninja -C build install
```

Only `libGL.so.1` (and its internal deps like `libglapi.so`) are copied to the package. These are loaded via `LD_LIBRARY_PATH` only when no system GL is found.

---

## Build Steps

### Step 1: Install build-time tools

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build pkg-config \
  python3 python3-pip python3-mako python3-yaml \
  autoconf automake libtool gettext bison flex gperf \
  p7zip-full libgl-dev
pip3 install meson
```

Note: `libgl-dev` is needed only for OpenGL headers during Qt/swaedit compile. The actual `libGL.so` at runtime comes from the user's system (or bundled Mesa).

### Step 2: Set up staging prefix

```bash
export PREFIX=/opt/static-deps
mkdir -p $PREFIX/{lib,include,share/pkgconfig}
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:$PREFIX/share/pkgconfig"
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig:$PREFIX/share/pkgconfig"
export CFLAGS="-fPIC -O2"
export CXXFLAGS="-fPIC -O2"
```

### Step 3: Build Tier 1-4 dependencies

Each library follows one of these patterns:

```bash
# autotools:
./configure --prefix=$PREFIX --enable-static --disable-shared
make -j$(nproc) && make install

# cmake:
cmake -B build -DCMAKE_INSTALL_PREFIX=$PREFIX \
  -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build -j$(nproc) && cmake --install build

# meson:
meson setup build --prefix=$PREFIX --default-library=static
ninja -C build && ninja -C build install
```

### Step 4: Build Qt 6.8.x (static)

```bash
wget -q https://download.qt.io/official_releases/qt/6.8/<LATEST>/single/qt-everywhere-src-<LATEST>.tar.xz
tar xf qt-everywhere-src-*.tar.xz && cd qt-everywhere-src-*

./configure [flags from Tier 5 section]
cmake --build build -j$(nproc)
cmake --install build
```

### Step 5: Build Mesa (shared, for bundled fallback)

```bash
# Mesa built with system compiler, shared libs, for the fallback .so
meson setup build [flags from Mesa section]
ninja -C build && ninja -C build install
```

### Step 6: Build swaedit

```bash
cmake -B build-static \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$PREFIX/qt6;$PREFIX" \
  -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++" \
  -DCMAKE_FIND_ROOT_PATH="$PREFIX"

cmake --build build-static -j$(nproc)
strip build-static/swaedit
```

### Step 7: Verify

```bash
# Check dynamic deps — should only show glibc + libGL
ldd build-static/swaedit
# Expected output (approximately):
#   linux-vdso.so.1
#   libGL.so.1 => /usr/lib/x86_64-linux-gnu/libGL.so.1
#   libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6
#   /lib64/ld-linux-x86-64.so.2
```

### Step 8: Package as .7z

```bash
mkdir -p package/swaedit/{lib,fonts}

# Binary + data
cp build-static/swaedit package/swaedit/
cp -r data schemas images package/swaedit/

# Mesa fallback
cp $PREFIX/mesa/lib/libGL.so.1* package/swaedit/lib/
cp $PREFIX/mesa/lib/libglapi.so* package/swaedit/lib/ 2>/dev/null || true

# Fallback font
cp fonts/DejaVuSans.ttf fonts/DejaVuSans-Bold.ttf package/swaedit/fonts/

# Fontconfig
cp fonts.conf package/swaedit/

# Launcher script
cp swaedit.sh package/swaedit/
chmod +x package/swaedit/swaedit.sh

cd package && 7z a -mx=9 ../SWAEdit-linux-x86_64.7z swaedit/
```

---

## GitHub Actions Workflow

### Caching strategy

Full dependency build (Tier 1-5) takes ~45-75 minutes. Cache the built prefix:

```yaml
- name: Cache static dependencies
  id: cache-deps
  uses: actions/cache@v4
  with:
    path: /opt/static-deps
    key: static-deps-v${{ env.DEPS_VERSION }}
```

- `DEPS_VERSION` bumped manually when dependency versions change.
- With cache hit: ~3-5 min (only swaedit itself).
- Cache miss: ~45-75 min (full dependency build).

### New job in build.yml

```yaml
build-linux-portable:
  runs-on: ubuntu-24.04
  steps:
    - uses: actions/checkout@v4

    - name: Install build tools
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          build-essential cmake ninja-build pkg-config \
          python3 python3-pip python3-mako \
          autoconf automake libtool bison flex gperf \
          p7zip-full libgl-dev
        pip3 install meson

    - name: Cache static dependencies
      id: cache-deps
      uses: actions/cache@v4
      with:
        path: /opt/static-deps
        key: static-deps-v1

    - name: Build static dependencies (if not cached)
      if: steps.cache-deps.outputs.cache-hit != 'true'
      run: ./scripts/build-static-deps.sh

    - name: Build swaedit
      run: ./scripts/build-swaedit-static.sh

    - name: Verify minimal dynamic deps
      run: |
        echo "=== file ==="
        file build-static/swaedit
        echo "=== ldd ==="
        ldd build-static/swaedit
        echo "=== checking deps ==="
        # Fail if any unexpected dynamic deps beyond glibc + libGL
        ldd build-static/swaedit | grep -v -E '(linux-vdso|libGL|libGLX|libGLdispatch|libc\.so|libm\.so|libdl\.so|libpthread\.so|ld-linux)' \
          | grep '=>' && echo "ERROR: unexpected dynamic deps" && exit 1 || true

    - name: Package .7z
      run: |
        mkdir -p package/swaedit/{lib,fonts}
        cp build-static/swaedit package/swaedit/
        cp -r data schemas images package/swaedit/
        cp /opt/static-deps/mesa/lib/libGL.so.1* package/swaedit/lib/ || true
        cp fonts/DejaVuSans.ttf fonts/DejaVuSans-Bold.ttf package/swaedit/fonts/
        cp fonts.conf package/swaedit/
        cp swaedit.sh package/swaedit/
        chmod +x package/swaedit/swaedit.sh
        cd package && 7z a -mx=9 ../SWAEdit-linux-x86_64.7z swaedit/

    - name: Upload artifact
      if: startsWith(github.ref, 'refs/tags/v')
      uses: actions/upload-artifact@v4
      with:
        name: swaedit-linux
        path: SWAEdit-linux-x86_64.7z
```

The existing `build-linux` job remains as a fast CI smoke test.

The `release` job gains an additional artifact download + publish for the Linux `.7z`.

### Scripts to create

| Script | Purpose |
|---|---|
| `scripts/build-static-deps.sh` | Downloads + builds all Tier 1-5 deps into `/opt/static-deps` |
| `scripts/build-swaedit-static.sh` | Configures + builds swaedit against the static prefix |

---

## Package Contents (.7z)

```
SWAEdit-linux-x86_64.7z
└── swaedit/
    ├── swaedit              # main binary
    ├── swaedit.sh           # launcher (Mesa fallback)
    ├── data/                # area XML data files
    ├── schemas/             # XML schemas
    ├── images/              # icons
    ├── fonts/
    │   ├── DejaVuSans.ttf
    │   └── DejaVuSans-Bold.ttf
    ├── fonts.conf           # fontconfig fallback config
    └── lib/
        └── libGL.so.1       # Mesa softpipe fallback
```

Estimated compressed size: ~20-35 MB.

---

## Portability: glibc Version Targeting

Building on `ubuntu-24.04` links against glibc 2.39. The binary will require glibc >= 2.39 at runtime.

**To support older distros**, build on an older runner:
- `ubuntu-22.04` → glibc 2.35 (covers most current distros)
- `ubuntu-20.04` → glibc 2.31 (covers distros from 2020+)

This is a future improvement. Starting with `ubuntu-24.04` is fine for initial implementation.

---

## Risk Register

| Risk | Impact | Mitigation |
|---|---|---|
| Qt static build fails to find xcb deps | Build blocked | Set explicit `CMAKE_PREFIX_PATH`; use Qt's `-bundled-xcb-xinput` |
| fontconfig can't find fonts at runtime | Blank text | Bundled `fonts.conf` + DejaVu Sans; `FONTCONFIG_PATH` env var |
| Mesa `glx=xlib` removed in latest version | No fallback GL | Pin Mesa version, or use `glx=dri` with softpipe |
| Binary too large (>150 MB uncompressed) | Distribution size | `strip` binary; `.7z` compression; already addressed |
| Cache size (~1-2 GB) | GitHub cache limits | Well within 10 GB limit |
| First build exceeds GH Actions time limit | Build fails | 45-75 min is within 6-hour limit; parallelize dep builds |
| libGL.so linked at build time prevents portable binary | Wrong GL at runtime | Link with `-lGL` but ensure it resolves dynamically; verify with `ldd` |
| Bundled Mesa conflicts with system Mesa via GLVND | Wrong GL used | Launcher script only sets `LD_LIBRARY_PATH` when no system GL exists |

---

## Implementation Order

1. Create `scripts/build-static-deps.sh` — build all deps into `/opt/static-deps`
2. Create `scripts/build-swaedit-static.sh` — build swaedit against the prefix
3. Test locally on an Ubuntu 24.04 machine/VM/container
4. Add `fonts/`, `fonts.conf`, `swaedit.sh` to the repository
5. Add `build-linux-portable` job to `.github/workflows/build.yml`
6. Update `release` job to include the Linux `.7z`
7. Test full CI pipeline with a tag push
