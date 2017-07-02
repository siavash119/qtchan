#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QUrl>
#include <QTreeView>
#include <QMap>
#include "netcontroller.h"
#include "boardtab.h"
#include "threadform.h"
#include "treemodel.h"
#include "treeitem.h"

struct Tab{
	enum TabType {Board,Thread} type;
	void *TabPointer;
	TreeItem *tn;
	QString query;
	QString display;
};

struct TreeTab{
	QString query;
	QString display = "";
	bool isExpanded = true;
	QList<TreeTab> children = QList<TreeTab>();
	//TreeTab *parent = 0;
};

Q_DECLARE_METATYPE(TreeTab)

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
	Q_OBJECT
protected:
	bool eventFilter(QObject *obj, QEvent *ev);

public:
	explicit MainWindow(QWidget *parent = 0);
	TreeModel *model = new TreeModel(this);
	QItemSelectionModel *selectionModel;
	QMap<QWidget*,Tab> tabs;
	~MainWindow();

	void addTab(TreeItem *child, TreeItem *parent = Q_NULLPTR, bool select = false);
	void deleteSelected();
	void loadSession();
	QObject *currentWidget();

private slots:
	void on_pushButton_clicked();
	void on_treeView_clicked(QModelIndex index);
	void onSelectionChanged();
	void on_lineEdit_returnPressed();

public slots:
	void focusTree();
	void focusBar();
	TreeItem *loadFromSearch(QString query, QString display, TreeItem *childOf, bool select = false);
	TreeItem *onNewThread(QWidget *parent, QString board, QString thread, QString display, TreeItem *childOf);
	void saveSession();
	void loadSessionFromFile(QString sessionFile);
	void prevTab();
	void nextTab();
	void prevParent();
	void nextParent();
	void toggleAutoUpdate();
	void toggleAutoExpand();
	void openExplorer();

private:
	Ui::MainWindow *ui;
	void setShortcuts();
	void removeTabs(TreeItem *tn);
	TreeTab saveChildren(TreeItem *tn, TreeTab *parent);
	void saveTreeToFile(QString fileName);
	void saveChildrenToFile(TreeItem *tn, int indent, QTextStream *out);
	void loadChildren(TreeTab session, TreeItem *parent = 0);

signals:
	void requestCatalog(QString);
	void setAutoUpdate(bool autoUpdate);
	void setAutoExpand(bool autoExpand);
};

extern MainWindow *mw;

enum FilterType{id,comment,trip};
enum BoardName{b,g,diy,h,pol,d};


#endif // MAINWINDOW_H
