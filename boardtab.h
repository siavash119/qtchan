#ifndef BOARDTAB_H
#define BOARDTAB_H

#include <QWidget>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "threadform.h"

namespace Ui {
class BoardTab;
}

enum BoardType{Index,Catalog};
class BoardTab : public QWidget
{
    Q_OBJECT

public:
    QString tabType;
    QString board;
    QString boardUrl;
    BoardType type;
    QString search;
    explicit BoardTab(QString board, BoardType type = BoardType::Index, QString search = "", QWidget *parent = 0);
    ~BoardTab();
    void addThread();
    QNetworkReply *reply;
    //TODO change to map
    QMap<QString,ThreadForm*> tfMap;
    void updatePosts();
    void setShortcuts();
    void getPosts();
    void focusIt();
    void startUp();

public slots:
    void findText(const QString text);

private:
    Ui::BoardTab *ui;

private slots:
    void loadThreads();
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();

signals:
    void finished();
};

#endif // BOARDTAB_H
