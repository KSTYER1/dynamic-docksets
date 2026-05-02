# OBS Forum Submission Draft: Dynamic Docksets

## Resource Title

Dynamic Docksets

## Version

1.1.0

## Category

OBS Studio Plugins

## Tags

docks, dock layout, docksets, stream deck, workflow, layout manager, hotkeys,
websocket

## Short Tagline

Save, restore, cycle, and automate OBS dock layouts.

## Supported Bit Versions

64-bit

## Supported Platforms

Windows

## Minimum OBS Studio Version

30.0.0

## Source Code URL

TODO: add public GitHub repository URL

## Download URL

TODO: add GitHub Release URL or upload the ZIP directly

## Overview

Dynamic Docksets is an unofficial third-party dock layout manager plugin for
OBS Studio. It lets you save your current OBS dock layout as a named "dockset",
restore it later, cycle between saved layouts, and automate layout switching
from hotkeys, Stream Deck buttons, or obs-websocket vendor requests.

It is useful when you regularly switch between different OBS workspaces, such
as streaming, editing, scene setup, monitoring, or fullscreen preview layouts.

## Features

- Manager dock with saved docksets, thumbnail previews, and actions for save,
  save as, load, rename, reorder, and delete.
- Quick Access dock with one compact button per saved dockset.
- Tools menu integration for saving, loading, and cycling layouts.
- OBS hotkeys for cycling to the next or previous layout.
- Optional auto-save for the active dockset.
- Optional auto-load when switching scene collections.
- Stream Deck friendly shortcut files. The plugin generates one `.vbs` file per
  dockset, plus cycle-next and cycle-previous shortcuts.
- Optional obs-websocket vendor requests for automation tools such as
  Streamer.bot.

## Stream Deck Shortcuts

Version 1.1.0 adds generated shortcut files for Stream Deck. You do not need to
configure WebSocket just to load a layout from a button.

Open the shortcuts folder from the Manager dock, then assign the desired `.vbs`
file to a Stream Deck `System: Open` action. Pressing the Stream Deck button
loads that dockset inside OBS.

The plugin also generates `_Cycle Next.vbs` and `_Cycle Prev.vbs`.

## Installation

Download the Windows x64 release archive and extract or copy its contents into
your OBS Studio installation directory.

The final layout should include:

```text
obs-plugins/64bit/dynamic-docksets.dll
data/obs-plugins/dynamic-docksets/locale/en-US.ini
data/obs-plugins/dynamic-docksets/locale/de-DE.ini
```

The release archive also includes `INSTALL.bat`, which can copy the plugin into
a selected OBS directory.

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
5. Double-click the saved dockset, use the Quick Access dock, assign OBS
   hotkeys, or run a generated Stream Deck shortcut to restore the layout.

## WebSocket Automation

Dynamic Docksets registers the vendor name `DynamicDocksets`. It also registers
`jrDockie` as a compatibility alias when available.

Supported requests:

- `LoadDockset`
- `CycleDockset`
- `SaveDockset`

Example request data:

```json
{ "filename": "Streaming Layout" }
```

## Support / Bugs

Please report issues in the resource discussion thread or in the GitHub issue
tracker once the repository is published.

## License

GPL-2.0-or-later.

## Disclaimer

Dynamic Docksets is an unofficial third-party plugin and is not affiliated with
or endorsed by the OBS Project.

AI-assisted tools were used during development and release preparation. The
maintainer is responsible for reviewing, testing, and publishing the released
plugin.

## Pre-Submit Checklist

- [ ] Public GitHub repository exists.
- [ ] README is visible on GitHub.
- [ ] GPL license is visible on GitHub.
- [ ] Source Code URL field points to the repository.
- [ ] Release ZIP is attached to GitHub Releases or uploaded to the forum.
- [ ] At least one screenshot/GIF is added to the resource description.
- [ ] Description is in English.
- [ ] No OBS logo is used as resource icon or marketing artwork.
