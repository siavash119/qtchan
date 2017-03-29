#include "boardtab.h"
#include "ui_boardtab.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QSettings>
#include "netcontroller.h"
#include "mainwindow.h"
#include "threadform.h"

BoardTab::BoardTab(QString board, BoardType type, QString search, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BoardTab)
{
    ui->setupUi(this);
    this->board = board;
    this->type = type;
    this->search = search;
    if(type == BoardType::Index) boardUrl = "https://a.4cdn.org/"+board+"/1.json";
    else boardUrl = "https://a.4cdn.org/"+board+"/catalog.json";
    //QDir().mkdir(board);
    QDir().mkpath(board+"/index/thumbs");
    //QDir().mkdir(board+"/thumbs");
    setShortcuts();
    reply = nc.manager->get(QNetworkRequest(QUrl(boardUrl)));
    connect(reply, &QNetworkReply::finished, this, &BoardTab::loadThreads);
}

BoardTab::~BoardTab()
{
    delete ui;
}

void BoardTab::setShortcuts(){
    QAction *refresh = new QAction(this);
    refresh->setShortcut(Qt::Key_R);
    connect(refresh, &QAction::triggered, this, &BoardTab::getPosts);
    this->addAction(refresh);
    QAction *focuser = new QAction(this);
    focuser->setShortcut(Qt::Key_F3);
    connect(focuser,&QAction::triggered,mw,&MainWindow::focusTree);
    this->addAction(focuser);
    QAction *focusBar = new QAction(this);
    focusBar->setShortcut(Qt::Key_F6);
    connect(focusBar,&QAction::triggered,mw,&MainWindow::focusBar);
    this->addAction(focusBar);
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
    QJsonArray threads;
    if(type==BoardType::Index) threads = QJsonDocument::fromJson(reply->readAll()).object()["threads"].toArray();
    else{
        qDebug() << "searching " + search;
        QRegularExpression re(search,QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match;
        QJsonArray allThreads = QJsonDocument::fromJson(reply->readAll()).array();
        QJsonArray pageThreads;
        for(int i=0;i<allThreads.size();i++){
            pageThreads = allThreads.at(i).toObject().value("threads").toArray();
            for(int j=0;j<pageThreads.size();j++){
                match = re.match(pageThreads.at(j).toObject().value("com").toString());
                if(match.hasMatch())threads.append(pageThreads.at(j));
            }
        }
    }
    int length = threads.size();
    qDebug() << QString("length is ").append(QString::number(length));
    QSettings settings;
    QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
    for(i=0;i<length;i++){
        QJsonObject p;
        if(type==BoardType::Index) p = threads.at(i).toObject()["posts"].toArray()[0].toObject();
        else p = threads.at(i).toObject();
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
