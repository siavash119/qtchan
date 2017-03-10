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
    delete post;
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
    if(!post->tim.isNull()){
        QString img;
        if(post->ext == QLatin1String(".jpg") || post->ext == QLatin1String(".png")){
            img = "https://i.4cdn.org/" % this->board % "/" % post->tim % post->ext;
        }
        else {
            img = "https://i.4cdn.org/" % this->board % "/" % post->tim % "s.jpg";
        }
        qDebug() << QString("getting ") % img;
        replyImage = nc.manager->get(QNetworkRequest(QUrl(img)));
        connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::getImageFinished);
        (this->type == Thread) && connect(ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
    }
    else{
        ui->tim->close();
    }
    this->show();
    this->setFixedHeight(ui->com->document()->size().height()+ui->sub->height()+ui->verticalLayout_2->spacing());
}

void ThreadForm::updateComHeight(){
    int newHeight = ui->com->document()->size().height()+ui->sub->height()+ui->verticalLayout_2->BottomToTop;
    if(newHeight > this->height()){
        this->setFixedHeight(newHeight);
    }
}

void ThreadForm::getImageFinished(){
    if(replyImage->error() == 0)
    {
        int scale = 250;
        QPixmap pic;
        pic.loadFromData(replyImage->readAll());
        QPixmap scaled = (pic.height() > pic.width()) ?
                 pic.scaledToHeight(scale, Qt::SmoothTransformation) :
                 pic.scaledToWidth(scale, Qt::SmoothTransformation);
        ui->tim->setPixmap(scaled);
        if(scaled.height() > this->height()){
            this->setFixedHeight(scaled.height());
        }
    }
        replyImage->deleteLater();
        disconnect(connectionImage);
}

void ThreadForm::imageClicked(){
    mw->onNewThread(this,board,threadNum);
}

QString ThreadForm::htmlParse(QString &html){
    return html.replace("<br>","\n").replace("&amp;","&")
            .replace("&gt;",">").replace("&lt;","<")
            .replace("&quot","\"").replace("&#039;","'")
            .replace("<wb>","").replace("<wbr>","");
}
