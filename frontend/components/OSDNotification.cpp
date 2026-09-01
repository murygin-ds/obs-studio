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
constexpr int screenMargin = 24;
constexpr int cornerRadius = 10;
constexpr int accentBarWidth = 6;
} // namespace

OSDNotification::OSDNotification(QWidget *parent)
	: QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
				  Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput),
	  label(new QLabel(this))
{
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_ShowWithoutActivating);

	QFont font = label->font();
	font.setPointSize(16);
	font.setBold(true);
	label->setFont(font);
	label->setStyleSheet("color: white; background: transparent;");

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(accentBarWidth + 18, 14, 22, 14);
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
#ifdef __APPLE__
	SetOverlayWindowBehavior(this);
#endif
	update();

	hideTimer.start(durationMs);
}

void OSDNotification::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QRectF box = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
	QPainterPath shape;
	shape.addRoundedRect(box, cornerRadius, cornerRadius);

	painter.fillPath(shape, QColor(24, 24, 28, 235));

	painter.setClipPath(shape);
	painter.fillRect(QRectF(box.left(), box.top(), accentBarWidth, box.height()), accent);
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
