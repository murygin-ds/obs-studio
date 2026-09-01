#pragma once

#include <QColor>
#include <QTimer>
#include <QWidget>

class QLabel;

/* Frameless always-on-top popup that briefly shows a short message in the
 * corner of the screen, meant to be visible over other applications. */
class OSDNotification : public QWidget {
	Q_OBJECT

public:
	explicit OSDNotification(QWidget *parent = nullptr);

	void Show(const QString &text, const QColor &accentColor, int durationMs = 2500);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	QLabel *label;
	QTimer hideTimer;
	QColor accent;

	QRect TargetScreenGeometry() const;
};
