# Dynamic Docksets

Dynamic Docksets is a third-party native OBS Studio dock for saving, loading,
 and cycling complete OBS dock layouts.

It is designed for creators who switch between different workspace setups such
as streaming, editing, moderation, and preview layouts.

## Features

- Manager dock for saving, loading, renaming, deleting, and reordering dock
  layouts.
- Quick-access dock with one button per saved layout.
- Thumbnail previews for saved docksets.
- Optional auto-save for the currently active dockset.
- Optional auto-load rules per OBS scene collection.
- Hotkeys for next and previous dockset.
- WebSocket vendor requests for `LoadDockset`, `CycleDockset`, and
  `SaveDockset`.
- Stream Deck friendly `.vbs` shortcut generation for each saved dockset.
- Trigger-folder watcher for one-click layout switching from external tools.

## Requirements

- OBS Studio 30.x, 31.x, or 32.x
- Windows x64 for the packaged release
- Qt 6, provided by OBS Studio
- Optional: `obs-websocket` for vendor request integrations

## Installation

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
View -> Docks -> Dynamic Docksets - Manager
View -> Docks -> Dynamic Docksets - Quick Access
```

## Basic Usage

1. Open `View -> Docks -> Dynamic Docksets - Manager`.
2. Arrange your OBS docks the way you want.
3. Click save and name the layout.
4. Double-click a saved dockset to restore it later.
5. Use the quick-access dock or hotkeys to switch layouts faster.

Dynamic Docksets stores one JSON file per layout in the OBS plugin config
folder, plus metadata for order, active selection, and scene-collection rules.

## Stream Deck Shortcuts

Dynamic Docksets can generate one `.vbs` shortcut per saved dockset, plus
cycle shortcuts such as `_Cycle Next.vbs` and `_Cycle Prev.vbs`.

These files are stored under:

```text
<OBS>\config\obs-studio\plugin_config\dynamic-docksets\shortcuts\
```

You can assign those files directly to Stream Deck or any launcher that can
open files. Internally, the shortcut writes a trigger file and the plugin
loads the matching dockset automatically.

## Version History

### 1.1.0

- Added Stream Deck shortcut file generation.
- Added a toolbar button to open the shortcuts folder.
- Added trigger-folder watching for external layout switching.
- Added `_Cycle Next.vbs` and `_Cycle Prev.vbs` helper shortcuts.

### 1.0.0

- Initial release.
- Added manager and quick-access docks.
- Added save, load, cycle, rename, delete, and reorder actions.
- Added thumbnail previews.
- Added per-collection auto-load rules.
- Added optional auto-save.
- Added hotkeys for cycling docksets.
- Added WebSocket vendor support with `jrDockie` compatibility alias.

## License

Dynamic Docksets is licensed under GPL-2.0-or-later.

## Disclaimer

Dynamic Docksets is an unofficial third-party plugin and is not affiliated with
or endorsed by the OBS Project.
