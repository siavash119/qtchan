#include "netcontroller.h"
#include "threadtab.h"
#include "ui_threadtab.h"
#include "threadform.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>

ThreadTab::ThreadTab(QString board, QString thread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadTab)
{
    this->updated = false;
    ui->setupUi(this);
    this->board = board;
    this->thread = thread;
    QDir().mkpath(board+"/"+thread);
    QString url = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
    reply = nc.manager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, &ThreadTab::loadPosts);
    myProcess = new QProcess(parent);
    QAction *foo = new QAction(this);
    foo->setShortcut(Qt::Key_G);
    connect(foo, &QAction::triggered, this, &ThreadTab::gallery);
    this->addAction(foo);
}

ThreadTab::~ThreadTab()
{
    delete ui;
}

void ThreadTab::gallery(){
    QString command = "feh"; QStringList arguments; arguments << QDir("./"+board+"/"+thread).absolutePath()
                                                              << "--auto-zoom"
                                                              << "--index-info" << "\"%n\n%S\n%wx%h\""
                                                              << "--borderless"
                                                              << "--image-bg" << "black"
                                                              << "--preload";
    myProcess->startDetached(command,arguments);
}

void ThreadTab::addPost(ThreadForm *tf){
    ui->threads->addWidget(tf);
}

void ThreadTab::addStretch(){
    ui->threads->addStretch(1);
}

void ThreadTab::loadPosts(){
    QJsonArray posts = QJsonDocument::fromJson(reply->readAll()).object()["posts"].toArray();
    int length = posts.size();
    qDebug() << QString("length is ").append(QString::number(length));
    for(int i=0;i<length;i++){
        QJsonObject p = posts.at(i).toObject();
        ThreadForm *tf = new ThreadForm(board,thread);
        ui->threads->addWidget(tf);
        tf->load(p);
        tfs.push_back(tf);
    }
    ui->threads->addStretch(1);
    reply->deleteLater();
}

void ThreadTab::updatePosts(){
    updated = false;
    int length = tfs.size();
    for(int i=0;i<length;i++){
        ((ThreadForm*)tfs.at(i))->updateComHeight();
    }
}
