#ifndef THREADFORM_H
#define THREADFORM_H

#include <QWidget>
#include <QNetworkReply>
#include <QByteArray>
#include <QString>
#include "post.h"
#include <QDir>
#include <QSet>
#include <QSignalMapper>

namespace Ui {
class ThreadForm;
}

enum PostType { Thread, Reply };

class ThreadForm : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadForm(QString board, QString threadNum, PostType type = Reply, QWidget *parent = 0);
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
    QString htmlParse(QString &html);
    Post *post;
    void updateComHeight();
    QDir *folder;
    QString folderPath;
    //void setImage(QString text);
    QSignalMapper *signalMapper;
    //void insert(int position, ThreadForm* tf);
    void insert(ThreadForm* tf);
    QSet<QString> quotelinks;
    QSet<QString> replies;
    void setReplies();
private:
    Ui::ThreadForm *ui;
    ThreadForm* clone();
    bool loadIt;
    QString imgURL;
    QString thumbURL;
    QString pathBase;
    QString filePath;
    QFile *file;
    QString thumbPath;
    QFile *thumb;
    QMetaObject::Connection connectionPost;
    QMetaObject::Connection connectionThumb;
    QMetaObject::Connection connectionImage;
    QNetworkReply *reply;
    QNetworkReply *replyThumb;
    QNetworkReply *replyImage;

signals:
    void loadThreadTab(ThreadForm*, QJsonArray&);
    void loadThread(ThreadForm*,QString&,QString&);
    //void searchPost(int position, QString postNum);
    void searchPost(QString postNum, ThreadForm* thetf);

public slots:
    void getOrigFinished();
    void getThumbFinished();
    void imageClicked();
    void hideClicked();
private slots:
    void quoteClicked(const QString &link);
    void onSearchPost(const QString &link, ThreadForm* thetf);

};

#endif // THREADFORM_H
