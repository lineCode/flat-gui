/**
MIT License

Copyright (c) 2020 Michael Scopchanov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "SSplitView.h"
#include "SSplitView_p.h"
#include <QVariantAnimation>

/*!
	\class SSplitView
	\inmodule FlatGui
	\brief Provides a horizontally splittable widget.
 */

/*!
	Constructs a SSplitView instance with a given \a parent.
 */

SSplitView::SSplitView(QWidget *parent) :
	QWidget(parent),
	m_ptr(new SSplitViewPrivate(this))
{

}

SSplitView::~SSplitView()
{
	delete m_ptr;
}

/*!
	\property SSplitView::baseWidget
	\brief The widget which is constantly shown.

	This property's default is \c nullptr.
 */

QWidget *SSplitView::baseWidget() const
{
	return m_ptr->baseWidget;
}

void SSplitView::setBaseWidget(QWidget *widget)
{
	if (!widget || m_ptr->inProgress || m_ptr->baseWidget == widget)
		return;

	widget->setParent(this);
	m_ptr->baseWidget = widget;

	emit baseWidgetChanged();
}

/*!
	\property SSplitView::sideWidget
	\brief The widget which is toggled.

	This property's default is \c nullptr.
 */

QWidget *SSplitView::sideWidget() const
{
	return m_ptr->sideWidget;
}

void SSplitView::setSideWidget(QWidget *widget)
{
	if (!widget || m_ptr->inProgress || m_ptr->sideWidget == widget)
		return;

	widget->setParent(this);
	m_ptr->sideWidget = widget;

	emit sideWidgetChanged();
}

/*!
	\property SSplitView::splitSide
	\brief The side on which the side widget is placed.

	This property's default is SSplitView::ST_Left.
 */

SSplitView::SideType SSplitView::splitSide() const
{
	return static_cast<SideType>(m_ptr->splitSide);
}

void SSplitView::setSplitSide(SSplitView::SideType side)
{
	if (m_ptr->inProgress || m_ptr->splitSide == side)
		return;

	m_ptr->splitSide = side;

	emit splitSideChanged();
}

/*!
	\property SSplitView::splitDuration
	\brief The duration of the side widget's toggle transition.

	This property's default is 250.
 */

int SSplitView::splitDuration() const
{
	return m_ptr->splitDuration;
}

void SSplitView::setSplitDuration(int duration)
{
	if (m_ptr->inProgress || m_ptr->splitDuration == duration)
		return;

	m_ptr->splitDuration = duration;

	emit splitDurationChanged();
}

/*!
	\property SSplitView::sideWidgetState
	\brief The state of the side widget.

	This property's default is SSplitView::ST_Closed.
 */

SSplitView::StateType SSplitView::sideWidgetState() const
{
	return static_cast<StateType>(m_ptr->sideWidgetState);
}

void SSplitView::openSideWidget()
{
	m_ptr->setSideWidgetState(ST_Open);

	m_ptr->layoutWidgets(m_ptr->splitSide == ST_Left
						 ? 0 : width() - m_ptr->sideWidget->width());
}

void SSplitView::closeSideWidget()
{
	m_ptr->setSideWidgetState(ST_Closed);

	m_ptr->layoutWidgets(m_ptr->splitSide == ST_Left
						 ? -m_ptr->sideWidget->width() : width());
}

/*!
	Toggles the side widget with a horizontal split transition.
 */

void SSplitView::toggleSideWidget()
{
	m_ptr->startTransition();
}

/*!
	\reimp
 */

void SSplitView::resizeEvent(QResizeEvent *)
{
	m_ptr->resizeWidgets();
}

SSplitViewPrivate::SSplitViewPrivate(SSplitView *parent) :
	p_ptr(parent),
	baseWidget(nullptr),
	sideWidget(nullptr),
	splitSide(SSplitView::ST_Left),
	splitDuration(250),
	sideWidgetState(SSplitView::ST_Closed),
	inProgress(false)
{

}

void SSplitViewPrivate::startTransition()
{
	if (inProgress || !sideWidget)
		return;

	auto *animation = new QVariantAnimation(p_ptr);
	bool isLeft = splitSide == SSplitView::ST_Left;
	bool isClosed = sideWidgetState == SSplitView::ST_Closed;

	inProgress = true;

	animation->setStartValue(isLeft
							 ? (isClosed ? -sideWidget->width() : 0)
							 : (isClosed ? 0 : sideWidget->width()));
	animation->setEndValue(isLeft
						   ? (isClosed ? 0 : -sideWidget->width())
						   : (isClosed ? sideWidget->width() : 0));
	animation->setDuration(splitDuration);
	animation->setEasingCurve(QEasingCurve::InOutQuad);

	QObject::connect(animation, &QVariantAnimation::valueChanged,
					 [this](const QVariant &value){
		layoutWidgets(value.toInt());
	});

	QObject::connect(animation, &QVariantAnimation::finished,
					 [this](){
		setSideWidgetState(sideWidgetState == SSplitView::ST_Closed
						   ? SSplitView::ST_Open : SSplitView::ST_Closed);

		inProgress = false;
	});

	animation->start(QVariantAnimation::DeleteWhenStopped);
}

void SSplitViewPrivate::setSideWidgetState(int value)
{
	if (sideWidgetState == value)
		return;

	sideWidgetState = value;

	emit p_ptr->sideWidgetStateChanged();
}

void SSplitViewPrivate::resizeWidgets()
{
	if (!sideWidget)
		return;

	bool isClosed = sideWidgetState == SSplitView::ST_Closed;

	layoutWidgets(splitSide == SSplitView::ST_Left
				  ? (isClosed ? -sideWidget->width() : 0)
				  : (isClosed ? 0 : sideWidget->width()));
}

void SSplitViewPrivate::layoutWidgets(int x)
{
	if (!baseWidget || !sideWidget)
		return;

	bool isLeft = splitSide == SSplitView::ST_Left;
	int w = p_ptr->width() - x;
	int h = p_ptr->height();

	x = isLeft ? x : w;

	baseWidget->setGeometry(isLeft ? x + sideWidget->width() : 0, 0,
							isLeft ? w - sideWidget->width() : w, h);
	sideWidget->setGeometry(x, 0, sideWidget->width(), h);
}
