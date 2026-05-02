# Dynamic Docksets

Dynamic Docksets is a third-party dock layout manager plugin for OBS Studio.
It lets you save, restore, cycle, and automate OBS dock layouts as named
"docksets".

The plugin is built for streamers and operators who switch between different
workspace layouts, for example a clean streaming layout, a setup/editing
layout, a fullscreen preview layout, or collection-specific production layouts.

## Features

- Manager dock with all saved docksets, thumbnail previews, and actions for
  save, save as, load, rename, reorder, and delete.
- Quick Access dock with one compact button per saved dockset.
- Tools menu integration for saving, loading, and cycling layouts.
- OBS hotkeys for cycling to the next or previous layout.
- Optional auto-save for the active dockset with a 5 second throttle.
- Optional auto-load mapping when switching scene collections.
- Stream Deck friendly shortcut files: one generated `.vbs` file per dockset,
  plus cycle-next and cycle-previous shortcuts.
- Optional obs-websocket vendor requests for automation tools such as
  Streamer.bot.

## Stream Deck Shortcuts

Dynamic Docksets can generate small `.vbs` shortcut files for every saved
dockset. A Stream Deck button can run one of these files with the built-in
`System: Open` action, so no WebSocket setup is required.

The generated files are stored in the plugin configuration directory:

```text
config/obs-studio/plugin_config/dynamic-docksets/shortcuts/
```

The plugin also creates `_Cycle Next.vbs` and `_Cycle Prev.vbs` for stepping
through layouts.

## Requirements

- OBS Studio 30.x, 31.x, or 32.x
- Windows x64
- Qt 6, provided by OBS Studio
- Optional: obs-websocket for vendor request automation

## Installation

Recommended installer:

1. Download the `*-setup.exe` file from the latest GitHub release.
2. Close OBS Studio.
3. Run the installer and select your OBS Studio installation folder.
4. Start OBS Studio again.

Portable or manual installation:

1. Download the release ZIP instead of the setup EXE.
2. Extract the ZIP into your OBS Studio installation folder, or copy the
   included `obs-plugins` and `data` folders into it.
3. Start OBS Studio again.

### Windows

Download the release archive and extract or copy its contents into your OBS
Studio installation directory.

The final layout should include:

```text
obs-plugins/64bit/dynamic-docksets.dll
data/obs-plugins/dynamic-docksets/locale/en-US.ini
data/obs-plugins/dynamic-docksets/locale/de-DE.ini
```

The packaged release also includes `INSTALL.bat`, which can copy the plugin
files into a selected OBS directory.

Restart OBS after installation. The docks appear under:

```text
View -> Docks -> Dynamic Docks - Manager
View -> Docks -> Dynamic Docks - Quick Access
```

## Basic Usage

1. Open the Manager dock from `View -> Docks`.
2. Arrange your OBS docks the way you want.
3. Click `Save as...` and give the layout a name.
4. Change your OBS workspace.
5. Double-click the saved dockset or use the Quick Access dock to restore it.

For Stream Deck usage, open the shortcuts folder from the Manager dock and
assign the generated `.vbs` file to a Stream Deck `System: Open` action.

## WebSocket Vendor Requests

Dynamic Docksets registers the vendor name `DynamicDocksets`. It also registers
`jrDockie` as a compatibility alias when available.

Supported requests:

- `LoadDockset`
- `CycleDockset`
- `SaveDockset`

Example request data for loading a dockset:

```json
{ "filename": "Streaming Layout" }
```

## Stored Data

Docksets are stored in the OBS plugin configuration directory:

```text
config/obs-studio/plugin_config/dynamic-docksets/
```

Each dockset is stored as JSON. Metadata, ordering, the active dockset,
collection mappings, shortcut files, and trigger files are also stored there.

## Version History

### 1.1.0

- Added generated Stream Deck shortcut files, one `.vbs` file per dockset.
- Added generated `_Cycle Next.vbs` and `_Cycle Prev.vbs` shortcuts.
- Added a Manager dock button to open the shortcuts folder.
- Added a file watcher based trigger folder for silent shortcut activation.

### 1.0.0

- Initial release.
- Added Manager dock, Quick Access dock, and Tools menu integration.
- Added save, load, cycle, rename, delete, and reorder actions.
- Added thumbnail previews.
- Added per-collection auto-load.
- Added optional auto-save.
- Added cycle hotkeys.
- Added obs-websocket vendor support.

## License

Dynamic Docksets is licensed under GPL-2.0-or-later.

## Disclaimer

Dynamic Docksets is an unofficial third-party plugin and is not affiliated with
or endorsed by the OBS Project.

AI-assisted tools were used during development and release preparation. The
maintainer is responsible for reviewing, testing, and publishing the released
plugin.
