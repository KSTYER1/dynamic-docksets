#pragma once

/* Runtime-detect obs-websocket and register a vendor with our requests.
 * If obs-websocket is not loaded, this is a silent no-op (logged once). */
void docksets_websocket_try_register();
void docksets_websocket_unregister();
