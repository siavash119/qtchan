#ifndef THREADTAB_H
#define THREADTAB_H

#include <QWidget>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QMap>
#include <QMutableMapIterator>
#include <QPointer>
#include "boardtab.h"
#include "clickablelabel.h"
#include "threadform.h"
#include "postform.h"

namespace Ui {
class ThreadTab;
}

class ThreadTab : public QWidget
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event);
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
    //std::vector<ThreadForm*> tfs;
    QMap<QString,ThreadForm*> tfMap;
    void updatePosts();
    bool updated;
    //QProcess *myProcess;
    void findText(const QString text);
    PostForm *myPostForm;
    void loadAllImages();
    ThreadForm* findPost(QString postNum);
    int getMinWidth();
    void setMinWidth(int minw);
    QPointer<ThreadForm> floating;
    bool floatIt;
    void getPosts();

public slots:
    //void findPost(int position, QString postNum);
    void focusIt();
    void updateWidth();
    void quoteIt(QString text);
    void floatReply(const QString &link);
    void deleteFloat();
    void updateFloat();

private:
    Ui::ThreadTab *ui;
    void setShortcuts();

private slots:
    void loadPosts();
    void gallery();
    void openPostForm();
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();

signals:
    void newPosts();
};

#endif // THREADTAB_H
