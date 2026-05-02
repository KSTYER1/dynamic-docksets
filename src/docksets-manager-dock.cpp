/*
 * Manager dock — list of saved docksets with thumbnails + actions.
 */

#include "docksets-manager-dock.hpp"
#include "docksets-core.hpp"
#include "docksets-shortcuts.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <util/bmem.h>

#include <QDesktopServices>
#include <QUrl>
#include <QEvent>
#include <QChildEvent>
#include <QDockWidget>
#include <QMainWindow>
#include <QStyle>
#include <QApplication>
#include <QTimer>
#include <QPointer>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QPixmap>
#include <QIcon>
#include <QSize>
#include <QStringList>
#include <QMainWindow>

DocksetsManagerDock::DocksetsManagerDock(QWidget *parent) : QFrame(parent)
{
	setFrameShape(QFrame::NoFrame);
	build_ui();

	connect(DocksetCore::instance(), &DocksetCore::docksetsChanged,
		this, &DocksetsManagerDock::refresh_list);
	connect(DocksetCore::instance(), &DocksetCore::activeDocksetChanged,
		this, &DocksetsManagerDock::on_active_changed);

	m_cb_auto_save->setChecked(DocksetCore::instance()->auto_save_enabled());
	m_cb_auto_load->setChecked(DocksetCore::instance()->collection_auto_load_enabled());

	refresh_list();
}

DocksetsManagerDock::~DocksetsManagerDock() = default;

void DocksetsManagerDock::build_ui()
{
	auto *outer = new QVBoxLayout(this);
	outer->setContentsMargins(8, 8, 8, 8);
	outer->setSpacing(6);

	auto *toolbar = new QHBoxLayout();
	m_btn_save = new QPushButton(QString::fromUtf8(obs_module_text("BtnSaveCurrent")));
	m_btn_save->setToolTip(QString::fromUtf8(obs_module_text("BtnSaveCurrentTip")));
	m_btn_save_as = new QPushButton(QString::fromUtf8(obs_module_text("BtnSaveAs")));
	m_btn_save_as->setToolTip(QString::fromUtf8(obs_module_text("BtnSaveAsTip")));
	m_btn_load = new QPushButton(QString::fromUtf8(obs_module_text("BtnLoad")));
	m_btn_delete = new QPushButton(QString::fromUtf8(obs_module_text("BtnDelete")));
	/* Use Qt-Standard-Icons (system-themed) instead of Unicode emojis —
	 * Windows' default UI font doesn't render color emojis reliably so
	 * they show up as empty boxes. Standard icons always render. */
	QStyle *style = QApplication::style();
	m_btn_refresh = new QPushButton();
	m_btn_refresh->setIcon(style->standardIcon(QStyle::SP_BrowserReload));
	m_btn_refresh->setToolTip(QString::fromUtf8(obs_module_text("BtnRefresh")));
	m_btn_refresh->setMaximumWidth(34);
	m_btn_shortcuts = new QPushButton();
	m_btn_shortcuts->setIcon(style->standardIcon(QStyle::SP_DirOpenIcon));
	m_btn_shortcuts->setToolTip(QString::fromUtf8(obs_module_text("BtnShortcuts")));
	m_btn_shortcuts->setMaximumWidth(34);

	toolbar->addWidget(m_btn_save);
	toolbar->addWidget(m_btn_save_as);
	toolbar->addStretch(1);
	toolbar->addWidget(m_btn_load);
	toolbar->addWidget(m_btn_delete);
	toolbar->addWidget(m_btn_shortcuts);
	toolbar->addWidget(m_btn_refresh);
	outer->addLayout(toolbar);

	auto *checks = new QHBoxLayout();
	m_cb_auto_save = new QCheckBox(QString::fromUtf8(obs_module_text("AutoSaveLabel")));
	m_cb_auto_save->setToolTip(QString::fromUtf8(obs_module_text("AutoSaveTip")));
	m_cb_auto_load = new QCheckBox(QString::fromUtf8(obs_module_text("AutoLoadLabel")));
	m_cb_auto_load->setToolTip(QString::fromUtf8(obs_module_text("AutoLoadTip")));
	checks->addWidget(m_cb_auto_save);
	checks->addWidget(m_cb_auto_load);
	checks->addStretch(1);
	outer->addLayout(checks);

	m_list = new QListWidget();
	m_list->setIconSize(QSize(80, 60));
	m_list->setSpacing(2);
	m_list->setContextMenuPolicy(Qt::CustomContextMenu);
	outer->addWidget(m_list, 1);

	m_status = new QLabel();
	m_status->setStyleSheet(QStringLiteral("color: #888; font-size: 9pt;"));
	outer->addWidget(m_status);

	connect(m_btn_save, &QPushButton::clicked, this, &DocksetsManagerDock::on_save_clicked);
	connect(m_btn_save_as, &QPushButton::clicked, this, &DocksetsManagerDock::on_save_as_clicked);
	connect(m_btn_load, &QPushButton::clicked, this, &DocksetsManagerDock::on_load_clicked);
	connect(m_btn_delete, &QPushButton::clicked, this, &DocksetsManagerDock::on_delete_clicked);
	connect(m_btn_refresh, &QPushButton::clicked, this, &DocksetsManagerDock::refresh_list);
	connect(m_btn_shortcuts, &QPushButton::clicked,
		this, &DocksetsManagerDock::on_open_shortcuts_clicked);
	connect(m_list, &QListWidget::itemDoubleClicked,
		this, &DocksetsManagerDock::on_item_double_clicked);
	connect(m_list, &QListWidget::customContextMenuRequested,
		this, &DocksetsManagerDock::on_context_menu);
	connect(m_cb_auto_save, &QCheckBox::toggled,
		this, &DocksetsManagerDock::on_auto_save_toggled);
	connect(m_cb_auto_load, &QCheckBox::toggled,
		this, &DocksetsManagerDock::on_auto_load_toggled);
}

