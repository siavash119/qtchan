#include "mytreeview.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

MyTreeView::MyTreeView(QWidget *parent) : QTreeView(parent)
{
}

void MyTreeView::mousePressEvent(QMouseEvent *event){
	if(event->button() == Qt::MiddleButton){
		QModelIndex index = indexAt(event->pos());
		if(index != rootIndex()){
			emit treeMiddleClicked(index);
		}
	}
	else return QTreeView::mousePressEvent(event);
}

void MyTreeView::keyPressEvent(QKeyEvent *event){
	if(event->key() == Qt::Key_Escape){
		emit hideNavBar();
	}
	return QTreeView::keyPressEvent(event);
}

QModelIndexList MyTreeView::selected(){
	QModelIndexList indexList = selectedIndexes();
	if(!indexList.size() && currentIndex().isValid()) {
		indexList.clear();
		indexList.append(currentIndex());
	}
	return indexList;
}

void MyTreeView::selectTab(QModelIndex ind){
	if(ind.isValid())
		setCurrentIndex(ind);
}
