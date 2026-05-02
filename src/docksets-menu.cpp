/*
 * Tools menu integration. Adds a "Dynamic Docksets" submenu under Tools/.
 */

#include "docksets-menu.hpp"
#include "docksets-core.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenuBar>

static DocksetsMenu *g_menu = nullptr;

DocksetsMenu *DocksetsMenu::instance()
{
	if (!g_menu)
		g_menu = new DocksetsMenu();
	return g_menu;
}

DocksetsMenu::DocksetsMenu(QObject *parent) : QObject(parent) {}

static QMenu *find_tools_menu()
{
	auto *mw = reinterpret_cast<QMainWindow *>(obs_frontend_get_main_window());
	if (!mw)
		return nullptr;
	QMenuBar *mb = mw->menuBar();
	if (!mb)
		return nullptr;
	for (QMenu *m : mb->findChildren<QMenu *>(QString(), Qt::FindDirectChildrenOnly)) {
		QString t = m->title();
		QString plain = t;
		plain.remove('&');
		if (plain.compare("Tools", Qt::CaseInsensitive) == 0
		    || plain.compare("Werkzeuge", Qt::CaseInsensitive) == 0
		    || plain.compare("Outils", Qt::CaseInsensitive) == 0)
			return m;
	}
	return nullptr;
}

void DocksetsMenu::install()
{
	if (m_root_menu)
		return;
	QMenu *tools = find_tools_menu();
	if (!tools)
		return;

	m_root_menu = tools->addMenu(QString::fromUtf8(obs_module_text("MenuTitle")));

	QAction *act_save = m_root_menu->addAction(QString::fromUtf8(obs_module_text("MenuSaveAs")));
	connect(act_save, &QAction::triggered, this, [this]() {
		auto *mw = reinterpret_cast<QMainWindow *>(obs_frontend_get_main_window());
		bool ok = false;
		QString name = QInputDialog::getText(mw,
			QString::fromUtf8(obs_module_text("MenuSaveAs")),
			QString::fromUtf8(obs_module_text("PromptSaveName")),
			QLineEdit::Normal, QString(), &ok);
		if (!ok || name.trimmed().isEmpty())
			return;
		DocksetCore::instance()->save_current_as(name.trimmed(), true);
	});

	m_load_submenu = m_root_menu->addMenu(QString::fromUtf8(obs_module_text("MenuLoadSubmenu")));
	rebuild_load_submenu();

	QAction *act_cycle = m_root_menu->addAction(QString::fromUtf8(obs_module_text("MenuCycleNext")));
	connect(act_cycle, &QAction::triggered, this, []() {
		DocksetCore::instance()->cycle_next();
	});

	connect(DocksetCore::instance(), &DocksetCore::docksetsChanged,
		this, &DocksetsMenu::rebuild_load_submenu);
}

void DocksetsMenu::uninstall()
{
	/* Don't manually delete: the menu is parented to the OBS Tools QMenu,
	 * which is parented to the QMenuBar, which is parented to the QMainWindow.
	 * During OBS shutdown the QMainWindow may already be in the middle of
	 * destroying its widget tree — a manual `delete` here can collide with
	 * that and either hang the unload or trigger a use-after-free.
	 *
	 * Just disconnect signals and forget the pointers. Qt's parent-child
	 * cleanup will free the menu when the QMenuBar goes away. */
	if (m_root_menu) {
		QObject::disconnect(DocksetCore::instance(), nullptr, this, nullptr);
		m_root_menu = nullptr;
		m_load_submenu = nullptr;
	}
}

void DocksetsMenu::rebuild_load_submenu()
{
	if (!m_load_submenu)
		return;
	m_load_submenu->clear();
	QStringList names = DocksetCore::instance()->ordered_names();
	if (names.isEmpty()) {
		QAction *a = m_load_submenu->addAction(
			QString::fromUtf8(obs_module_text("MenuNoDocksets")));
		a->setEnabled(false);
		return;
	}
	for (const QString &n : names) {
		QAction *a = m_load_submenu->addAction(n);
		connect(a, &QAction::triggered, this, [n]() {
			DocksetCore::instance()->load(n);
		});
	}
}
