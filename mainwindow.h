#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QItemSelection>
#include "boardtab.h"
#include <vector>
#include "threadform.h"
#include "netcontroller.h"
#include <QUrl>
#include <QShortcut>
#include <QShortcutEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QNetworkReply *reply;
    std::vector<Tab> tabs;
    //std::vector<QWidget*> tabs;

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

public:
    explicit MainWindow(QWidget *parent = 0);
    QStandardItemModel* model;

    void addTab();
    ~MainWindow();

    void show_one(QModelIndex index);
    void deleteSelected();
    //void getThread(ThreadForm *tf, QString url);
    QShortcut *shortcut;

private slots:
    void on_pushButton_clicked();
    //void replyFinished();

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
    void loadSession();
    void nextTab();
    void prevTab();
private:
    Ui::MainWindow *ui;
    QModelIndexList boardsSelected;
    void setShortcuts(QWidget* parent);

signals:
    void requestCatalog(QString);
};

extern MainWindow* mw;

enum FilterType{id,comment,trip};
enum BoardName{b,g,diy,h,pol,d};
#endif // MAINWINDOW_H
