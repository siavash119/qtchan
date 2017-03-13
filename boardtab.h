#ifndef BOARDTAB_H
#define BOARDTAB_H

#include <QWidget>
#include "threadform.h"
#include <QNetworkReply>
#include <QNetworkRequest>

namespace Ui {
class BoardTab;
}

class BoardTab : public QWidget
{
    Q_OBJECT

public:
    QString tabType;
    QString board;
    QString boardUrl;
    explicit BoardTab(QString board, QWidget *parent = 0);
    ~BoardTab();
    void addThread();
    QNetworkReply *reply;
    std::vector<ThreadForm*> posts;
    void updatePosts();

private:
    Ui::BoardTab *ui;

private slots:
    void loadThreads();
    void getPosts();
};

#endif // BOARDTAB_H