void DocksetsManagerDock::refresh_list()
{
	QString prev = m_list->currentItem() ? m_list->currentItem()->data(Qt::UserRole).toString()
					     : QString();
	m_list->clear();
	const QString active = DocksetCore::instance()->active_dockset();
	auto infos = DocksetCore::instance()->list_all();
	for (const auto &di : infos) {
		QString label = di.name;
		if (di.name == active)
			label = QString::fromUtf8("\xe2\x98\x85 ") + label; /* ★ */
		QStringList meta;
		if (!di.scene_collection.isEmpty())
			meta << (QString::fromUtf8("\xf0\x9f\x94\x97 ") + di.scene_collection); /* 🔗 */
		if (di.modified_at.isValid())
			meta << di.modified_at.toString(QStringLiteral("dd.MM. HH:mm"));
		QString tooltip = QString::fromUtf8(obs_module_text("ItemTooltip"))
					  .replace(QStringLiteral("%1"), di.name);

		auto *item = new QListWidgetItem();
		item->setText(label + (meta.isEmpty() ? QString() : QStringLiteral("\n") + meta.join(QStringLiteral("  \xe2\x80\xa2  "))));
		item->setToolTip(tooltip);
		item->setData(Qt::UserRole, di.name);
		if (di.has_thumbnail) {
			item->setIcon(QIcon(di.thumbnail));
		} else {
			item->setIcon(QIcon());
		}
		m_list->addItem(item);
		if (di.name == prev)
			m_list->setCurrentItem(item);
	}

	int n = infos.size();
	if (n == 0)
		m_status->setText(QString::fromUtf8(obs_module_text("StatusEmpty")));
	else
		m_status->setText(QString::fromUtf8(obs_module_text("StatusCount"))
					  .replace(QStringLiteral("%1"), QString::number(n)));
}

void DocksetsManagerDock::on_active_changed(const QString &)
{
	refresh_list();
}

void DocksetsManagerDock::on_save_clicked()
{
	QString active = DocksetCore::instance()->active_dockset();
	if (active.isEmpty()) {
		on_save_as_clicked();
		return;
	}
	if (DocksetCore::instance()->save_current_as(active, true))
		m_status->setText(QString::fromUtf8(obs_module_text("StatusSaved"))
					  .replace(QStringLiteral("%1"), active));
}

