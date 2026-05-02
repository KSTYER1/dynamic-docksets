/*
 * Dynamic Docksets — core save/load logic.
 * Stores each dockset as a JSON file under <obs-config>/plugin_config/dynamic-docksets/.
 */

#include "docksets-core.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>
#include <util/bmem.h>

#include <QMainWindow>
#include <QDockWidget>
#include <QByteArray>
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPixmap>
#include <QImage>
#include <QTimer>
#include <QPointer>

static DocksetCore *g_instance = nullptr;

DocksetCore *DocksetCore::instance()
{
	if (!g_instance)
		g_instance = new DocksetCore();
	return g_instance;
}

DocksetCore::DocksetCore(QObject *parent) : QObject(parent)
{
	m_auto_save_timer = new QTimer(this);
	m_auto_save_timer->setSingleShot(true);
	m_auto_save_timer->setInterval(5000); /* 5 s throttle */
	connect(m_auto_save_timer, &QTimer::timeout, this, [this]() {
		if (!m_auto_save_enabled || m_active.isEmpty())
			return;
		save_current_as(m_active, /*capture_thumb=*/false);
	});
	load_meta();
}

DocksetCore::~DocksetCore() = default;

QString DocksetCore::config_dir() const
{
	char *cfg = obs_module_get_config_path(obs_current_module(), "");
	QString path = QString::fromUtf8(cfg ? cfg : "");
	bfree(cfg);
	if (!path.isEmpty()) {
		QDir().mkpath(path);
	}
	return path;
}

QString DocksetCore::safe_filename(const QString &name) const
{
	QString s = name;
	for (QChar &c : s) {
		if (!c.isLetterOrNumber() && c != '_' && c != '-' && c != ' ')
			c = '_';
	}
	s = s.trimmed();
	if (s.isEmpty())
		s = QStringLiteral("dockset");
	return s + QStringLiteral(".json");
}

QString DocksetCore::file_for(const QString &name) const
{
	return QDir(config_dir()).filePath(safe_filename(name));
}

QMainWindow *DocksetCore::main_window() const
{
	return reinterpret_cast<QMainWindow *>(obs_frontend_get_main_window());
}

void DocksetCore::load_meta()
{
	QString path = QDir(config_dir()).filePath(QStringLiteral("_meta.json"));
	QFile f(path);
	if (!f.exists())
		return;
	if (!f.open(QIODevice::ReadOnly))
		return;
	QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
	f.close();
	QJsonObject o = doc.object();
	m_active = o.value(QStringLiteral("active_dockset")).toString();
	m_auto_save_enabled = o.value(QStringLiteral("auto_save_enabled")).toBool(false);
	m_collection_auto_load = o.value(QStringLiteral("collection_auto_load")).toBool(false);

	m_order.clear();
	for (auto v : o.value(QStringLiteral("order")).toArray())
		m_order.append(v.toString());

	m_collection_to_dockset.clear();
	QJsonObject mappings = o.value(QStringLiteral("collection_to_dockset")).toObject();
	for (auto it = mappings.begin(); it != mappings.end(); ++it)
		m_collection_to_dockset.insert(it.key(), it.value().toString());
}

void DocksetCore::save_meta() const
{
	QJsonObject o;
	o.insert(QStringLiteral("active_dockset"), m_active);
	o.insert(QStringLiteral("auto_save_enabled"), m_auto_save_enabled);
	o.insert(QStringLiteral("collection_auto_load"), m_collection_auto_load);
	QJsonArray ord;
	for (const QString &n : m_order)
		ord.append(n);
	o.insert(QStringLiteral("order"), ord);
	QJsonObject mappings;
	for (auto it = m_collection_to_dockset.constBegin(); it != m_collection_to_dockset.constEnd(); ++it)
		mappings.insert(it.key(), it.value());
	o.insert(QStringLiteral("collection_to_dockset"), mappings);

	QString path = QDir(config_dir()).filePath(QStringLiteral("_meta.json"));
	QFile f(path);
	if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		f.write(QJsonDocument(o).toJson(QJsonDocument::Indented));
		f.close();
	}
}

