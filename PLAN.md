# SWAEdit Rebuild Plan — Qt6 + C++

> Star Wars MUD Area Editor. Originally QtJambi 4.5.2 + JOGL 2.0 + JAXB (Java, ~2009).
> Rebuild target: Qt6 + C++ with QOpenGLWidget. Zero dependency on Java.

---

## Phase 0 — Project Skeleton

### 0.1 CMake project setup
- Create `CMakeLists.txt` at project root
- Require Qt6 components: `Widgets`, `OpenGLWidgets`, `Xml`
- Set C++17 standard minimum
- Output binary: `swaedit`

### 0.2 Directory structure
```
src/
  main.cpp
  core/           # XML I/O, renumberer, cloner, flags logic
  model/          # Area data model classes (C++ port of JAXB classes)
  gui/            # All widgets, dialogs, main window
  map/            # 3D map: mapper algorithm, OpenGL widget, room placement
data/             # Existing XML config files (copied as-is from original)
schemas/          # Existing XSD schemas (copied as-is)
resources/        # Icons, images (images/icon.png etc.)
```

### 0.3 Main window shell
- `MainWindow` : `QMainWindow` with empty menu bar, status bar, central `QStackedWidget`
- File menu stubs: New, Open, Save, Save As, Quit
- Action menu stubs (empty for now)
- Application icon from `images/icon.png`
- Loads and runs — smoke test on macOS, Linux, Windows

---

## Phase 1 — Data Model & XML I/O

### 1.1 Area model classes
Port the JAXB-generated model to plain C++ structs/classes. Each class maps 1:1 to the XSD types in `schemas/area.xsd`. Use `QString` for text, `int` for vnums/numbers, `QList` for collections.

Classes to create (reference: `src/main/java/pl/swmud/ns/swmud/_1_0/area/`):
- `Area` — root container, owns all sections below
- `Head` — name, authors, builders, security, vnums (lvnum/uvnum), flags, economy (goldlow/goldhigh), reset (frequency + message), ranges
- `Room` — vnum, name, description, nightDescription, light, flags, sectorType, teledelay, televnum, tunnel
  - `Room::Exit` — direction (0-10), description, keyword, flags, key, vnum (destination), distance
  - `Room::ExtraDesc` — keyword, description
  - `Room::Program` — type, triggerArgs, commands
- `Mobile` — vnum, name, shortDesc, longDesc, description, race, level, actFlags, affectedFlags, alignment, sex, credits, position, dialog
  - `Sectiona` (attributes), `Sections` (stats), `Sectionr`, `Sectionx`, `Sectiont`, `Sectionv`
  - `Mobile::Program` — same structure as room programs
- `Object` — vnum, name, shortDesc, description, actionDesc, type, extraFlags, wearFlags, layers, values[6], weight, cost, gender, level
  - `Object::Affect` — location, modifier
  - `Object::Requirement` — location, modifier, type
  - `Object::ExtraDesc`, `Object::Program`
- `Reset` — command (M/O/P/G/E/H/B/T/D/R/C), extra, arg1-arg4
- `Shop` — keeper (vnum), types[5], profitBuy, profitSell, openHour, closeHour, flags
- `Repair` — keeper, types, profitFix, shopType, openHour, closeHour
- `Special` — vnum, function1, function2

### 1.2 Config/lookup model classes
Port the smaller JAXB models used for the XML config files in `data/`:
- `FlagDef` / `FlagsDef` — for all `*flags.xml` (bit position + name + description)
- `ExitDef` / `ExitsDef` — for `exits.xml` (direction number + name + abbreviation)
- `ItemTypeDef` — for `itemtypes.xml` (type id + name + value field definitions with subvalues)
- `ResetDef` — for `resetsinfo.xml` (command char + description + arg definitions)
- `NameDef` / `NamesDef` — for `races.xml`, `languages.xml`, `planets.xml`, `progtypes.xml`, `mobilespecfunctions.xml`
- `TypeDef` / `TypesDef` — for `positions.xml`, `repairtypes.xml`, `roomsectortypes.xml`
- `HighlighterDef` — for `highlighter.xml` (keyword categories for syntax coloring)

