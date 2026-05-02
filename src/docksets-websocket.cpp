/*
 * WebSocket vendor via OBS proc-handler API.
 *
 * Modern obs-websocket (v5+) does NOT export DLL symbols for the vendor
 * API. Instead it adds procs to OBS' main proc handler. Calls go through
 * obs_get_proc_handler() + proc_handler_call() with calldata args.
 *
 * Proc names (as registered by obs-websocket):
 *   obs_websocket_register_vendor(in string "name", out ptr "vendor")
 *   obs_websocket_vendor_register_request(in ptr "vendor", in string "type",
 *                                          in ptr "callback", in ptr "priv_data",
 *                                          out bool "success")
 *
 * If obs-websocket isn't loaded, proc_handler_call returns false and we
 * silently skip — no link-time dependency.
 */

#include "docksets-websocket.hpp"
#include "docksets-core.hpp"

#include <obs-module.h>
#include <obs-data.h>
#include <plugin-support.h>
#include <util/c99defs.h>
#include <callback/proc.h>
#include <callback/calldata.h>

#include <QString>
#include <QMetaObject>

typedef void *obs_websocket_vendor;
typedef void (*obs_websocket_request_callback_function)(obs_data_t *request_data,
						        obs_data_t *response_data,
						        void *priv_data);

static obs_websocket_vendor g_vendor_primary = nullptr;
static obs_websocket_vendor g_vendor_compat = nullptr;
static bool g_registered = false;

/* --- request handlers (called from obs-websocket worker threads) --- */

static void req_load_dockset(obs_data_t *req, obs_data_t *resp, void *)
{
	const char *fn = obs_data_get_string(req, "filename");
	if (!fn)
		fn = obs_data_get_string(req, "name");
	if (!fn || !*fn) {
		obs_data_set_string(resp, "error", "missing filename/name");
		return;
	}
	QString name = QString::fromUtf8(fn);
	bool ok = false;
	QMetaObject::invokeMethod(DocksetCore::instance(), [name, &ok]() {
		ok = DocksetCore::instance()->load(name);
	}, Qt::BlockingQueuedConnection);
	obs_data_set_bool(resp, "loaded", ok);
}

static void req_cycle_dockset(obs_data_t *, obs_data_t *resp, void *)
{
	QMetaObject::invokeMethod(DocksetCore::instance(), []() {
		DocksetCore::instance()->cycle_next();
	}, Qt::QueuedConnection);
	obs_data_set_bool(resp, "cycled", true);
}

static void req_save_dockset(obs_data_t *req, obs_data_t *resp, void *)
{
	const char *name = obs_data_get_string(req, "name");
	if (!name || !*name) {
		obs_data_set_string(resp, "error", "missing name");
		return;
	}
	QString qname = QString::fromUtf8(name);
	bool ok = false;
	QMetaObject::invokeMethod(DocksetCore::instance(), [qname, &ok]() {
		ok = DocksetCore::instance()->save_current_as(qname, true);
	}, Qt::BlockingQueuedConnection);
	obs_data_set_bool(resp, "saved", ok);
}

/* --- proc-handler wrappers --- */

static obs_websocket_vendor proc_register_vendor(const char *name)
{
	proc_handler_t *ph = obs_get_proc_handler();
	if (!ph)
		return nullptr;
	calldata_t cd;
	uint8_t stack[128];
	calldata_init_fixed(&cd, stack, sizeof(stack));
	calldata_set_string(&cd, "name", name);
	if (!proc_handler_call(ph, "obs_websocket_register_vendor", &cd))
		return nullptr;
	return (obs_websocket_vendor)calldata_ptr(&cd, "vendor");
}

static bool proc_register_request(obs_websocket_vendor v, const char *type,
				   obs_websocket_request_callback_function cb)
{
	if (!v)
		return false;
	proc_handler_t *ph = obs_get_proc_handler();
	if (!ph)
		return false;
	calldata_t cd;
	uint8_t stack[256];
	calldata_init_fixed(&cd, stack, sizeof(stack));
	calldata_set_ptr(&cd, "vendor", v);
	calldata_set_string(&cd, "type", type);
	calldata_set_ptr(&cd, "callback", (void *)cb);
	calldata_set_ptr(&cd, "priv_data", nullptr);
	if (!proc_handler_call(ph, "obs_websocket_vendor_register_request", &cd))
		return false;
	return calldata_bool(&cd, "success");
}

static void register_requests_on(obs_websocket_vendor v)
{
	if (!v)
		return;
	proc_register_request(v, "LoadDockset", req_load_dockset);
	proc_register_request(v, "CycleDockset", req_cycle_dockset);
	proc_register_request(v, "SaveDockset", req_save_dockset);
}

void docksets_websocket_try_register()
{
	if (g_registered)
		return;

	/* Try the proc-handler call. If obs-websocket isn't loaded, the proc
	 * isn't registered and proc_handler_call returns false. */
	g_vendor_primary = proc_register_vendor("DynamicDocksets");
	if (g_vendor_primary) {
		register_requests_on(g_vendor_primary);
		obs_log(LOG_INFO, "WebSocket vendor 'DynamicDocksets' registered");
		g_registered = true;
	} else {
		obs_log(LOG_INFO, "WebSocket vendor: obs-websocket not available, skipping");
		return;
	}

	/* Compat alias for users with existing jrDockie scripts. Only succeeds
	 * if jrdockie itself isn't loaded (vendor names are unique). */
	g_vendor_compat = proc_register_vendor("jrDockie");
	if (g_vendor_compat) {
		register_requests_on(g_vendor_compat);
		obs_log(LOG_INFO, "WebSocket vendor 'jrDockie' (compat alias) registered");
	}
}

void docksets_websocket_unregister()
{
	/* obs-websocket clears vendors on its own shutdown. We just forget
	 * the pointers so a subsequent load fresh-registers cleanly. */
	g_vendor_primary = nullptr;
	g_vendor_compat = nullptr;
	g_registered = false;
}
