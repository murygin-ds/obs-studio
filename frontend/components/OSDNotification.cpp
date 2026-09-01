#include "OSDNotification.hpp"

#include <utility/platform.hpp>

#include <QCursor>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>

#include "moc_OSDNotification.cpp"

namespace {
constexpr int screenMargin = 40;
constexpr int cornerRadius = 12;
constexpr int dotSize = 16;
constexpr int paddingX = 28;
constexpr int paddingY = 20;
constexpr int borderWidth = 2;

Qt::WindowFlags OverlayFlags()
{
	Qt::WindowFlags flags = Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
				Qt::WindowDoesNotAcceptFocus;
#ifndef _WIN32
	/* Click-through and translucency both need a layered window on Windows,
	 * which does not reliably show up on top of fullscreen games, so the
	 * Windows popup stays a plain opaque window. */
	flags |= Qt::WindowTransparentForInput;
#endif
	return flags;
}

bool UseTranslucency()
{
#ifdef _WIN32
	return false;
#else
	return true;
#endif
}
} // namespace

OSDNotification::OSDNotification(QWidget *parent)
	: QWidget(parent, OverlayFlags()),
	  label(new QLabel(this)),
	  translucent(UseTranslucency())
{
	if (translucent) {
		setAttribute(Qt::WA_TranslucentBackground);
	}
	setAttribute(Qt::WA_ShowWithoutActivating);

	QFont font = label->font();
	font.setPointSize(20);
	font.setBold(true);
	label->setFont(font);
	label->setStyleSheet("color: white; background: transparent;");

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(paddingX + dotSize + 14, paddingY, paddingX, paddingY);
	layout->addWidget(label);

	hideTimer.setSingleShot(true);
	connect(&hideTimer, &QTimer::timeout, this, &QWidget::hide);
}

void OSDNotification::Show(const QString &text, const QColor &accentColor, int durationMs)
{
	accent = accentColor;
	label->setText(text);
	adjustSize();

	QRect screen = TargetScreenGeometry();
	move(screen.right() - width() - screenMargin, screen.top() + screenMargin);

	show();
	raise();
	SetOverlayWindowBehavior(this);
	update();

	hideTimer.start(durationMs);
}

void OSDNotification::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	const QColor background(24, 24, 28, translucent ? 235 : 255);
	QRectF box = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

	if (translucent) {
		QPainterPath shape;
		shape.addRoundedRect(box, cornerRadius, cornerRadius);
		painter.fillPath(shape, background);
	} else {
		painter.fillRect(rect(), background);
		painter.setPen(QPen(accent, borderWidth));
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(rect().adjusted(borderWidth / 2, borderWidth / 2, -borderWidth, -borderWidth));
	}

	painter.setPen(Qt::NoPen);
	painter.setBrush(accent);
	painter.drawEllipse(QRectF(paddingX, box.center().y() - dotSize / 2.0, dotSize, dotSize));
}

QRect OSDNotification::TargetScreenGeometry() const
{
	/* The pointer usually sits on the screen the user is looking at, e.g.
	 * the one with the game while OBS runs in the background. */
	QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
	if (!screen) {
		screen = QGuiApplication::primaryScreen();
	}

	return screen ? screen->availableGeometry() : QRect(0, 0, 1280, 720);
}
