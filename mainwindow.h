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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QNetworkReply *reply;
    QMap<int,Tab> tabsNew;

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

public:
    explicit MainWindow(QWidget *parent = 0);
    QStandardItemModel *model;
    int pages = 0;

    void addTab();
    ~MainWindow();
    void removePage(int searchPage, QAbstractItemModel* model, QModelIndex parent = QModelIndex());
    void selectPage(int searchPage, QAbstractItemModel* model, QModelIndex parent = QModelIndex());

    void show_one(QModelIndex index);
    void deleteSelected();
    void loadSession();

private slots:
    void on_pushButton_clicked();

    void on_treeView_clicked(QModelIndex index);
    void onSelectionChanged();
    //void onSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_lineEdit_returnPressed();

public slots:
    void onNewThread(QWidget* parent, QString board, QString thread);
    void focusTree();
    void focusBar();
    void loadFromSearch(QString searchString, bool select);
    void saveSession();
    void nextTab(QModelIndex qmi);
    void prevTab(QModelIndex qmi);
private:
    Ui::MainWindow *ui;
    QModelIndexList boardsSelected;
    void setShortcuts();


signals:
    void requestCatalog(QString);
};

extern MainWindow* mw;

enum FilterType{id,comment,trip};
enum BoardName{b,g,diy,h,pol,d};
#endif // MAINWINDOW_H
