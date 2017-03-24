#include "boardtab.h"
#include "ui_boardtab.h"
#include "netcontroller.h"
#include "threadform.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QSettings>

BoardTab::BoardTab(QString board, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BoardTab)
{
    ui->setupUi(this);
    this->board = board;
    boardUrl = "https://a.4cdn.org/"+board+"/1.json";
    //QDir().mkdir(board);
    QDir().mkpath(board+"/index/thumbs");
    //QDir().mkdir(board+"/thumbs");
    reply = nc.manager->get(QNetworkRequest(QUrl(boardUrl)));
    connect(reply, &QNetworkReply::finished, this, &BoardTab::loadThreads);
    QAction *refresh = new QAction(this);
    refresh->setShortcut(Qt::Key_R);
    connect(refresh, &QAction::triggered, this, &BoardTab::getPosts);
    this->addAction(refresh);
}

BoardTab::~BoardTab()
{
    delete ui;
}

void BoardTab::getPosts(){
    qDebug() << "refreshing /" + board + "/";
    reply = nc.manager->get(QNetworkRequest(QUrl(boardUrl)));
    connect(reply, &QNetworkReply::finished, this, &BoardTab::loadThreads);
}


void BoardTab::updatePosts(){
    int length = posts.size();
    for(int i=0;i<length;i++){
        ((ThreadForm*)posts.at(i))->updateComHeight();
    }
}

void BoardTab::loadThreads(){
    int i = posts.size();
    while(i--){
        //((ThreadForm*)posts.at(i))->deleteLater();
        ((ThreadForm*)posts.at(i))->close();
        ((ThreadForm*)posts.at(i))->deleteLater();
        posts.pop_back();
    }
    QJsonArray threads = QJsonDocument::fromJson(reply->readAll()).object()["threads"].toArray();
    int length = threads.size();
    qDebug() << QString("length is ").append(QString::number(length));
    QSettings settings;
    QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
    for(i=0;i<length;i++){
        QJsonObject p = threads.at(i).toObject()["posts"].toArray()[0].toObject();
        QString threadNum = QString("%1").arg(p["no"].toDouble(),0,'f',0);
        if (!idFilters.contains(threadNum)){
            ThreadForm *tf = new ThreadForm(board,threadNum,Thread,this);
            ui->threads->addWidget(tf);
            tf->load(p);
            posts.push_back(tf);
        }
        else{
            qDebug() << "threadNum "+threadNum+" filtered!";
        }
    }
    ui->threads->addStretch(1);
    reply->deleteLater();
}
