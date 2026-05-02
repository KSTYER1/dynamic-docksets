#pragma once

#include <QObject>

class QMenu;
class QAction;

class DocksetsMenu : public QObject {
	Q_OBJECT

public:
	static DocksetsMenu *instance();
	void install();
	void uninstall();

private slots:
	void rebuild_load_submenu();

private:
	explicit DocksetsMenu(QObject *parent = nullptr);

	QMenu *m_root_menu = nullptr;
	QMenu *m_load_submenu = nullptr;
};
