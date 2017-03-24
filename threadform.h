#ifndef THREADFORM_H
#define THREADFORM_H

#include <QWidget>
#include <QNetworkReply>
#include <QByteArray>
#include <QString>
#include "post.h"
#include <QDir>
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
    //void setThread(QString threadName);
    PostType type;
    QString threadNum;
    QString board;
    double no;
    QString time;
    QString name;
    QNetworkReply *reply;
    QNetworkReply *replyThumb;
    QNetworkReply *replyImage;
    QMetaObject::Connection connectionPost;
    QMetaObject::Connection connectionThumb;
    QMetaObject::Connection connectionImage;
    QString htmlParse(QString &html);
    Post *post;
    void updateComHeight();
    QDir *folder;
    QString folderPath;
    QString imgURL;
    QString thumbURL;
    QString pathBase;
    QString filePath;
    QFile *file;
    QString thumbPath;
    QFile *thumb;
    //void setImage(QString text);
    QSignalMapper *signalMapper;
    bool loadIt;
    //void insert(int position, ThreadForm* tf);
    void insert(ThreadForm* tf);
    ThreadForm* clone();

signals:
    void loadThreadTab(ThreadForm*, QJsonArray&);
    void loadThread(ThreadForm*,QString&,QString&);
    //void searchPost(int position, QString postNum);
    void searchPost(QString postNum, ThreadForm* thetf);
private:
    Ui::ThreadForm *ui;

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
