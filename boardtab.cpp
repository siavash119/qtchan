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
    //TODO check if actual board
    this->board = board;
    this->type = type;
    this->search = search;
    this->setWindowTitle("/"+board+"/"+search);
    if(type == BoardType::Index) boardUrl = "https://a.4cdn.org/"+board+"/1.json";
    else boardUrl = "https://a.4cdn.org/"+board+"/catalog.json";
    startUp();
    //QDir().mkdir(board);
    /*QDir().mkpath(board+"/index/thumbs");
    //QDir().mkdir(board+"/thumbs");
    setShortcuts();
    reply = nc.jsonManager->get(QNetworkRequest(QUrl(boardUrl)));
    connect(reply, &QNetworkReply::finished, this, &BoardTab::loadThreads);*/
}


void BoardTab::startUp(){
    QDir().mkpath(board+"/index/thumbs");
    //QDir().mkdir(board+"/thumbs");
    setShortcuts();
    reply = nc.jsonManager->get(QNetworkRequest(QUrl(boardUrl)));
    connect(reply, &QNetworkReply::finished, this, &BoardTab::loadThreads);
}

BoardTab::~BoardTab()
{
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        disconnect(mapI.value());
        mapI.value()->deleteLater();
        mapI.remove();
        //QCoreApplication::processEvents();
    }
    emit finished();
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
    QAction *focusSearch = new QAction(this);
    focusSearch->setShortcut(QKeySequence("Ctrl+f"));
    connect(focusSearch,&QAction::triggered,this,&BoardTab::focusIt);
    this->addAction(focusSearch);
}

void BoardTab::findText(const QString text){
    QRegularExpression re(text,QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    ThreadForm* tf;
    bool pass = false;
    if (text == "") pass = true;
    qDebug().noquote() << "searching " + text;
    QMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        tf = mapI.value();
        if(pass) { tf->show(); continue;};
        match = re.match(tf->post.com);
        if(!match.hasMatch()){
            tf->hide();
        }
        else qDebug().noquote().nospace() << "found " << text << " in thread #" << tf->post.no;
    }
}

void BoardTab::getPosts(){
    qDebug("refreshing /%s/",board.toLatin1().constData());
    reply = nc.jsonManager->get(QNetworkRequest(QUrl(boardUrl)));
    connect(reply, &QNetworkReply::finished, this, &BoardTab::loadThreads);
}


void BoardTab::updatePosts(){
    /*int length = posts.size();
    for(int i=0;i<length;i++){
        ((ThreadForm*)posts.at(i))->updateComHeight();
    }*/
}

void BoardTab::loadThreads(){
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        delete mapI.value();
        mapI.remove();
        //QCoreApplication::processEvents();
    }
    QJsonArray threads;
    if(reply->error()){
        qDebug().noquote() << "loading post error:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    if(type==BoardType::Index) threads = QJsonDocument::fromJson(reply->readAll()).object().value("threads").toArray();
    else{
        qDebug("searching %s",search.toLatin1().constData());
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
    qDebug("%s",QString("length is ").append(QString::number(length)).toLatin1().constData());
    QSettings settings;
    QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
    for(int i=0;i<length;i++){
        QJsonObject p;
        if(type==BoardType::Index) p = threads.at(i).toObject().value("posts").toArray().at(0).toObject();
        else p = threads.at(i).toObject();
        QString threadNum = QString("%1").arg(p.value("no").toDouble(),0,'f',0);
        if (!idFilters.contains(threadNum)){
            ThreadForm *tf = new ThreadForm(board,threadNum,Thread,true,false,this);
            ui->threads->addWidget(tf);
            tf->load(p);
            tfMap.insert(tf->post.no,tf);
            //posts.push_back(tf);
        }
        else{
            qDebug("threadNum %s filtered!",threadNum.toLatin1().constData());
        }
        //QCoreApplication::processEvents();
    }
    ui->threads->addStretch(1);
    reply->deleteLater();
}

void BoardTab::on_pushButton_clicked()
{
    findText(ui->lineEdit->text());
}

void BoardTab::on_lineEdit_returnPressed()
{
    findText(ui->lineEdit->text());
}

void BoardTab::focusIt(){
    ui->lineEdit->setFocus();
}
