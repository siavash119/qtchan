#include "treeitem.h"
#include <QStringList>
#include <QDebug>

TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent)
	: parent(parent), itemData(data)
{
}

TreeItem::TreeItem(const QList<QVariant> &data,
				   TreeItem *parent, QWidget *tab, TreeItemType type)
	: parent(parent), tab(tab), type(type), itemData(data)
{
}

TreeItem::~TreeItem()
{
	if(tab != Q_NULLPTR){
		tab->disconnect();
		tab->deleteLater();
	}
	qDeleteAll(children);
	emit deleting(this);
}

void TreeItem::appendChild(TreeItem *item)
{
	children.append(item);
}

void TreeItem::removeChild(int row)
{
	children.removeAt(row);
}

void TreeItem::removeChildren(){
	qDeleteAll(children);
}

TreeItem *TreeItem::child(int row) const
{
	return children.value(row);
}

int TreeItem::childCount() const
{
	return children.count();
}

int TreeItem::columnCount() const
{
	return itemData.count();
}

QVariant TreeItem::data(int column) const
{
	return itemData.value(column);
}

int TreeItem::row() const
{
	if (parent)
		return parent->children.indexOf(const_cast<TreeItem*>(this));

	return 0;
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
	(void)columns;
	if (position < 0 || position > children.size())
		return false;

	for (int row = 0; row < count; ++row) {
		QList<QVariant> newData;
		TreeItem *child = new TreeItem(newData, this);
		children.insert(position, child);
		child->parent = this;
	}

	return true;
}

void TreeItem::insertChild(int pos, TreeItem *child)
{
	children.insert(pos, child);
	child->parent = this;
}

bool TreeItem::setData(int column, const QVariant &value)
{
	if (column < 0 || column >= itemData.size())
		return false;

	itemData[column] = value;
	if(column == 0) display = value.toString();
	return true;
}
