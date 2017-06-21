#ifndef THREADTAB_H
#define THREADTAB_H

#include <QWidget>
#include <QMutableMapIterator>
#include <QPointer>
#include <QThread>
#include <QSpacerItem>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include "clickablelabel.h"
#include "threadtabhelper.h"
#include "postform.h"

namespace Ui {
class ThreadTab;
}

class ThreadTab : public QWidget
{
    Q_OBJECT
    Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
    QSpacerItem space = QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
public:
    QString board;
    QString thread;
    QString threadUrl;
    explicit ThreadTab(QString board, QString thread, QWidget *parent = 0);
    ~ThreadTab();
    QMap<QString,ThreadForm*> tfMap;
    bool updated;
    void findText(const QString text);
    PostForm myPostForm;
    void loadAllImages();
    ThreadForm* findPost(QString postNum);
    int getMinWidth();
    void setMinWidth(int minw);
    QPointer<ThreadForm> floating;
    bool floatIt;
    QThread workerThread;
    ThreadTabHelper helper;
    int formsTotal = 0;
    int formsUnseen = 0;
    static int checkIfVisible(QMap<QString,ThreadForm*> &tfMap);

public slots:
    void addStretch();
    void focusIt();
    void updateWidth();
    void quoteIt(QString text);
    void floatReply(const QString &link);
    void deleteFloat();
    void updateFloat();
    void onNewTF(ThreadForm* tf);
    void onWindowTitle(QString title);
    //void checkScroll();

private:
    Ui::ThreadTab *ui;
    QMetaObject::Connection connectionAutoUpdate;
    void setShortcuts();
    QFuture<int> newImage;
    QFutureWatcher<int> watcher;

private slots:
    void gallery();
    void openPostForm();
    void on_pushButton_clicked();
    void on_lineEdit_returnPressed();

signals:
    void autoUpdate(bool update);
    void unseen(int totalUnseen);
};

#endif // THREADTAB_H
