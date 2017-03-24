#ifndef THREADTAB_H
#define THREADTAB_H

#include <QWidget>
#include "threadform.h"
#include "boardtab.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>

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
    QString threadUrl;
    explicit ThreadTab(QString board, QString thread, QWidget *parent = 0);
    ~ThreadTab();
    void addPost(ThreadForm *tf);
    void addStretch();
    void addThread();
    QNetworkRequest request;
    QNetworkReply *reply;
    std::vector<ThreadForm*> tfs;
    void updatePosts();
    bool updated;
    QProcess *myProcess;

public slots:
    //void findPost(int position, QString postNum);
    void findPost(QString postNum, ThreadForm* thetf);
private:
    Ui::ThreadTab *ui;

private slots:
    void loadPosts();
    void gallery();
    void openPostForm();
    void getPosts();

signals:
    void newPosts();
};

#endif // THREADTAB_H
