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

private slots:
    void on_pushButton_clicked();
    //void replyFinished();

    void on_treeView_clicked(QModelIndex index);
    void onSelectionChanged();
    //void onSelectionChanged(const QItemSelection &, const QItemSelection &);

public slots:
    void onNewThread(ThreadForm* tf, QString board, QString thread);
private:
    Ui::MainWindow *ui;
    QModelIndexList boardsSelected;

signals:
    void requestCatalog(QString);
};

extern MainWindow* mw;

#endif // MAINWINDOW_H