void DocksetsManagerDock::on_save_as_clicked()
{
	auto *mw = reinterpret_cast<QMainWindow *>(obs_frontend_get_main_window());
	bool ok = false;
	QString name = QInputDialog::getText(mw,
		QString::fromUtf8(obs_module_text("BtnSaveAs")),
		QString::fromUtf8(obs_module_text("PromptSaveName")),
		QLineEdit::Normal, QString(), &ok);
	if (!ok || name.trimmed().isEmpty())
		return;
	if (DocksetCore::instance()->exists(name.trimmed())) {
		auto rc = QMessageBox::question(this,
			QString::fromUtf8(obs_module_text("ConfirmOverwriteTitle")),
			QString::fromUtf8(obs_module_text("ConfirmOverwriteText"))
				.replace(QStringLiteral("%1"), name.trimmed()));
		if (rc != QMessageBox::Yes)
			return;
	}
	DocksetCore::instance()->save_current_as(name.trimmed(), true);
}

void DocksetsManagerDock::on_load_clicked()
{
	auto *it = m_list->currentItem();
	if (!it)
		return;
	QString name = it->data(Qt::UserRole).toString();
	DocksetCore::instance()->load(name);
}

void DocksetsManagerDock::on_delete_clicked()
{
	auto *it = m_list->currentItem();
	if (!it)
		return;
	QString name = it->data(Qt::UserRole).toString();
	auto rc = QMessageBox::question(this,
		QString::fromUtf8(obs_module_text("ConfirmDeleteTitle")),
		QString::fromUtf8(obs_module_text("ConfirmDeleteText"))
			.replace(QStringLiteral("%1"), name));
	if (rc != QMessageBox::Yes)
		return;
	DocksetCore::instance()->remove(name);
}

void DocksetsManagerDock::on_item_double_clicked(QListWidgetItem *item)
{
	if (!item)
		return;
	QString name = item->data(Qt::UserRole).toString();
	DocksetCore::instance()->load(name);
}

void DocksetsManagerDock::on_context_menu(const QPoint &pos)
{
	auto *it = m_list->itemAt(pos);
	if (!it)
		return;
	QString name = it->data(Qt::UserRole).toString();

	QMenu menu(this);
	QAction *act_load = menu.addAction(QString::fromUtf8(obs_module_text("CtxLoad")));
	QAction *act_rename = menu.addAction(QString::fromUtf8(obs_module_text("CtxRename")));

	menu.addSeparator();
	QMenu *coll_menu = menu.addMenu(QString::fromUtf8(obs_module_text("CtxAssignCollection")));
	struct char_list {
		char **arr;
		~char_list() { bfree(arr); }
	};
	char **collections = obs_frontend_get_scene_collections();
	if (collections) {
		for (size_t i = 0; collections[i]; i++) {
			QString c = QString::fromUtf8(collections[i]);
			QAction *a = coll_menu->addAction(c);
			connect(a, &QAction::triggered, this, [c, name]() {
				DocksetCore::instance()->set_collection_mapping(c, name);
			});
		}
		bfree(collections);
	}
	QAction *act_clear_coll = coll_menu->addAction(
		QString::fromUtf8(obs_module_text("CtxAssignCollectionNone")));

	menu.addSeparator();
	QAction *act_up = menu.addAction(QString::fromUtf8(obs_module_text("CtxMoveUp")));
	QAction *act_down = menu.addAction(QString::fromUtf8(obs_module_text("CtxMoveDown")));
	menu.addSeparator();
	QAction *act_delete = menu.addAction(QString::fromUtf8(obs_module_text("CtxDelete")));

	connect(act_load, &QAction::triggered, this, [name]() {
		DocksetCore::instance()->load(name);
	});
	connect(act_rename, &QAction::triggered, this, [this, name]() {
		bool ok = false;
		QString to = QInputDialog::getText(this,
			QString::fromUtf8(obs_module_text("CtxRename")),
			QString::fromUtf8(obs_module_text("PromptRename")),
			QLineEdit::Normal, name, &ok);
		if (!ok || to.trimmed().isEmpty() || to == name)
			return;
		DocksetCore::instance()->rename_dockset(name, to.trimmed());
	});
	connect(act_clear_coll, &QAction::triggered, this, [name]() {
		QHash<QString, QString> map = DocksetCore::instance()->all_collection_mappings();
		for (auto it2 = map.constBegin(); it2 != map.constEnd(); ++it2) {
			if (it2.value() == name)
				DocksetCore::instance()->set_collection_mapping(it2.key(), QString());
		}
	});
	connect(act_up, &QAction::triggered, this, [name]() {
		DocksetCore::instance()->move_up(name);
	});
	connect(act_down, &QAction::triggered, this, [name]() {
		DocksetCore::instance()->move_down(name);
	});
	connect(act_delete, &QAction::triggered, this, [this, name]() {
		auto rc = QMessageBox::question(this,
			QString::fromUtf8(obs_module_text("ConfirmDeleteTitle")),
			QString::fromUtf8(obs_module_text("ConfirmDeleteText"))
				.replace(QStringLiteral("%1"), name));
		if (rc == QMessageBox::Yes)
			DocksetCore::instance()->remove(name);
	});

	menu.exec(m_list->mapToGlobal(pos));
}

