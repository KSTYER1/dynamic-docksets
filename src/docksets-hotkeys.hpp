#pragma once

#include <obs.h>

void docksets_hotkeys_register();
void docksets_hotkeys_unregister();

/* Save and load hotkey bindings via OBS hotkey config (called from
 * obs_module_save / obs_module_load). For now we let OBS persist them
 * automatically through its frontend hotkey machinery. */
