#include "mytreeview.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

MyTreeView::MyTreeView(QWidget *parent) : QTreeView(parent)
{
}

void MyTreeView::mousePressEvent(QMouseEvent *event){
	if(event->button() == Qt::MiddleButton){
		emit treeMiddleClicked(event->pos());
	}
	else return QTreeView::mousePressEvent(event);
}

void MyTreeView::keyPressEvent(QKeyEvent *event){
	if(event->key() == Qt::Key_Escape){
		emit hideNavBar();
	}
	return QTreeView::keyPressEvent(event);
}