QPixmap DocksetCore::capture_main_window_thumbnail(const QSize &target)
{
	QMainWindow *mw = main_window();
	if (!mw)
		return QPixmap();
	QPixmap full = mw->grab();
	if (full.isNull())
		return QPixmap();
	return full.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

bool DocksetCore::save_current_as(const QString &name, bool capture_thumbnail)
{
	if (name.trimmed().isEmpty())
		return false;
	QMainWindow *mw = main_window();
	if (!mw)
		return false;

	QByteArray state = mw->saveState();
	QByteArray geometry = mw->saveGeometry();

	QJsonObject o;
	o.insert(QStringLiteral("name"), name);
	qint64 now_ms = QDateTime::currentMSecsSinceEpoch();
	o.insert(QStringLiteral("created_at"), now_ms); /* preserved on overwrite below */
	o.insert(QStringLiteral("modified_at"), now_ms);
	o.insert(QStringLiteral("qt_state"), QString::fromLatin1(state.toBase64()));
	o.insert(QStringLiteral("qt_geometry"), QString::fromLatin1(geometry.toBase64()));

	QJsonArray docks;
	for (QDockWidget *d : mw->findChildren<QDockWidget *>()) {
		if (!d->isHidden())
			docks.append(d->objectName());
	}
	o.insert(QStringLiteral("visible_docks"), docks);

	if (m_collection_to_dockset.values().contains(name)) {
		/* Find the collection that maps to this name */
		for (auto it = m_collection_to_dockset.constBegin(); it != m_collection_to_dockset.constEnd(); ++it) {
			if (it.value() == name) {
				o.insert(QStringLiteral("scene_collection"), it.key());
				break;
			}
		}
	}

	if (capture_thumbnail) {
		QPixmap thumb = capture_main_window_thumbnail();
		if (!thumb.isNull()) {
			QByteArray png;
			QBuffer buf(&png);
			buf.open(QIODevice::WriteOnly);
			thumb.save(&buf, "PNG");
			o.insert(QStringLiteral("thumbnail_png"), QString::fromLatin1(png.toBase64()));
		}
	}

	QString path = file_for(name);

	/* Preserve created_at if file already exists. */
	QFile existing(path);
	if (existing.exists() && existing.open(QIODevice::ReadOnly)) {
		QJsonObject prev = QJsonDocument::fromJson(existing.readAll()).object();
		existing.close();
		if (prev.contains(QStringLiteral("created_at")))
			o.insert(QStringLiteral("created_at"), prev.value(QStringLiteral("created_at")));
		if (!capture_thumbnail && prev.contains(QStringLiteral("thumbnail_png")))
			o.insert(QStringLiteral("thumbnail_png"), prev.value(QStringLiteral("thumbnail_png")));
	}

	QFile f(path);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return false;
	f.write(QJsonDocument(o).toJson(QJsonDocument::Indented));
	f.close();

	if (!m_order.contains(name))
		m_order.append(name);
	m_active = name;
	save_meta();
	emit docksetsChanged();
	emit activeDocksetChanged(m_active);
	return true;
}

bool DocksetCore::load(const QString &name)
{
	QMainWindow *mw = main_window();
	if (!mw)
		return false;
	QFile f(file_for(name));
	if (!f.open(QIODevice::ReadOnly))
		return false;
	QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
	f.close();
	if (o.isEmpty())
		return false;

	QByteArray state = QByteArray::fromBase64(o.value(QStringLiteral("qt_state")).toString().toLatin1());
	/* Geometry (window position/size/maximized state) is stored in the
	 * dockset for export/portability, but NOT applied on load. Restoring
	 * geometry would un-maximize OBS at startup when a collection-change
	 * event triggers auto-load. The user expects window state to be
	 * managed by OBS itself; docksets are about DOCK LAYOUT only. */
	if (!state.isEmpty())
		mw->restoreState(state);

	m_active = name;
	save_meta();
	emit activeDocksetChanged(m_active);
	return true;
}

bool DocksetCore::rename_dockset(const QString &from, const QString &to)
{
	if (from == to || to.trimmed().isEmpty())
		return false;
	if (!exists(from) || exists(to))
		return false;
	if (!QFile::rename(file_for(from), file_for(to)))
		return false;
	int idx = m_order.indexOf(from);
	if (idx >= 0)
		m_order[idx] = to;
	if (m_active == from)
		m_active = to;
	for (auto it = m_collection_to_dockset.begin(); it != m_collection_to_dockset.end(); ++it) {
		if (it.value() == from)
			it.value() = to;
	}
	save_meta();
	emit docksetsChanged();
	if (m_active == to)
		emit activeDocksetChanged(m_active);
	return true;
}

bool DocksetCore::remove(const QString &name)
{
	if (!exists(name))
		return false;
	QFile::remove(file_for(name));
	m_order.removeAll(name);
	if (m_active == name)
		m_active.clear();
	for (auto it = m_collection_to_dockset.begin(); it != m_collection_to_dockset.end();) {
		if (it.value() == name)
			it = m_collection_to_dockset.erase(it);
		else
			++it;
	}
	save_meta();
	emit docksetsChanged();
	emit activeDocksetChanged(m_active);
	return true;
}

bool DocksetCore::exists(const QString &name) const
{
	return QFile::exists(file_for(name));
}

QVector<DocksetInfo> DocksetCore::list_all()
{
	QVector<DocksetInfo> out;
	QStringList names = ordered_names();
	for (const QString &n : names) {
		QFile f(file_for(n));
		if (!f.open(QIODevice::ReadOnly))
			continue;
		QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
		f.close();
		DocksetInfo di;
		di.name = o.value(QStringLiteral("name")).toString(n);
		di.created_at = QDateTime::fromMSecsSinceEpoch(
			o.value(QStringLiteral("created_at")).toVariant().toLongLong());
		di.modified_at = QDateTime::fromMSecsSinceEpoch(
			o.value(QStringLiteral("modified_at")).toVariant().toLongLong());
		for (auto it = m_collection_to_dockset.constBegin(); it != m_collection_to_dockset.constEnd(); ++it) {
			if (it.value() == di.name) {
				di.scene_collection = it.key();
				break;
			}
		}
		QString thumb_b64 = o.value(QStringLiteral("thumbnail_png")).toString();
		if (!thumb_b64.isEmpty()) {
			QByteArray png = QByteArray::fromBase64(thumb_b64.toLatin1());
			QPixmap pm;
			if (pm.loadFromData(png, "PNG")) {
				di.thumbnail = pm;
				di.has_thumbnail = true;
			}
		}
		out.append(di);
	}
	return out;
}

QStringList DocksetCore::ordered_names() const
{
	/* Self-heal: also include any orphan files on disk that aren't in m_order yet. */
	QStringList names = m_order;
	QDir d(config_dir());
	for (const QString &fn : d.entryList(QStringList() << QStringLiteral("*.json"),
					     QDir::Files)) {
		if (fn == QStringLiteral("_meta.json"))
			continue;
		QFile f(d.filePath(fn));
		if (!f.open(QIODevice::ReadOnly))
			continue;
		QJsonObject o = QJsonDocument::fromJson(f.readAll()).object();
		f.close();
		QString n = o.value(QStringLiteral("name")).toString();
		if (!n.isEmpty() && !names.contains(n))
			names.append(n);
	}
	return names;
}

void DocksetCore::set_order(const QStringList &order)
{
	m_order = order;
	save_meta();
	emit docksetsChanged();
}

void DocksetCore::move_up(const QString &name)
{
	int idx = m_order.indexOf(name);
	if (idx > 0) {
		m_order.swapItemsAt(idx, idx - 1);
		save_meta();
		emit docksetsChanged();
	}
}

void DocksetCore::move_down(const QString &name)
{
	int idx = m_order.indexOf(name);
	if (idx >= 0 && idx + 1 < m_order.size()) {
		m_order.swapItemsAt(idx, idx + 1);
		save_meta();
		emit docksetsChanged();
	}
}

QString DocksetCore::active_dockset() const
{
	return m_active;
}

void DocksetCore::cycle_next()
{
	QStringList names = ordered_names();
	if (names.isEmpty())
		return;
	int idx = names.indexOf(m_active);
	int next = (idx + 1) % names.size();
	load(names[next]);
}

void DocksetCore::cycle_prev()
{
	QStringList names = ordered_names();
	if (names.isEmpty())
		return;
	int idx = names.indexOf(m_active);
	int prev = (idx <= 0 ? names.size() - 1 : idx - 1);
	load(names[prev]);
}

void DocksetCore::set_collection_mapping(const QString &collection, const QString &dockset)
{
	if (dockset.isEmpty())
		m_collection_to_dockset.remove(collection);
	else
		m_collection_to_dockset.insert(collection, dockset);
	save_meta();
	emit docksetsChanged();
}

QString DocksetCore::get_dockset_for_collection(const QString &collection) const
{
	return m_collection_to_dockset.value(collection);
}

QHash<QString, QString> DocksetCore::all_collection_mappings() const
{
	return m_collection_to_dockset;
}

bool DocksetCore::collection_auto_load_enabled() const
{
	return m_collection_auto_load;
}

void DocksetCore::set_collection_auto_load_enabled(bool on)
{
	m_collection_auto_load = on;
	save_meta();
}

bool DocksetCore::auto_save_enabled() const
{
	return m_auto_save_enabled;
}

void DocksetCore::set_auto_save_enabled(bool on)
{
	m_auto_save_enabled = on;
	save_meta();
}

void DocksetCore::on_layout_changed_throttled()
{
	if (!m_auto_save_enabled || m_active.isEmpty())
		return;
	if (m_auto_save_timer)
		m_auto_save_timer->start(); /* restarts, debounce */
}
