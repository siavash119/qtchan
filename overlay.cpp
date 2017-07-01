#include "overlay.h"
#include <QPainter>

Overlay::Overlay(QWidget *parent):
	color(QColor(0,0,0, 120))
{
	rect = parent->rect();
	displayText = "Posting...";
	//rect.setWidth(rect.width()  *0.05f);
}

void Overlay::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	//painter.setPen(Qt::NoPen);
	painter.fillRect(rect, color);
	painter.setPen({200, 200, 255});
	painter.setFont({"arial,helvetica", 48});
	painter.drawText(rect, displayText, Qt::AlignHCenter | Qt::AlignVCenter);
}