### 1.3 XML I/O (replaces JAXBOperations)
Create `XmlIO` class (in `src/core/`) with static methods:
- `Area loadArea(const QString &filePath)` — parse area XML using `QXmlStreamReader`
- `bool saveArea(const Area &area, const QString &filePath)` — write area XML using `QXmlStreamWriter`
- `FlagsDef loadFlags(const QString &filePath)` — load any flags XML
- `ExitsDef loadExits(const QString &filePath)`
- `ItemTypes loadItemTypes(const QString &filePath)`
- `NamesDef loadNames(const QString &filePath)`
- `TypesDef loadTypes(const QString &filePath)`
- `ResetsDef loadResets(const QString &filePath)`
- `HighlighterDef loadHighlighter(const QString &filePath)`

Reference: `src/main/java/pl/swmud/ns/swaedit/core/JAXBOperations.java`

### 1.4 Validation
- Round-trip test: load a sample area XML, save it, load again, compare
- Verify all 26 config XMLs parse correctly at startup

---

## Phase 2 — Core Editors

### 2.1 Main window wiring (port of SWAEdit.jui + SWAEdit.java)
The main window is the central hub. Port `SWAEdit.jui` to a Qt6 `.ui` file or build programmatically.

Layout (from original):
- **Menu bar**: File (New, Open, Save, Save As, Quit), Action (entity operations, Show Map, Renumber)
- **Left panel**: `QTabWidget` with tabs for each entity type, each containing a `QListWidget` showing entities by vnum + short name
- **Right panel**: `QStackedWidget` switching between editor forms based on selected entity type
- **Status bar**: file name, modified indicator

Startup behavior (from `SWAEdit()` constructor, line 156-203):
- Load all 26 config XMLs into lookup structures
- Set up item value labels, room constants, reset constants, mobile sex dropdown, shop constants
- Install event filters
- Set up system tray
- Start backup timer
- Show `WelcomeScreen`

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/SWAEdit.java`

### 2.2 Area Head editor (port of NewAreaWidget.jui)
Fields: area name, authors, builders, security level, lower vnum, upper vnum, economy gold low/high, reset frequency (minutes), reset message, area flags (via FlagsWidget), level ranges.

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/NewAreaWidget.java`

### 2.3 Room editor (built into main window + NewExitWidget)
Fields: vnum, name, description (`QTextEdit`), night description, light, room flags (via FlagsWidget), sector type (`QComboBox` from `roomsectortypes.xml`), teledelay, televnum, tunnel.

Sub-editors:
- **Exits list** with add/edit/delete — each exit via `NewExitDialog` (port of `NewExitWidget.jui`): direction dropdown (from `exits.xml`), destination vnum, keyword, description, key vnum, exit flags (via FlagsWidget), distance
- **Extra descriptions** via `ExtraDescDialog` (port of `ExtraDescWidget.jui`)
- **Programs** via `ProgramsWidget` (port of `ProgramsWidget.jui`)

References:
- `src/main/java/pl/swmud/ns/swaedit/gui/NewExitWidget.java`
- `src/main/java/pl/swmud/ns/swaedit/gui/ExtraDescWidget.java`
- `src/main/java/pl/swmud/ns/swaedit/gui/ProgramsWidget.java`

### 2.4 Mobile editor (built into main window + MobileSpecialsWidget)
Fields: vnum, name, short desc, long desc, description (`QTextEdit`), race (`QComboBox` from `races.xml`), level, act flags, affected flags, alignment, sex, credits, position (`QComboBox` from `positions.xml`), dialog.

