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
    QString board;
    explicit BoardTab(QString board, QWidget *parent = 0);
    ~BoardTab();
    void addPost(ThreadForm *tf);
    void addStretch();
    void addThread();
    QNetworkReply *reply;

private:
    Ui::BoardTab *ui;

private slots:
    void loadThreads();
};

#endif // BOARDTAB_H
