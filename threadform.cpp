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
#include <QSettings>
#include <QStringList>
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
    connect(ui->hide,&ClickableLabel::clicked,this,&ThreadForm::hideClicked);
    connect(ui->com,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
    connect(ui->replies,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
}

ThreadForm::~ThreadForm()
{
    delete ui;
}

void ThreadForm::setText(QString text){
    ui->com->setText(text);
}

void ThreadForm::load(QJsonObject &p){
    //set post number
    post = new Post(p);
    ui->no->setText(post->no);

    //set comment
    //TODO replace <span class="quote"> and <a href="url">
    ui->com->setText(post->com);
    quotelinks = nc.filter->findQuotes(post->com);

    //set subject
    ui->sub->setText(htmlParse(post->sub));

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
        ui->pictureLayout->deleteLater();
    }
    this->show();
    //updateComHeight();
}

void ThreadForm::updateComHeight(){
    //ui->scrollArea->height()
    /*const QSize newSize = ui->com->sizeHint();
    const QSize oldSize = ui->scrollArea->size();
    //ui->scrollArea->setMaximumHeight(newSize.height());
    //ui->com->setMinimumSize(newSize);
    if(newSize.height() > oldSize.height()){
        if(type == PostType::Thread){
            if(newSize.height() > 500) ui->scrollArea->setFixedHeight(500);
            else ui->scrollArea->setFixedHeight(newSize.height());
        }
        else ui->scrollArea->setFixedHeight(newSize.height());
    }*/
    //int docHeight = ui->com->document()->size().height();
    /*int docHeight = ui->com->height();
    int newHeight = docHeight+ui->sub->height()+ui->verticalLayout_2->BottomToTop;
    if(newHeight > this->height()){
        if(type == PostType::Reply){
            ui->com->setMinimumHeight(docHeight);
            this->setFixedHeight(newHeight);
            this->setMaximumHeight(newHeight);
        }
        else if(newHeight < 500){
            ui->com->setMinimumHeight(docHeight);
            this->setFixedHeight(newHeight);
            this->setMaximumHeight(newHeight);
        }
        else{
            this->setFixedHeight(500);
            this->setMaximumHeight(500);
        }
    }*/
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
    /*QPixmap scaled = QPixmap(path).scaled(post->tn_w,
                                          post->tn_h,
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);*/
    int scale = 250;
    QPixmap pic(path);
    QPixmap scaled = (pic.height() > pic.width()) ?
             pic.scaledToHeight(scale, Qt::SmoothTransformation) :
             pic.scaledToWidth(scale, Qt::SmoothTransformation);
    ui->tim->setPixmap(scaled);
    //ui->tim->setFixedSize(post->tn_w,post->tn_h);
    ui->tim->setMaximumSize(scaled.size());
    /*if(scaled.height() > this->height()){
        this->setFixedHeight(scaled.height());
    }*/
}

void ThreadForm::imageClicked(){
    qDebug() << "clicked "+post->filename;
    (this->type == PostType::Reply) ? openImage() : mw->onNewThread(this,board,threadNum);
}

void ThreadForm::hideClicked(){
    QSettings settings;
    QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
    idFilters.append(threadNum);
    settings.setValue("filters/"+board+"/id",idFilters);
    qDebug() << "hide Clicked so "+threadNum+" filtered!";
    this->close();
}

void ThreadForm::openImage(){
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir().absoluteFilePath(filePath)));
}

QString ThreadForm::htmlParse(QString &html){
    return html.replace("<br>","\n").replace("&amp;","&")
            .replace("&gt;",">").replace("&lt;","<")
            .replace("&quot","\"").replace("&#039;","'")
            .replace("<wb>","\n").replace("<wbr>","\n");
}

void ThreadForm::quoteClicked(const QString &link)
{
    qDebug() << link;
    QRegularExpression postLink("#p(\\d+)");
    QRegularExpressionMatch match = postLink.match(link);
    if (match.hasMatch()) {
        //emit searchPost(ui->com->textCursor().position(),match.captured(1));
        emit searchPost(match.captured(1),this);
    }
}

void ThreadForm::onSearchPost(const QString &link, ThreadForm* thetf){
    emit searchPost(link,thetf);
}

void ThreadForm::insert(ThreadForm* tf){
    ThreadForm *newtf = tf->clone();
    ui->quotes->addWidget(newtf);
    newtf->show();
    this->update();
}

ThreadForm* ThreadForm::clone(){
    ThreadForm* tfs = new ThreadForm(this->board,this->threadNum,this->type);
    tfs->ui->no->setText(post->no);
    tfs->ui->com->setText(post->com);
    tfs->ui->sub->setText(post->sub);
    tfs->ui->name->setText(post->name);
    if(!post->tim.isNull()){
        tfs->filePath = pathBase%post->no%"-"%post->filename%post->ext;
        tfs->file = new QFile(filePath);
        if(post->ext == QLatin1String(".jpg") || post->ext == QLatin1String(".png")){
            loadIt = true;
            tfs->loadImage(filePath);
        }
        else {
            loadIt = false;
            tfs->imgURL = this->board % "/" % post->tim % "s.jpg";
            tfs->thumbPath = pathBase%"thumbs/"%post->no%"-"%post->filename%"s.jpg";
            tfs->thumb = new QFile(thumbPath);
            tfs->loadImage(thumbPath);
        }
        connect(tfs->ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
    }
    else
        tfs->ui->pictureLayout->deleteLater();
    disconnect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::hideClicked);
    connect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::deleteLater);
    connect(tfs,&ThreadForm::searchPost,this,&ThreadForm::onSearchPost);
     //       connect(tf,&ThreadForm::searchPost,this,&ThreadTab::findPost);
    //tfs->ui->com->setFixedHeight(this->ui->com->height());
    //qDebug() << this->height();
    //tfs->setFixedHeight(this->height());
    return tfs;
}

void ThreadForm::setReplies(){
    QString repliesString;
    foreach (const QString &reply, replies)
    {
        repliesString+="<a href=\"#p" % reply % "\">>>" % reply % "</a> ";
    }
    ui->replies->setText(repliesString.mid(0,repliesString.length()-1));
}
