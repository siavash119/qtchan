#include "boardtab.h"
#include "ui_boardtab.h"
#include "netcontroller.h"
#include "threadform.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

BoardTab::BoardTab(QString board, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BoardTab)
{
    ui->setupUi(this);
    this->board = board;
    QString url = "https://a.4cdn.org/"+board+"/1.json";
    reply = nc.manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, &BoardTab::loadThreads);
}

BoardTab::~BoardTab()
{
    delete ui;
}

void BoardTab::addPost(ThreadForm *tf){
    ui->threads->addWidget(tf);
}

void BoardTab::addStretch(){
    ui->threads->addStretch(1);
}

void BoardTab::addThread(){

}

void BoardTab::loadThreads(){
    QJsonArray threads = QJsonDocument::fromJson(reply->readAll())
            .object()["threads"].toArray();
    qDebug() << threads;
    int length = threads.size();
    qDebug() << QString("length is ").append(QString::number(length));
    for(int i=0;i<length;i++){
        ThreadForm *tf = new ThreadForm(Thread);
        QJsonObject p = threads.at(i).toObject()["posts"].toArray()[0].toObject();
        tf->board = board;
        tf->threadNum = QString("%1").arg(p["no"].toDouble(),0,'f',0);
        tf->load(p);
        this->addPost(tf);
    }
    this->addStretch();
    reply->deleteLater();
}
