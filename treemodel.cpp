#include "treeitem.h"
#include "treemodel.h"

#include <QCoreApplication>
#include <QStringList>
#include <QMimeData>
#include <QIODevice>
#include <QTextStream>
#include <QDataStream>
#include <QPointer>

TreeModel::TreeModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;
	rootData << "root";
	root = new TreeItem(rootData, 0);
}

TreeModel::~TreeModel() {
	delete root;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
	TreeItem *parentItem = getItem(parent);
	bool success;

	beginInsertRows(parent, position, position + rows - 1);
	success = parentItem->insertChildren(position, rows, root->columnCount());
	endInsertRows();

	return success;
}

void TreeModel::addChild(QModelIndex &parent, TreeItem *child)
{
	TreeItem *parentItem;
	if(parent.isValid()) parentItem = getItem(parent);
	else parentItem = root;
	int cc = parentItem->childCount();
	beginInsertRows(parent,cc,cc);
	parentItem->insertChild(cc,child);
	endInsertRows();
}

void TreeModel::addParent(TreeItem *newTN)
{
	beginInsertRows(QModelIndex(),rowCount(),rowCount());
	root->insertChild(rowCount(),newTN);
	endInsertRows();
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
	return getItem(parent)->columnCount();
}

static const char s_TreeItemMimeType[] = "application/x-TreeItem";

QStringList TreeModel::mimeTypes() const
{
	return QStringList() << s_TreeItemMimeType;
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData;
	QByteArray data;

	QDataStream stream(&data, QIODevice::WriteOnly);
	QList<TreeItem *> items;

	foreach (const QModelIndex &index, indexes) {
		TreeItem *item = getItem(index);
		if (!items.contains(item))
			items << item;
	}
	stream << QCoreApplication::applicationPid();
	stream << items.count();
	foreach (TreeItem *item, items) {
		stream << reinterpret_cast<qlonglong>(item);
	}
	mimeData->setData(s_TreeItemMimeType, data);
	return mimeData;
}

bool TreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action,
							 int row, int column, const QModelIndex &parent)
{
	Q_ASSERT(action == Qt::MoveAction);
	Q_UNUSED(column);
	if(!mimeData->hasFormat(s_TreeItemMimeType)) {
		return false;
	}
	QByteArray data = mimeData->data(s_TreeItemMimeType);
	QDataStream stream(&data, QIODevice::ReadOnly);
	qint64 senderPid;
	stream >> senderPid;
	if(senderPid != QCoreApplication::applicationPid()) {
		return false;
	}
	TreeItem *parentItem = getItem(parent);
	Q_ASSERT(parentItem);
	int count;
	stream >> count;
	if(row == -1) row = rowCount(parent);
	for(int i = 0; i < count; ++i) {
		qlonglong itemPtr;
		stream >> itemPtr;
		TreeItem *item = reinterpret_cast<TreeItem *>(itemPtr);
		if (item->row() < row && parentItem == item->parent)
			--row;

		removeItem(item);

		beginInsertRows(parent, row, row);
		parentItem->insertChild(row, item);
		endInsertRows();
		++row;
	}
	return true;
}

Qt::DropActions TreeModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

Qt::DropActions TreeModel::supportedDragActions() const
{
	return Qt::MoveAction;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid()) return QVariant();

	if(role != Qt::DisplayRole && role != Qt::EditRole)return QVariant();

	TreeItem *item = getItem(index);
	return item->data(index.column());
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role != Qt::EditRole)
		return false;

	TreeItem *item = getItem(index);
	bool result = item->setData(index.column(), value);

	if(result)
		emit dataChanged(index, index);

	return result;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if(!index.isValid())
		return Qt::ItemIsDropEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,int role) const{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return root->data(section);

	return QVariant();
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
	if(!index.isValid())
		return root;
	else return static_cast<TreeItem*>(index.internalPointer());
}

