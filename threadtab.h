#ifndef THREADTAB_H
#define THREADTAB_H

#include <QWidget>
#include "threadform.h"
#include "boardtab.h"
#include <QNetworkReply>
#include <QNetworkRequest>

namespace Ui {
class ThreadTab;
}

class ThreadTab : public QWidget
{
    Q_OBJECT
public:
    BoardTab *boardtab;
    QString board;
    QString thread;
    explicit ThreadTab(QString board, QString thread, QWidget *parent = 0);
    ~ThreadTab();
    void addPost(ThreadForm *tf);
    void addStretch();
    void addThread();
    QNetworkReply *reply;
    std::vector<ThreadForm*> tfs;
    void updatePosts();
    bool updated;

private:
    Ui::ThreadTab *ui;

private slots:
    void loadPosts();

signals:
    void newPosts();
};

#endif // THREADTAB_H
