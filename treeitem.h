#ifndef TREEITEM_H
#define TREEITEM_H

#include <QList>
#include <QVariant>
#include <QWidget>

enum TreeItemType {board, thread};

class TreeItem
{
public:
	explicit TreeItem(const QList<QVariant> &data, TreeItem *parent);
	explicit TreeItem(const QList<QVariant> &data, TreeItem *parent, QWidget *tab, TreeItemType type = thread);
	~TreeItem();

	void appendChild(TreeItem *child);
	void removeChild(int row);

	TreeItem *child(int row) const;
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	void insertChild(int pos, TreeItem *child);
	bool insertChildren(int position, int count, int columns);
	bool setData(int column, const QVariant &value);

	TreeItem *parent;
	QWidget *tab;
	TreeItemType type;
	QString query;
	QString display;
	int unseen;

private:
	QList<QVariant> itemData;
	QList<TreeItem*> children;
};

#endif // TREEITEM_H
