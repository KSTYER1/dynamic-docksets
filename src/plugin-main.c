/*
Dynamic Docksets — save/load/cycle OBS dock layouts (docksets).
Copyright (C) 2026 K_STYER, GPLv2 or later.
*/

#include <obs-module.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
	return "Save, load and cycle OBS dock layouts with thumbnails, hotkeys, per-scene-collection auto-load, auto-save, and a quick-access dock.";
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return "Dynamic Docks";
}

MODULE_EXPORT const char *obs_module_author(void)
{
	return "K_STYER";
}

extern void docksets_register_all(void);
extern void docksets_unregister_all(void);

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

MODULE_EXPORT void obs_module_post_load(void)
{
	docksets_register_all();
}

void obs_module_unload(void)
{
	/* Log BEFORE unregister so we always see this line even if a later
	 * step hangs; helps diagnose shutdown issues. */
	obs_log(LOG_INFO, "plugin unloading...");
	docksets_unregister_all();
	obs_log(LOG_INFO, "plugin unloaded");
}
