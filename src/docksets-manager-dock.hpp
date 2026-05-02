#pragma once

#include <QFrame>
#include <QString>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QCheckBox;
class QLabel;

class DocksetsManagerDock : public QFrame {
	Q_OBJECT

public:
	explicit DocksetsManagerDock(QWidget *parent = nullptr);
	~DocksetsManagerDock() override;

private slots:
	void refresh_list();
	void on_save_clicked();
	void on_save_as_clicked();
	void on_load_clicked();
	void on_delete_clicked();
	void on_item_double_clicked(QListWidgetItem *item);
	void on_context_menu(const QPoint &pos);
	void on_auto_save_toggled(bool on);
	void on_auto_load_toggled(bool on);
	void on_active_changed(const QString &name);
	void on_open_shortcuts_clicked();

private:
	void build_ui();

	QListWidget *m_list = nullptr;
	QPushButton *m_btn_save = nullptr;
	QPushButton *m_btn_save_as = nullptr;
	QPushButton *m_btn_load = nullptr;
	QPushButton *m_btn_delete = nullptr;
	QPushButton *m_btn_refresh = nullptr;
	QPushButton *m_btn_shortcuts = nullptr;
	QCheckBox *m_cb_auto_save = nullptr;
	QCheckBox *m_cb_auto_load = nullptr;
	QLabel *m_status = nullptr;
};
