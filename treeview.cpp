#include "treeview.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

TreeView::TreeView(QWidget *parent) : QTreeView(parent)
{
}

void TreeView::mousePressEvent(QMouseEvent *event){
	if(event->button() == Qt::MiddleButton){
		QModelIndex index = indexAt(event->pos());
		if(index != rootIndex()){
			emit treeMiddleClicked(index);
		}
	}
	else return QTreeView::mousePressEvent(event);
}

void TreeView::keyPressEvent(QKeyEvent *event){
	if(event->key() == Qt::Key_Escape){
		emit hideNavBar();
	}
	return QTreeView::keyPressEvent(event);
}

QModelIndexList TreeView::selected(){
	QModelIndexList indexList = selectedIndexes();
	if(!indexList.size() && currentIndex().isValid()) {
		indexList.clear();
		indexList.append(currentIndex());
	}
	return indexList;
}

void TreeView::selectTab(QModelIndex ind){
	if(ind.isValid()){
		setCurrentIndex(ind);
		this->selectionModel()->setCurrentIndex(ind,QItemSelectionModel::ClearAndSelect);
	}
}
