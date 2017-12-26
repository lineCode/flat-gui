#include "Alert.h"
#include "Alert_p.h"
#include "ToolBar.h"
#include "PushButton.h"
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QLabel>
#include <QStyle>

/*!
 * \class Alert
 * \inmodule FlatGui
 */

Alert::Alert(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint),
	m_ptr(new AlertPrivate(this))
{
	auto *layoutMain = new QVBoxLayout(this);
	auto *layoutBody = new QVBoxLayout();
	auto *layoutTitle = new QHBoxLayout();
	auto *effect = new QGraphicsDropShadowEffect(this);
	auto *body = new QWidget(this);

	m_ptr->labTitle->setAlignment(Qt::AlignCenter);
	m_ptr->labTitle->setMinimumHeight(48);
	m_ptr->labTitle->setStyleSheet(".QLabel {"
								   "	border: none;"
								   "	border-bottom: 1px solid palette(midlight);"
								   "}");

	layoutTitle->addWidget(m_ptr->labTitle);
	layoutTitle->setContentsMargins(12, 0, 12, 0);
	layoutTitle->setSpacing(0);

	m_ptr->layoutMessage->setContentsMargins(12, 12, 12, 12);
	m_ptr->layoutMessage->setSpacing(0);

	m_ptr->toolBar->setBackgroundRole(QPalette::Window);
	m_ptr->toolBar->addStretch();

	layoutBody->addLayout(layoutTitle);
	layoutBody->addLayout(m_ptr->layoutMessage);
	layoutBody->addWidget(m_ptr->toolBar);
	layoutBody->setContentsMargins(0, 0, 0, 0);
	layoutBody->setSpacing(0);

	effect->setOffset(0);
	effect->setColor(palette().color(QPalette::Shadow));
	effect->setBlurRadius(5);

	body->setAutoFillBackground(true);
	body->setBackgroundRole(QPalette::Button);
	body->setGraphicsEffect(effect);
	body->setLayout(layoutBody);

	layoutMain->addWidget(body);
	layoutMain->setContentsMargins(10, 10, 10, 10);
	layoutMain->setSpacing(0);

	setAttribute(Qt::WA_TranslucentBackground);
}

Alert::~Alert()
{
	delete m_ptr;
}

void Alert::setTitle(const QString &title)
{
	m_ptr->labTitle->setText("<b>" + title + "</b>");
}

void Alert::setWidget(QWidget *widget)
{
	widget->setParent(this);
	m_ptr->layoutMessage->addWidget(widget);
}

void Alert::setButtons(const QStringList &buttonNames, int defaultButtonIndex)
{
	foreach (const QString &buttonName, buttonNames) {
		auto *button = new PushButton(this);
		int index = buttonNames.indexOf(buttonName);

		button->setText(buttonName);
		button->setDefault(index == defaultButtonIndex);
		button->setProperty("index", index);

		m_ptr->toolBar->addWidget(button);

		connect(button, &PushButton::clicked, this, &Alert::onButtonClicked);
	}

	m_ptr->defaultButtonIndex = defaultButtonIndex;
}

int Alert::showAlert(QWidget *parent, const QPixmap &icon, const QString &title, const QString &message, const QStringList &buttonNames, int defaultButtonIndex)
{
	Alert alert(parent);
	auto *widget = new QWidget();
	auto *layoutMessage = new QHBoxLayout(widget);
	auto *labIcon = new QLabel(widget);
	auto *labMessage = new QLabel(widget);

	labIcon->setPixmap(icon);
	labIcon->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	labIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

	labMessage->setText(message);
	labMessage->setForegroundRole(QPalette::Text);
	labMessage->setTextFormat(Qt::RichText);
	labMessage->setOpenExternalLinks(true);
	labMessage->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
	labMessage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	labMessage->setWordWrap(true);

	layoutMessage->addWidget(labIcon);
	layoutMessage->addWidget(labMessage);
	layoutMessage->setContentsMargins(0, 0, 0, 0);
	layoutMessage->setSpacing(20);

	alert.setTitle(title);
	alert.setWidget(widget);
	alert.setButtons(buttonNames, defaultButtonIndex);
	alert.adjustSize();

	return alert.exec();
}

void Alert::reject()
{
	done(m_ptr->defaultButtonIndex);
}

void Alert::showEvent(QShowEvent *event)
{
	if (!parentWidget()) {
		QDialog::showEvent(event);
		return;
	}

	const QRect parentRect(parentWidget()->mapToGlobal(QPoint(0, 0)), parentWidget()->size());
	auto *flyIn = new QPropertyAnimation(this, "pos");

	flyIn->setStartValue(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignHCenter | Qt::AlignBottom, size(), parentRect).topLeft());
	flyIn->setEndValue(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), parentRect).topLeft());
	flyIn->setDuration(500);
	flyIn->setEasingCurve(QEasingCurve::OutBack);
	flyIn->start(QPropertyAnimation::DeleteWhenStopped);
}

void Alert::onButtonClicked()
{
	done(sender()->property("index").toInt());
}

AlertPrivate::AlertPrivate(Alert *parent) :
	labTitle(new QLabel(parent)),
	layoutMessage(new QVBoxLayout()),
	toolBar(new ToolBar(parent)),
	defaultButtonIndex(0)
{

}