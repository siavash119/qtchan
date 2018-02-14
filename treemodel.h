#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class TreeItem;

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit TreeModel(QObject *parent = 0);
	~TreeModel();

	QVariant data(const QModelIndex &index, int role = Qt::EditRole) const Q_DECL_OVERRIDE;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
	int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	bool insertRows(int position, int rows,const QModelIndex &parent = QModelIndex()) override;
	void addChild(QModelIndex &parent, TreeItem *child);
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;
	Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
	QStringList mimeTypes() const Q_DECL_OVERRIDE;
	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;
	TreeItem *root;
	void addParent(TreeItem *newTN);
	void removeItem(TreeItem *item);
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QModelIndex getIndex(TreeItem *item) const;
	TreeItem *getItem(const QModelIndex &index) const;

	void saveSessionToFile(QString filename);
	void loadSessionFromFile(QString filename);
	void addTab(TreeItem *child, TreeItem *parent, bool select);
	void removeTab(QModelIndex ind);
	void removeChildren(QModelIndex ind);

signals:
	void loadFromSearch(QString query,QString display,TreeItem* tn,bool select = false);
	void selectTab(QModelIndex ind);
	void removingTab(TreeItem *tn);
};

#endif // TREEMODEL_H
