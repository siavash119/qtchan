#include "netcontroller.h"
#include "threadtab.h"
#include "ui_threadtab.h"
#include "threadform.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ThreadTab::ThreadTab(QString board, QString thread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadTab)
{
    ui->setupUi(this);
    this->board = board;
    this->thread = thread;
    QString url = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
    reply = nc.manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, &ThreadTab::loadPosts);
}

ThreadTab::~ThreadTab()
{
    delete ui;
}

void ThreadTab::addStretch(){
    ui->threads->addStretch(1);
}

void ThreadTab::addPost(ThreadForm *tf){
    ui->threads->addWidget(tf);
}

void ThreadTab::loadPosts(){
    QJsonArray posts = QJsonDocument::fromJson(reply->readAll()).object()["posts"].toArray();
    int length = posts.size();
    qDebug() << QString("length is ").append(QString::number(length));
    for(int i=0;i<length;i++){
        ThreadForm *tf = new ThreadForm();
        QJsonObject p = posts.at(i).toObject();
        tf->board = board;
        tf->threadNum = QString("%1").arg(p["no"].toDouble(),0,'f',0);
        tf->load(p);
        this->addPost(tf);
    }
    this->addStretch();
    reply->deleteLater();

}
