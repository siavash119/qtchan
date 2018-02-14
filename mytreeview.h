#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>

class MyTreeView : public QTreeView
{
	Q_OBJECT
public:
	explicit MyTreeView(QWidget *parent = nullptr);
	QModelIndexList selected();
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
