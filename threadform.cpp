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
#include <QStringBuilder>
#include <QDir>
#include <QDesktopServices>
#include <QSignalMapper>
using namespace std;


ThreadForm::ThreadForm(QString board, QString threadNum, PostType type,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadForm)
{
    this->board = board;
    this->threadNum = threadNum;
    this->type = type;
    ui->setupUi(this);
    pathBase = "./"%board%"/" % ((type == PostType::Reply) ? threadNum : "index") % "/";
}

ThreadForm::~ThreadForm()
{
    delete ui;
}

void ThreadForm::setText(QString text){
    ui->com->setPlainText(text);
//    ui->plainTextEdit->resize(1000,2000);
}

void ThreadForm::load(QJsonObject &p){
    //set post number
    post = new Post(p);
    ui->no->setPlainText(post->no);

    //set comment
    //TODO replace <span class="quote"> and <a href="url">
    ui->com->setHtml(post->com);

    //set subject
    ui->sub->setPlainText(htmlParse(post->sub));

    //set name
    ui->name->setText(post->name);

    //set image
    //TODO clean if-else's
    if(!post->tim.isNull()){
        imgURL = this->board % "/" % post->tim % post->ext;
        filePath = pathBase%post->no%"-"%post->filename%post->ext;
        file = new QFile(filePath);
        if(post->ext == QLatin1String(".jpg") || post->ext == QLatin1String(".png")){
            loadIt = true;
            if(!file->exists()){
                qDebug() << QString("getting https://i.4cdn.org/")  % imgURL;
                replyImage = nc.manager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % imgURL)));
                connectionImage = connect(replyImage, &QNetworkReply::finished,this, &ThreadForm::getOrigFinished);
            }
            else{
                loadImage(filePath);
            }
        }
        else {
            loadIt = false;
            if(!file->exists()){
                qDebug() << QString("getting https://i.4cdn.org/")  % imgURL;
                replyImage = nc.manager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % imgURL)));
                connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::getOrigFinished);
            }
            imgURL = this->board % "/" % post->tim % "s.jpg";
            thumbPath = pathBase%"thumbs/"%post->no%"-"%post->filename%"s.jpg";
            thumb = new QFile(thumbPath);
            if(!thumb->exists()){
                qDebug() << QString("getting https://i.4cdn.org/")  % imgURL;
                replyThumb = nc.manager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % imgURL)));
                connectionThumb = connect(replyThumb, &QNetworkReply::finished,this,&ThreadForm::getThumbFinished);
            }
            else{
                loadImage(thumbPath);
            }
        }
        connect(ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
    }
    else{
        ui->verticalLayout->deleteLater();
    }
    this->show();
    updateComHeight();
}

void ThreadForm::updateComHeight(){
    //ui->com->setMinimumHeight(ui->com->document()->size().height());
    int docHeight = ui->com->document()->size().height();
    int newHeight = docHeight+ui->sub->height()+ui->verticalLayout_2->BottomToTop;
    if(newHeight > this->height()){
        if(type == PostType::Reply){
            this->setFixedHeight(newHeight);
        }
        else if(newHeight < 500){
            ui->com->setMinimumHeight(docHeight);
            this->setFixedHeight(newHeight);
        }
        else{
            this->setFixedHeight(500);
        }
    }
}

void ThreadForm::getOrigFinished(){
    if(replyImage->error() == 0)
    {
        file->open(QIODevice::WriteOnly);
        file->write(replyImage->readAll());
        file->close();
        qDebug() << "saved file "+filePath;
        if(loadIt) loadImage(filePath);
    }
        replyImage->deleteLater();
        disconnect(connectionImage);
}

void ThreadForm::getThumbFinished(){
    if(replyThumb->error() == 0)
    {
        thumb->open(QIODevice::WriteOnly);
        thumb->write(replyThumb->readAll());
        thumb->close();
        qDebug() << "saved file "+thumbPath;
        loadImage(thumbPath);
    }
    replyThumb->deleteLater();
    disconnect(connectionThumb);

}

void ThreadForm::loadImage(QString path){
    int scale = 250;
    QPixmap pic(path);
    QPixmap scaled = (pic.height() > pic.width()) ?
             pic.scaledToHeight(scale, Qt::SmoothTransformation) :
             pic.scaledToWidth(scale, Qt::SmoothTransformation);
    ui->tim->setPixmap(scaled);
    ui->tim->setMaximumSize(scaled.size());
    if(scaled.height() > this->height()){
        this->setFixedHeight(scaled.height());
    }
}

void ThreadForm::imageClicked(){
    qDebug() << "clicked "+post->filename;
    qDebug() << ui->tim->width();
    (this->type == PostType::Reply) ? openImage() : mw->onNewThread(this,board,threadNum);
}

void ThreadForm::openImage(){
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir().absoluteFilePath(filePath)));
}

QString ThreadForm::htmlParse(QString &html){
    return html.replace("<br>","\n").replace("&amp;","&")
            .replace("&gt;",">").replace("&lt;","<")
            .replace("&quot","\"").replace("&#039;","'")
            .replace("<wb>","").replace("<wbr>","");
}
