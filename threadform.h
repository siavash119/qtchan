#ifndef THREADFORM_H
#define THREADFORM_H

#include <QWidget>
#include <QNetworkReply>
#include <QByteArray>
#include <QString>
#include "post.h"

namespace Ui {
class ThreadForm;
}

enum PostType { Thread, Reply };

class ThreadForm : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadForm(PostType type = Reply, QWidget *parent = 0);
    ~ThreadForm();
    void setText(QString text);
    void setImage(QByteArray img);
    //void getImage(QNetworkAccessManager *manager, QString *img);
    void load(QJsonObject &p);
    //void setThread(QString threadName);
    PostType type;
    QString threadNum;
    QString board;
    double no;
    QString time;
    QString name;
    QNetworkReply *reply;
    QNetworkReply *replyImage;
    QMetaObject::Connection connectionPost;
    QMetaObject::Connection connectionImage;
    QString htmlParse(QString &html);
    Post *post;
    void updateComHeight();
    //void setImage(QString text);

signals:
    void loadThreadTab(ThreadForm*, QJsonArray&);
    void loadThread(ThreadForm*,QString&,QString&);
private:
    Ui::ThreadForm *ui;

public slots:
    void getImageFinished();
    void imageClicked();
};

#endif // THREADFORM_H
