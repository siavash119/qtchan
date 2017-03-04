#include "threadform.h"
#include "ui_threadform.h"
#include <QByteArray>
#include <QPixmap>
#include <QImageReader>
#include <QFile>
#include <iostream>
#include "netcontroller.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "mainwindow.h"
using namespace std;


ThreadForm::ThreadForm(PostType type, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadForm)
{
    this->type = type;
    ui->setupUi(this);
}

ThreadForm::~ThreadForm()
{
    delete ui;
}

void ThreadForm::setText(QString text){
    ui->com->setPlainText(text);
//    ui->plainTextEdit->resize(1000,2000);
}

/*void ThreadForm::setThread(QString threadName){
    qDebug() << this->threadNum;
}*/

void ThreadForm::load(QJsonObject &p){
    //set post number
    ui->no->setPlainText(QString("%1").arg(p["no"].toDouble(),0,'f',0));

    //set comment
    QString com = p["com"].toString();
    //TODO replace <span class="quote"> and <a href="url">
    com = htmlParse(com);
    ui->com->setPlainText(com);

    //set subject
    QString sub = p["sub"].toString();
    sub = htmlParse(sub);
    ui->sub->setPlainText(sub);

    //set image
    if(!p["tim"].isNull()){
        QString img;
        QString ext = p["ext"].toString();
        if(ext==".jpg" || ext == ".png"){
            img = "https://i.4cdn.org/"+this->board+"/"
                    +QString("%1").arg(p["tim"].toDouble(),0,'f',0).append(ext);
        }
        else {
            img = "https://i.4cdn.org/"+this->board+"/"
                    +QString("%1").arg(p["tim"].toDouble(),0,'f',0).append("s.jpg");

        }
        qDebug() << QString("getting ")+img;
        replyImage = nc.manager->get(QNetworkRequest(QUrl(img)));
        ui->tim->setFixedHeight(250);
        this->setFixedHeight(250);
        connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::getImageFinished);
        (this->type == Thread) && connect(ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
    }
}

void ThreadForm::getImageFinished(){
    if(replyImage->error() == 0)
    {
        int scale = 250;
        QPixmap pic;
        pic.loadFromData(replyImage->readAll());
        //int scale = this->type == Thread ? 250 : 125;
        if(pic.height() > pic.width()) ui->tim->setPixmap(pic.scaledToHeight(scale, Qt::SmoothTransformation));
        else ui->tim->setPixmap(pic.scaledToWidth(scale, Qt::SmoothTransformation));
        //this->setFixedHeight(pic.height());
    }
        replyImage->deleteLater();
        disconnect(connectionImage);
}

void ThreadForm::imageClicked(){
    mw->onNewThread(this,board,threadNum);
    //emit loadThread(this,board,threadNum);
    /*QString url = "https://a.4cdn.org/"+this->board+"/thread/"
            +this->threadNum+".json";
    qDebug() << QString("getting ")+url;
    reply = nc.manager->get(QNetworkRequest(QUrl(url)));
    connectionPost = connect(reply, &QNetworkReply::finished,this,&ThreadForm::getThreadFinished);*/
}

void ThreadForm::getThreadFinished(){
    if(reply->error() == 0){
        QJsonArray posts = QJsonDocument::fromJson(reply->readAll()).object()["posts"].toArray();
        qDebug() << posts;
        emit loadThreadTab(this,posts);
    }
    reply->deleteLater();
    disconnect(connectionPost);
}

QString ThreadForm::htmlParse(QString &html){
    return html.replace("<br>","\n").replace("&amp;","&")
            .replace("&gt;",">").replace("&lt;","<")
            .replace("&quot","\"").replace("&#039;","'")
            .replace("<wb>","").replace("<wbr>","");
}
