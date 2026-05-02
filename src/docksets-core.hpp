#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QHash>
#include <QPixmap>
#include <QDateTime>

class QTimer;

struct DocksetInfo {
	QString name;
	QDateTime created_at;
	QDateTime modified_at;
	QString scene_collection; /* leer = keine Auto-Load-Bindung */
	QString hotkey_label;     /* Anzeige-String, leer wenn nicht gesetzt */
	QPixmap thumbnail;
	bool has_thumbnail = false;
};

class DocksetCore : public QObject {
	Q_OBJECT

public:
	static DocksetCore *instance();

	/* CRUD */
	bool save_current_as(const QString &name, bool capture_thumbnail = true);
	bool load(const QString &name);
	bool rename_dockset(const QString &from, const QString &to);
	bool remove(const QString &name);
	bool exists(const QString &name) const;

	/* Listing */
	QVector<DocksetInfo> list_all();
	QStringList ordered_names() const;
	void set_order(const QStringList &order);
	void move_up(const QString &name);
	void move_down(const QString &name);

	/* Active tracking */
	QString active_dockset() const;
	void cycle_next();
	void cycle_prev();

	/* Per-collection mapping */
	void set_collection_mapping(const QString &collection, const QString &dockset);
	QString get_dockset_for_collection(const QString &collection) const;
	QHash<QString, QString> all_collection_mappings() const;
	bool collection_auto_load_enabled() const;
	void set_collection_auto_load_enabled(bool);

	/* Auto-save */
	bool auto_save_enabled() const;
	void set_auto_save_enabled(bool);
	void on_layout_changed_throttled(); /* called by Qt signal hook */

	/* Thumbnail */
	QPixmap capture_main_window_thumbnail(const QSize &target = QSize(200, 150));

	/* Storage paths */
	QString config_dir() const;

signals:
	void docksetsChanged();
	void activeDocksetChanged(const QString &name);

private:
	explicit DocksetCore(QObject *parent = nullptr);
	~DocksetCore() override;

	void load_meta();
	void save_meta() const;
	QString safe_filename(const QString &name) const;
	QString file_for(const QString &name) const;

	/* Helpers for QMainWindow access */
	class QMainWindow *main_window() const;

	/* State */
	QString m_active;
	QStringList m_order;
	QHash<QString, QString> m_collection_to_dockset;
	bool m_auto_save_enabled = false;
	bool m_collection_auto_load = false;

	QTimer *m_auto_save_timer = nullptr;
};