void DocksetsManagerDock::on_auto_save_toggled(bool on)
{
	DocksetCore::instance()->set_auto_save_enabled(on);
}

void DocksetsManagerDock::on_auto_load_toggled(bool on)
{
	DocksetCore::instance()->set_collection_auto_load_enabled(on);
}

void DocksetsManagerDock::on_open_shortcuts_clicked()
{
	QString dir = ShortcutsManager::instance()->shortcuts_dir();
	if (dir.isEmpty())
		return;
	QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

/* ---------------------------------------------------------------------------
 * Module-level registration: invoked from plugin-main.c (extern "C" bridge).
 * Wires up docks, menu, hotkeys, websocket, and frontend events.
 * --------------------------------------------------------------------------- */

#include "docksets-quick-dock.hpp"
#include "docksets-menu.hpp"
#include "docksets-hotkeys.hpp"
#include "docksets-websocket.hpp"

#include <QObject>

static DocksetsManagerDock *g_manager_dock = nullptr;
static DocksetsQuickDock *g_quick_dock = nullptr;
static QMetaObject::Connection g_dock_loc_conn;

/* Event filter that triggers throttled auto-save whenever any dock widget
 * inside the main window is moved, resized, shown or hidden. Also installs
 * itself onto newly added dock widgets via the main window's ChildAdded
 * event. */
class AutoSaveFilter : public QObject {
public:
	explicit AutoSaveFilter(QMainWindow *mw) : QObject(mw), m_mw(mw)
	{
		mw->installEventFilter(this);
		for (QDockWidget *d : mw->findChildren<QDockWidget *>())
			d->installEventFilter(this);
	}

protected:
	bool eventFilter(QObject *obj, QEvent *ev) override
	{
		if (obj == m_mw && ev->type() == QEvent::ChildAdded) {
			/* New dock widget added to the main window -> hook it. */
			auto *ce = static_cast<QChildEvent *>(ev);
			if (auto *d = qobject_cast<QDockWidget *>(ce->child()))
				d->installEventFilter(this);
			return false;
		}
		if (qobject_cast<QDockWidget *>(obj)) {
			QEvent::Type t = ev->type();
			if (t == QEvent::Move || t == QEvent::Resize
			    || t == QEvent::Show || t == QEvent::Hide
			    || t == QEvent::ParentChange) {
				DocksetCore::instance()->on_layout_changed_throttled();
			}
		}
		return false;
	}

private:
	QMainWindow *m_mw;
};

static AutoSaveFilter *g_autosave_filter = nullptr;

/* OBS fires SCENE_COLLECTION_CHANGED at startup as it loads the initial
 * collection. At that moment, OBS hasn't yet applied its own saved
 * window state (maximize/geometry). If we restore a dockset's layout
 * right then, OBS's later max-state apply gets overridden visually.
 * We gate auto-load on FINISHED_LOADING so it only triggers on actual
 * user-initiated collection switches. */
static bool g_obs_finished_loading = false;

/* Captured at register-time so we can restore it at FINISHED_LOADING. Adding
 * two docks during post_load can interfere with OBS's window-state restore
 * and end up de-maximizing the window. We snapshot before, restore after. */
static Qt::WindowStates g_saved_window_states = Qt::WindowNoState;
static QByteArray g_saved_window_geometry;
static bool g_window_state_captured = false;

static void on_frontend_event(enum obs_frontend_event event, void *)
{
	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		g_obs_finished_loading = true;
		/* Restore the window state we saw before we added our docks.
		 * Use a 0-delay timer so this runs after the current event-loop
		 * iteration finishes any pending layout work. */
		if (g_window_state_captured) {
			auto *mw = reinterpret_cast<QMainWindow *>(obs_frontend_get_main_window());
			if (mw) {
				QPointer<QMainWindow> mw_ptr(mw);
				Qt::WindowStates st = g_saved_window_states;
				QByteArray geom = g_saved_window_geometry;
				QTimer::singleShot(0, mw, [mw_ptr, st, geom]() {
					if (!mw_ptr)
						return;
					if (!geom.isEmpty())
						mw_ptr->restoreGeometry(geom);
					mw_ptr->setWindowState(st);
				});
			}
		}
		return;
	}
	if (event == OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED) {
		if (!g_obs_finished_loading)
			return; /* skip the initial startup-time collection load */
		const char *name = obs_frontend_get_current_scene_collection();
		if (!name)
			return;
		QString collection = QString::fromUtf8(name);
		bfree((void *)name);
		if (!DocksetCore::instance()->collection_auto_load_enabled())
			return;
		QString target = DocksetCore::instance()->get_dockset_for_collection(collection);
		if (!target.isEmpty()) {
			QMetaObject::invokeMethod(DocksetCore::instance(), [target]() {
				DocksetCore::instance()->load(target);
			}, Qt::QueuedConnection);
		}
	}
}

