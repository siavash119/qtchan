#include "overlay.h"
#include <QPainter>

Overlay::Overlay(QWidget *parent):
	QWidget(parent), parent(parent), color(QColor(0,0,0, 120))
{
	this->setObjectName("overlay");
	rectToFill = parent->rect();
	this->setGeometry(rectToFill);
	displayText = "Posting...";
}

void Overlay::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(rectToFill, color);
	painter.setPen({200, 200, 255});
	painter.setFont({"arial,helvetica", 48});
	painter.drawText(rectToFill, displayText, Qt::AlignHCenter | Qt::AlignVCenter);
	QWidget::paintEvent(event);
}
