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
    //com = htmlParse(com);
    ui->com->setHtml(com);
    //ui->com->setPlainText(com);

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
        connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::getImageFinished);
        (this->type == Thread) && connect(ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
    }
    else{
        ui->tim->close();
    }
    this->show();
    qDebug()<< ui->verticalLayout_2->BottomToTop;
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
