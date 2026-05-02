/*
 * Quick-Access dock — a horizontal toolbar of buttons, one per saved dockset.
 * Clicking a button loads that dockset.
 */

#include "docksets-quick-dock.hpp"
#include "docksets-core.hpp"

#include <obs-module.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QStyle>
#include <QApplication>

DocksetsQuickDock::DocksetsQuickDock(QWidget *parent) : QFrame(parent)
{
	setFrameShape(QFrame::NoFrame);
	m_layout = new QHBoxLayout(this);
	m_layout->setContentsMargins(6, 4, 6, 4);
	m_layout->setSpacing(4);

	connect(DocksetCore::instance(), &DocksetCore::docksetsChanged,
		this, &DocksetsQuickDock::rebuild);
	connect(DocksetCore::instance(), &DocksetCore::activeDocksetChanged,
		this, &DocksetsQuickDock::on_active_changed);

	rebuild();
}

DocksetsQuickDock::~DocksetsQuickDock() = default;

static void clear_layout(QLayout *lay)
{
	if (!lay)
		return;
	while (QLayoutItem *it = lay->takeAt(0)) {
		if (QWidget *w = it->widget())
			w->deleteLater();
		delete it;
	}
}

void DocksetsQuickDock::rebuild()
{
	clear_layout(m_layout);

	QStringList names = DocksetCore::instance()->ordered_names();
	if (names.isEmpty()) {
		auto *lbl = new QLabel(QString::fromUtf8(obs_module_text("QuickEmpty")));
		lbl->setStyleSheet(QStringLiteral("color: #888; font-size: 9pt;"));
		m_layout->addWidget(lbl);
		m_layout->addStretch(1);
		return;
	}

	const QString active = DocksetCore::instance()->active_dockset();
	for (const QString &n : names) {
		auto *btn = new QPushButton(n);
		btn->setToolTip(n);
		if (n == active) {
			btn->setStyleSheet(QStringLiteral(
				"QPushButton { background-color: #f0c040; color: #000; "
				"font-weight: 600; padding: 3px 10px; border-radius: 3px; }"));
		} else {
			btn->setStyleSheet(QStringLiteral(
				"QPushButton { padding: 3px 10px; border-radius: 3px; }"));
		}
		connect(btn, &QPushButton::clicked, this, [n]() {
			DocksetCore::instance()->load(n);
		});
		m_layout->addWidget(btn);
	}

	auto *cycle = new QPushButton();
	cycle->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
	cycle->setToolTip(QString::fromUtf8(obs_module_text("QuickCycleTip")));
	cycle->setMaximumWidth(32);
	connect(cycle, &QPushButton::clicked, []() {
		DocksetCore::instance()->cycle_next();
	});
	m_layout->addWidget(cycle);
	m_layout->addStretch(1);
}

void DocksetsQuickDock::on_active_changed(const QString &)
{
	rebuild();
}
