#ifndef THREADFORM_H
#define THREADFORM_H

#include <QWidget>
#include <QNetworkReply>
#include <QByteArray>
#include <QString>
#include <QDir>
#include <QSet>
#include <QList>
#include <QSignalMapper>
#include <QMouseEvent>
#include <QFutureWatcher>
#include "post.h"

namespace Ui {
class ThreadForm;
}

enum PostType { Thread, Reply };

class ThreadForm : public QWidget
{
    Q_OBJECT
    bool gettingFile = false;

public:
    explicit ThreadForm(QString board, QString threadNum, PostType type = Reply, bool root = true, bool autoExpand = false, QWidget *parent = 0);
    ~ThreadForm();
    void setText(QString text);
    void setImage(QByteArray img);
    //void getImage(QNetworkAccessManager *manager, QString *img);
    void load(QJsonObject &p);
    void loadImage(QString path);
    void openImage();
    PostType type;
    QString threadNum;
    QString board;
    static QString htmlParse(QString &html);
    static QString titleParse(QString &title);
    Post *post;
    void updateComHeight();
    QDir *folder;
    QString folderPath;
    //void setImage(QString text);
    QSignalMapper *signalMapper;
    //void insert(int position, ThreadForm* tf);
    void insert(ThreadForm* tf);
    void deleteHideLayout();
    QSet<QString> quotelinks;
    QMap<double,QString> replies;
    void setReplies();
    void loadOrig();
    ThreadForm* clone();
    QList<ThreadForm*> clones;
    //TODO check settings -> filter
    bool hidden = false;
    bool root;
    bool autoExpand;
    static QImage scaleImage(QString path);
    QFutureWatcher<QImage> watcher;

private:
    Ui::ThreadForm *ui;
    bool loadIt;
    QString fileURL;
    QString thumbURL;
    QString pathBase;
    QString filePath;
    QFile *file;
    QString thumbPath;
    QFile *thumb;
    QMetaObject::Connection connectionPost;
    QMetaObject::Connection connectionThumb;
    QMetaObject::Connection connectionImage;
    //QNetworkReply *reply; //Use for cross-thread gets later?
    QNetworkReply *replyThumb;
    QNetworkReply *replyImage;
    void getFile();
    void getThumb();
    QWidget* tab; //pointer to tab

signals:
    void loadThreadTab(ThreadForm*, QJsonArray&);
    void loadThread(ThreadForm*,QString&,QString&);
    void updateWidth();
    void floatLink(const QString &link);
    void updateFloat();
    //void searchPost(int position, QString postNum);

public slots:
    void getOrigFinished();
    void getThumbFinished();
    void imageClicked();
    void hideClicked();
private slots:
    void quoteClicked(const QString &link);
    void on_replies_linkHovered(const QString &link);
    //void downloading(qint64 read, qint64 total);

    void on_com_linkHovered(const QString &link);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    //void mouseMoveEvent(QMouseEvent *event);
};

#endif // THREADFORM_H