extern "C" void docksets_register_all(void)
{
	/* Snapshot the current window state BEFORE we add anything. Adding
	 * docks to QMainWindow can cause Qt to re-layout and lose the
	 * maximize state. We restore it on FINISHED_LOADING. */
	if (auto *mw = reinterpret_cast<QMainWindow *>(obs_frontend_get_main_window())) {
		g_saved_window_states = mw->windowState();
		g_saved_window_geometry = mw->saveGeometry();
		g_window_state_captured = true;
		obs_log(LOG_INFO, "captured window state: max=%d full=%d",
			(g_saved_window_states & Qt::WindowMaximized) ? 1 : 0,
			(g_saved_window_states & Qt::WindowFullScreen) ? 1 : 0);
	}

	/* Initialize shortcut watcher BEFORE the dock so first-time bootstrap of
	 * the shortcuts/triggers folders happens cleanly. */
	ShortcutsManager::instance()->initialize();

	/* Create the manager dock and register it via OBS frontend. */
	g_manager_dock = new DocksetsManagerDock();
	obs_frontend_add_dock_by_id("dynamic-docksets.manager",
		obs_module_text("ManagerDockTitle"), g_manager_dock);

	g_quick_dock = new DocksetsQuickDock();
	obs_frontend_add_dock_by_id("dynamic-docksets.quick",
		obs_module_text("QuickDockTitle"), g_quick_dock);

	DocksetsMenu::instance()->install();

	docksets_hotkeys_register();
	docksets_websocket_try_register();

	obs_frontend_add_event_callback(on_frontend_event, nullptr);

	/* Install AutoSaveFilter on the main window. It watches all current
	 * dock widgets for move/resize/show/hide events AND auto-hooks new
	 * dock widgets that get added later (via ChildAdded). Drag&Drop
	 * reorganisation IS now caught — much more robust than the previous
	 * tabifiedDockWidgetActivated-only approach. */
	auto *mw = reinterpret_cast<QMainWindow *>(obs_frontend_get_main_window());
	if (mw && !g_autosave_filter) {
		g_autosave_filter = new AutoSaveFilter(mw);
	}

	obs_log(LOG_INFO, "dynamic-docksets: docks + menu + hotkeys registered");
}

extern "C" void docksets_unregister_all(void)
{
	obs_frontend_remove_event_callback(on_frontend_event, nullptr);
	docksets_websocket_unregister();
	docksets_hotkeys_unregister();
	DocksetsMenu::instance()->uninstall();
	ShortcutsManager::instance()->shutdown();
	/* Don't manually delete the event filter: it's parented to the
	 * QMainWindow which during shutdown is itself being torn down. Just
	 * clear our pointer; Qt's parent-child cleanup handles it. */
	g_autosave_filter = nullptr;
	if (g_dock_loc_conn)
		QObject::disconnect(g_dock_loc_conn);
	/* OBS owns the dock widgets after add_dock_by_id; it deletes them on shutdown. */
	g_manager_dock = nullptr;
	g_quick_dock = nullptr;
}
