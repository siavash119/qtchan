#ifndef THREADTAB_H
#define THREADTAB_H

#include <QWidget>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QMap>
#include <QMutableMapIterator>
#include "boardtab.h"
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
    QProcess *myProcess;
    void findText(const QString text);
    PostForm *myPostForm;
    void loadAllImages();
    ThreadForm* findPost(QString postNum);

public slots:
    //void findPost(int position, QString postNum);
    void focusIt();
private:
    Ui::ThreadTab *ui;
    void setShortcuts();

private slots:
    void loadPosts();
    void gallery();
    void openPostForm();
    void getPosts();
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();

signals:
    void newPosts();
};

#endif // THREADTAB_H