Sections (stat blocks — each a group of spinboxes):
- `Sectiona`: str, int, wis, dex, con, cha, lck, frc
- `Sections`: (savingthrows) poison, para, breath, spell, staff
- `Sectionr`: resist flags
- `Sectionx`: attack flags, defense flags
- `Sectiont`: hitroll, damroll, hitdice (number, size, bonus), damdice, etc.
- `Sectionv`: ac values

Sub-editors:
- **Mobile specials** via `MobileSpecialsWidget` (port of `MobileSpecialsWidget.jui`) — assign spec functions from `mobilespecfunctions.xml`
- **Programs** via `ProgramsWidget`

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/MobileSpecialsWidget.java`

### 2.5 Object editor (built into main window)
Fields: vnum, name, short desc, description, action desc, item type (`QComboBox` from `itemtypes.xml`), extra flags, wear flags, layers, weight, cost, gender, level.

Dynamic value fields: when item type changes, show/hide value[0]-value[5] with labels and widgets appropriate for that type (defined in `itemtypes.xml` — each type specifies which values are active, their labels, and whether they use spinboxes, comboboxes, or flag editors via subvalues).

Sub-editors:
- **Affects list** — add/edit/delete (location + modifier)
- **Requirements list** — add/edit/delete (location + modifier + type)
- **Extra descriptions** via `ExtraDescDialog`
- **Programs** via `ProgramsWidget`

### 2.6 Reusable FlagsWidget (port of FlagsWidget.jui + SingleFlagWidget + ValueFlagsWidget + ResetFlagsWidget)
A reusable dialog/widget that displays a grid of checkboxes, one per bit flag. Used everywhere flags are edited (area flags, room flags, exit flags, item wear/extra, mobile act/affected, resist, attack, defense, shop, x-flags).

- `FlagsWidget` — takes a `FlagsDef` and a current `int` value, shows checkboxes, returns updated value
- `SingleFlagWidget` — one checkbox + label for a single flag bit
- `ValueFlagsWidget` — flags with an associated numeric value (used for some special flag types)
- `ResetFlagsWidget` — flags specific to reset commands

References:
- `src/main/java/pl/swmud/ns/swaedit/gui/FlagsWidget.java`
- `src/main/java/pl/swmud/ns/swaedit/gui/SingleFlagWidget.java`
- `src/main/java/pl/swmud/ns/swaedit/gui/ValueFlagsWidget.java`
- `src/main/java/pl/swmud/ns/swaedit/gui/ResetFlagsWidget.java`

---

## Phase 3 — Supporting Editors & Utilities

### 3.1 Resets editor (port of NewResetWidget.jui)
- List of reset commands in sequence order
- Each reset: command char (M/O/P/G/E/H/B/T/D/R/C), args 1-4 with labels that change based on command type (defined in `resetsinfo.xml`)
- Add/edit/delete/reorder resets

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/NewResetWidget.java`

