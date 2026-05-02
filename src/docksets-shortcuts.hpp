#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class DocksetCore;
class QFileSystemWatcher;

/* Manages per-dockset shortcut files (.vbs) that Stream Deck (or any
 * file-launcher) can run to load a dockset. Each .vbs creates a .trigger
 * file in the watched triggers/ directory. The plugin reacts to that and
 * loads the corresponding dockset. */
class ShortcutsManager : public QObject {
	Q_OBJECT

public:
	static ShortcutsManager *instance();

	void initialize();
	void shutdown();

	QString shortcuts_dir() const;
	QString triggers_dir() const;

	/* Re-emit the .vbs files for current docksets (handles add/rename/delete). */
	void rebuild_shortcuts();

private slots:
	void on_triggers_changed(const QString &path);

private:
	explicit ShortcutsManager(QObject *parent = nullptr);
	~ShortcutsManager() override;

	void process_pending_triggers();
	void write_dockset_shortcut(const QString &dockset_name);
	void write_cycle_shortcuts();
	QString safe_filename(const QString &name) const;

	QFileSystemWatcher *m_watcher = nullptr;
	bool m_initialized = false;
};
