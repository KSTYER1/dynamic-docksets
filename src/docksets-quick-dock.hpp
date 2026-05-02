#pragma once

#include <QFrame>

class QHBoxLayout;
class QPushButton;

class DocksetsQuickDock : public QFrame {
	Q_OBJECT

public:
	explicit DocksetsQuickDock(QWidget *parent = nullptr);
	~DocksetsQuickDock() override;

private slots:
	void rebuild();
	void on_active_changed(const QString &name);

private:
	QHBoxLayout *m_layout = nullptr;
};