### 3.2 Shops editor (port of NewShopWidget.jui)
Fields: keeper (mobile vnum), buyable item types[5] (comboboxes), profit buy %, profit sell %, open hour, close hour, shop flags.

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/NewShopWidget.java`

### 3.3 Repairs editor
Fields: keeper (mobile vnum), repair types, profit fix, shop type, open hour, close hour. Similar layout to shops.

### 3.4 Specials editor
List of special function assignments: mobile vnum + function1 + function2 (from `mobilespecfunctions.xml`).

### 3.5 Programs editor with syntax highlighting
- `ProgramsWidget` — type dropdown (from `progtypes.xml`), trigger args field, multi-line commands editor (`QPlainTextEdit`)
- `ProgramsHighlighter` : `QSyntaxHighlighter` — port highlighting rules from `highlighter.xml` (keyword categories with colors)

Reference:
- `src/main/java/pl/swmud/ns/swaedit/gui/ProgramsHighlighter.java`
- `data/highlighter.xml`

### 3.6 Renumberer (port of Renumberer.java)
- Takes old vnum + new vnum
- Cascades the change through ALL references: room exits, resets, shops, repairs, specials
- `RenumberDialog` (port of `RenumberWidget.jui`) — input old/new vnum
- `RenumberWarningsDialog` (port of `RenumberWarningsWidget.jui`) — show conflicts before applying

Reference: `src/main/java/pl/swmud/ns/swaedit/core/Renumberer.java`

### 3.7 Cloner (port of Cloner.java)
Deep-copy any entity (room, mobile, object) with a new vnum. Must clone all nested structures (exits, programs, affects, etc.).

Reference: `src/main/java/pl/swmud/ns/swaedit/core/Cloner.java`

### 3.8 Event filters
- `ExitEventFilter` — validates exit-related input fields
- `ResetVnumEventFilter` — validates vnum input in reset editor
- `ToolTipEventFilter` — rich tooltips on hover

References:
- `src/main/java/pl/swmud/ns/swaedit/gui/ExitEventFilter.java`
- `src/main/java/pl/swmud/ns/swaedit/gui/ResetVnumEventFilter.java`
- `src/main/java/pl/swmud/ns/swaedit/gui/ToolTipEventFilter.java`

---

## Phase 4 — 3D Map

### 4.1 Mapper algorithm (port of Mapper.java)
The mapper converts the flat list of rooms + exits into a spatial graph with 3D coordinates. Key behavior to preserve:

**Island detection**: Rooms that are not connected via exits form separate "islands". The algorithm:
1. Iterate all rooms in the area
2. For each unvisited room, assign it as the root of a new island (increment `islandNo`)
3. BFS/DFS walk: follow each exit from the current room, compute child coordinates by direction offset (N: z-1, E: x+1, S: z+1, W: x-1, up: y+1, down: y-1, NE: z-1/x+1, NW: z-1/x-1, SE: z+1/x+1, SW: z+1/x-1)
4. Skip already-visited rooms (avoids cycles)
5. After all rooms placed: assign reverse exits, detect coordinate collisions (rooms at same coords get offset to different layers), group rooms by island number

Data structures:
- `MapRoom` — wraps a `Room` with `RoomCoords` (x, y, z, islandNo, layer), parent exit, child map {ExitWrapper -> MapRoom}
- `RoomCoords` — (x, y, z, islandNo, layer), supports clone, distance, equals/hash
- `RoomSpread` — computes spatial extent (min/max) of an island along each axis, used for centering
- `ExitWrapper` — wraps an Exit with: reverse exit reference, drawn flag, distant flag, reverse direction lookup, direction name lookup, comparable by direction

Output: `QMap<int, QList<MapRoom*>> islandRooms` — rooms grouped by island number.

References:
- `src/main/java/pl/swmud/ns/swaedit/map/Mapper.java` (lines 29-45: createMap, lines 77-93: recursive makeMapRoom, lines 217-227: createIslands)
- `src/main/java/pl/swmud/ns/swaedit/map/MapRoom.java`
- `src/main/java/pl/swmud/ns/swaedit/map/RoomCoords.java`
- `src/main/java/pl/swmud/ns/swaedit/map/RoomSpread.java`
- `src/main/java/pl/swmud/ns/swaedit/map/ExitWrapper.java`

### 4.2 MapWidget — QOpenGLWidget subclass
- Renders rooms as 3D boxes/nodes, exits as lines/edges between them
- Two-way exits rendered differently from one-way exits
- Distant exits (distance > 1 in coord space) rendered with dashed/different style
- Current island selector (dropdown or prev/next buttons to switch between islands)
- Camera controls: scroll to zoom, drag to rotate, middle-drag to pan
- Click room to select it (highlight, emit signal to main window to select in editor)
- Room marking/flagging visible on map (visual indicator on marked rooms)
- 30 FPS refresh via `QTimer` (port of `Animator.java`)

### 4.3 Text rendering — QPainter overlay
Use `QPainter` on top of `QOpenGLWidget` for room labels (vnum and/or short name). QPainter provides:
- Native font rendering with full antialiasing and subpixel hinting
- Retina/HiDPI support automatically
- No texture atlas management needed (replaces the original `GLFont.java` which built a 10x10 character grid texture)

Implementation: in `QOpenGLWidget::paintGL()`, after OpenGL rendering, call `QPainter painter(this)` and draw text labels at projected screen coordinates of each room.

### 4.4 Integration with main window
- Action menu -> "Show Map" creates/shows the MapWidget for the current area
- Selecting a room on the map selects it in the room list/editor
- Selecting a room in the editor highlights it on the map
- "Refresh Map" action rebuilds the map from current room/exit data

---

## Phase 5 — Polish

### 5.1 System tray
- `QSystemTrayIcon` with app icon
- Context menu: Quit
- Click to toggle main window visibility (minimize to tray)

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/SWAEdit.java` lines 215-243

