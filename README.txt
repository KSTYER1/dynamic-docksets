Dynamic Docksets - OBS Dock Plugin v1.1.0
=========================================

Dynamic Docksets saves, loads, and cycles complete OBS dock layouts.

Highlights in 1.1.0
-------------------

  * Stream Deck friendly shortcut generation with one `.vbs` file per dockset
  * Shortcut folder button in the manager dock
  * Trigger-folder watching for external layout switching


Package Contents
----------------

  dynamic-docksets-1.1.0\
    INSTALL.bat
    README.txt
    obs-plugins\
      64bit\
        dynamic-docksets.dll
        dynamic-docksets.pdb
    data\
      obs-plugins\
        dynamic-docksets\
          locale\
            en-US.ini
            de-DE.ini


Installation
------------

Recommended: double-click `INSTALL.bat`, choose your OBS folder, and let the
script copy the files for you.

Manual installation: copy BOTH top-level folders, `obs-plugins\` and `data\`,
into your OBS Studio installation directory.


Usage
-----

Open the docks from:

  View -> Docks -> Dynamic Docksets - Manager
  View -> Docks -> Dynamic Docksets - Quick Access

Use the manager dock to save, rename, reorder, and load layouts. Use the
quick-access dock or hotkeys to switch faster.


Stream Deck Shortcuts
---------------------

Shortcut files are generated under:

  <OBS>\config\obs-studio\plugin_config\dynamic-docksets\shortcuts\

Each shortcut writes a trigger file that the plugin watches, which lets
external launchers switch layouts without OBS WebSocket setup.


Requirements
------------

  - OBS Studio 30.x / 31.x / 32.x (Windows x64)
  - Qt 6 (provided by OBS)
  - Optional: obs-websocket for vendor request integrations


License
-------

GPLv2 or later.
