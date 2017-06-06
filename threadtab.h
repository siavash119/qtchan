#ifndef THREADTAB_H
#define THREADTAB_H

#include <QWidget>
#include <QMutableMapIterator>
#include <QPointer>
#include <QThread>
#include <QSpacerItem>
#include "clickablelabel.h"
#include "threadtabhelper.h"
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
    QString board;
    QString thread;
    QString threadUrl;
    explicit ThreadTab(QString board, QString thread, QWidget *parent = 0);
    ~ThreadTab();
    void addStretch();
    QMap<QString,ThreadForm*> tfMap;
    bool updated;
    void findText(const QString text);
    PostForm *myPostForm;
    void loadAllImages();
    ThreadForm* findPost(QString postNum);
    int getMinWidth();
    void setMinWidth(int minw);
    QPointer<ThreadForm> floating;
    bool floatIt;
    QThread*  workerThread;
    ThreadTabHelper* helper;

public slots:
    void focusIt();
    void updateWidth();
    void quoteIt(QString text);
    void floatReply(const QString &link);
    void deleteFloat();
    void updateFloat();
    void onNewTF(ThreadForm* tf);
    void onWindowTitle(QString title);

private:
    Ui::ThreadTab *ui;
    void setShortcuts();
    QSpacerItem *space;

private slots:
    void gallery();
    void openPostForm();
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();
};

#endif // THREADTAB_H