### 5.2 Autosave / backup timer
- `QTimer` fires periodically (configurable interval)
- Saves a backup copy of the current area file

Reference: `SWAEdit.java` line 133, 191

### 5.3 Welcome screen
- `WelcomeScreen` shown on startup
- App info, recent files (optional enhancement)

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/WelcomeScreen.java`

### 5.4 Balloon / notification widget
- `BalloonWidget` for tooltip-style popups and status notifications

Reference: `src/main/java/pl/swmud/ns/swaedit/gui/BalloonWidget.java`

### 5.5 Error logging
- Redirect stderr to `swaedit_err.log`
- Redirect stdout to `swaedit_out.log`

### 5.6 Undo/Redo (new — not in original)
- `QUndoStack` with undo commands for all entity modifications
- Ctrl+Z / Ctrl+Shift+Z shortcuts
- Edit menu: Undo, Redo

### 5.7 Packaging
- CPack configuration in CMake
- macOS: `.dmg` bundle
- Linux: `.AppImage`
- Windows: `.msi` / portable `.zip`

---

## File-to-file mapping reference

| Original Java file | New C++ file(s) | Phase |
|---|---|---|
| `core/JAXBOperations.java` | `src/core/XmlIO.h/.cpp` | 1 |
| `core/Renumberer.java` | `src/core/Renumberer.h/.cpp` | 3 |
| `core/Cloner.java` | `src/core/Cloner.h/.cpp` | 3 |
| `core/FlagsWrapper.java` | `src/core/FlagsWrapper.h/.cpp` | 2 |
| `core/ResetWrapper.java` | `src/core/ResetWrapper.h/.cpp` | 3 |
| `core/IFlagsSetter.java` | (interface folded into FlagsWidget) | 2 |
| `swmud/_1_0/area/*.java` (72 files) | `src/model/Area.h/.cpp` + ~12 model files | 1 |
| `gui/SWAEdit.java` + `.jui` | `src/gui/MainWindow.h/.cpp/.ui` | 2 |
| `gui/FlagsWidget.java` + `.jui` | `src/gui/FlagsWidget.h/.cpp/.ui` | 2 |
| `gui/SingleFlagWidget.java` | `src/gui/SingleFlagWidget.h/.cpp` | 2 |
| `gui/ValueFlagsWidget.java` + `.jui` | `src/gui/ValueFlagsWidget.h/.cpp/.ui` | 2 |
| `gui/ResetFlagsWidget.java` | `src/gui/ResetFlagsWidget.h/.cpp` | 2 |
| `gui/NewAreaWidget.java` + `.jui` | `src/gui/NewAreaDialog.h/.cpp/.ui` | 2 |
| `gui/NewExitWidget.java` + `.jui` | `src/gui/NewExitDialog.h/.cpp/.ui` | 2 |
| `gui/NewResetWidget.java` + `.jui` | `src/gui/NewResetDialog.h/.cpp/.ui` | 3 |
| `gui/NewShopWidget.java` + `.jui` | `src/gui/NewShopDialog.h/.cpp/.ui` | 3 |
| `gui/ExtraDescWidget.java` + `.jui` | `src/gui/ExtraDescDialog.h/.cpp/.ui` | 2 |
| `gui/ProgramsWidget.java` + `.jui` | `src/gui/ProgramsWidget.h/.cpp/.ui` | 3 |
| `gui/ProgramsHighlighter.java` | `src/gui/ProgramsHighlighter.h/.cpp` | 3 |
| `gui/MobileSpecialsWidget.java` + `.jui` | `src/gui/MobileSpecialsWidget.h/.cpp/.ui` | 2 |
| `gui/MessagesWidget.java` + `.jui` | `src/gui/MessagesWidget.h/.cpp/.ui` | 5 |
| `gui/RenumberWidget.java` + `.jui` | `src/gui/RenumberDialog.h/.cpp/.ui` | 3 |
| `gui/RenumberWarningsWidget.java` + `.jui` | `src/gui/RenumberWarningsDialog.h/.cpp/.ui` | 3 |
| `gui/VnumQuestionWidget.java` + `.jui` | `src/gui/VnumQuestionDialog.h/.cpp/.ui` | 2 |
| `gui/BalloonWidget.java` | `src/gui/BalloonWidget.h/.cpp` | 5 |
| `gui/WelcomeScreen.java` | `src/gui/WelcomeScreen.h/.cpp` | 5 |
| `gui/ExitEventFilter.java` | `src/gui/ExitEventFilter.h/.cpp` | 3 |
| `gui/ResetVnumEventFilter.java` | `src/gui/ResetVnumEventFilter.h/.cpp` | 3 |
| `gui/ToolTipEventFilter.java` | `src/gui/ToolTipEventFilter.h/.cpp` | 3 |
| `map/Mapper.java` | `src/map/Mapper.h/.cpp` | 4 |
| `map/MapRoom.java` | `src/map/MapRoom.h/.cpp` | 4 |
| `map/RoomCoords.java` | `src/map/RoomCoords.h/.cpp` | 4 |
| `map/RoomSpread.java` | `src/map/RoomSpread.h/.cpp` | 4 |
| `map/ExitWrapper.java` | `src/map/ExitWrapper.h/.cpp` | 4 |
| `map/Animator.java` | (inlined as QTimer in MapWidget) | 4 |
| `map/GLFont.java` | (replaced by QPainter overlay in MapWidget) | 4 |
| `gui/MapWidget.java` | `src/map/MapWidget.h/.cpp` | 4 |
| `core/FileServer.java` | **DROPPED** | — |

---

## Existing data files (use as-is, no changes needed)

All files in `data/` and `schemas/` are XML configs and XSD schemas that define the game's constants. They are loaded at startup and drive the editor's dropdowns, flag checkboxes, and validation. Copy them into the new build verbatim.

```
data/areaflags.xml          data/exitflags.xml         data/exits.xml
data/highlighter.xml        data/itemextraflags.xml    data/itemtypes.xml
data/itemwearflags.xml      data/languages.xml         data/mobileactflags.xml
data/mobileaffectedflags.xml data/mobilespecfunctions.xml data/planets.xml
data/positions.xml          data/progtypes.xml         data/races.xml
data/repairtypes.xml        data/resetsinfo.xml        data/resistflags.xml
data/roomflags.xml          data/roomsectortypes.xml   data/shopflags.xml
data/xflags.xml             data/attackflags.xml       data/defenseflags.xml
data/usmessages.xml
schemas/area.xsd            schemas/exits.xsd          schemas/flags.xsd
schemas/highlighter.xsd     schemas/itemtypes.xsd      schemas/names.xsd
schemas/types.xsd           schemas/resets.xsd         schemas/lastupdate.xsd
schemas/progtypes.xsd       schemas/stringtypes.xsd    schemas/usmessages.xsd
schemas/usprotocol.xsd
```
