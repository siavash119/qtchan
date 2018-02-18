#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>

class TreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit TreeView(QWidget *parent = nullptr);
	QModelIndexList selected();
public slots:
	void selectTab(QModelIndex ind);
protected:
	void mousePressEvent(QMouseEvent *);
	void keyPressEvent(QKeyEvent *);
signals:
	void treeMiddleClicked(QModelIndex);
	void hideNavBar();

public slots:
};

#endif // MYTREEVIEW_H
