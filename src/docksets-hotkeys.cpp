/*
 * Dynamic Docksets — global hotkeys for cycle next/prev.
 * Per-dockset hotkeys are registered dynamically when a user assigns one
 * via the manager dock; that's done from docksets-manager-dock.cpp.
 */

#include "docksets-hotkeys.hpp"
#include "docksets-core.hpp"

#include <obs-module.h>
#include <plugin-support.h>

static obs_hotkey_id g_hk_next = OBS_INVALID_HOTKEY_ID;
static obs_hotkey_id g_hk_prev = OBS_INVALID_HOTKEY_ID;

static void hk_next_cb(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(key);
	if (!pressed)
		return;
	DocksetCore::instance()->cycle_next();
}

static void hk_prev_cb(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(key);
	if (!pressed)
		return;
	DocksetCore::instance()->cycle_prev();
}

void docksets_hotkeys_register()
{
	g_hk_next = obs_hotkey_register_frontend("dynamic_docksets.cycle_next",
		obs_module_text("HotkeyCycleNext"), hk_next_cb, nullptr);
	g_hk_prev = obs_hotkey_register_frontend("dynamic_docksets.cycle_prev",
		obs_module_text("HotkeyCyclePrev"), hk_prev_cb, nullptr);
}

void docksets_hotkeys_unregister()
{
	if (g_hk_next != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(g_hk_next);
	if (g_hk_prev != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(g_hk_prev);
	g_hk_next = OBS_INVALID_HOTKEY_ID;
	g_hk_prev = OBS_INVALID_HOTKEY_ID;
}