void TreeModel::removeItem(TreeItem *item)
{
	const int row = item->row();
	QModelIndex idx = createIndex(row, 0, item);
	beginRemoveRows(idx.parent(), row, row);
	item->parent->removeChild(row);
	endRemoveRows();
}

void TreeModel::removeTab(QModelIndex ind){
	TreeItem *tn = getItem(ind);
	if(!tn || tn == root) return;
	tn->deleteLater();
}

void TreeModel::removeChildren(QModelIndex ind){
	TreeItem *tn = getItem(ind);
	if(!tn || tn == root) return;
	tn->removeChildren();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if(!hasIndex(row, column, parent))
		return QModelIndex();

	TreeItem *parentItem = getItem(parent);

	TreeItem *childItem = parentItem->child(row);
	if(childItem) return createIndex(row, column, childItem);
	else return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	TreeItem *childItem = getItem(index);
	if (childItem == root)
		return QModelIndex();

	TreeItem *parentItem = childItem->parent;
	if (parentItem == root)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	if(parent.column() > 0) return 0;

	TreeItem *parentItem = getItem(parent);
	return parentItem->childCount();
}

QModelIndex TreeModel::getIndex(TreeItem *item) const
{
	return createIndex(item->row(),0,item);
}

void TreeModel::saveSessionToFile(QString fileName)
{
	QFile data(fileName);
	data.open(static_cast<QFile::OpenMode>(QFile::WriteOnly | QFile::Truncate));
	QTextStream out(&data);
	QList<TreeItem*> parents;
	QList<int> lines;
	parents << root;
	lines << 0;
	int i;
	TreeItem *parent;
	TreeItem *child;
	QString indent = "\t";
	while(!parents.isEmpty()) {
		parent = parents.last();
		i = lines.last();
		while(i<parent->childCount()) {
			child = parent->child(i);
			out << indent.repeated(lines.size()-1);
			out << child->query << "\t" << child->display << endl;
			i++;
			lines.last()++;
			if(child->childCount()) {
				parents << child;
				lines << 0;
				break;
			}
		}
		while (!parents.isEmpty() && parents.last()->childCount() == lines.last()) {
			lines.pop_back();
			parents.pop_back();
		}
	}
}

void TreeModel::loadSessionFromFile(QString sessionFile)
{
	QFile session(sessionFile);
	session.open(QFile::ReadOnly);
	QTextStream in(&session);
	QString line;
	QList<TreeItem*> parents;
	parents.append(root);
	QList<int> indents;
	indents << 0;
	int position;
	while(!in.atEnd()) {
		line = in.readLine();
		if(line.isEmpty())continue;
		position = 0;
		while(position < line.length()) {
			if(line.at(position) != '\t') break;
			position++;
		}
		line = line.mid(position).trimmed();
		if(line.isEmpty())continue;
		QStringList columns = line.split("\t", QString::SkipEmptyParts);
		if(columns.size() == 1) columns.append(columns.at(0));
		if(position > indents.last()) {
			if(parents.last()->childCount() > 0) {
				parents << parents.last()->child(parents.last()->childCount()-1);
				indents << position;
			}
		}
		else{
			while(position < indents.last() && parents.count() > 0) {
				parents.pop_back();
				indents.pop_back();
			}
		}
		emit loadFromSearch(columns.at(0),columns.at(1),parents.last(),false);
	}
}

void TreeModel::addTab(TreeItem *child, TreeItem *parent, bool select)
{
	if(parent == Q_NULLPTR || parent == root) {
		addParent(child);
	}
	else{
		QModelIndex ind = getIndex(parent);
		addChild(ind,child);
	}
	connect(child,&TreeItem::deleting,this,&TreeModel::onDeleting);
	if(select) {
		emit selectTab(getIndex(child));
	}
}

void TreeModel::onDeleting(TreeItem *tn){
	removeItem(tn);
	emit removingTab(tn);
}
