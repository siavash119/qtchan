#include "threadform.h"
#include "ui_threadform.h"
#include <QPixmap>
#include <QImageReader>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringBuilder>
#include <QDesktopServices>
#include <QSettings>
#include <QStringList>
#include <iostream>
#include <QListIterator>
#include "netcontroller.h"
#include "mainwindow.h"
#include "threadtab.h"

//TODO Possibly refactor file checks and pointers to dir and file objects
//TODO Possibly decouple the file and thumb getters to the post class
ThreadForm::ThreadForm(QString board, QString threadNum, PostType type, bool root, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadForm)
{
    this->board = board;
    this->threadNum = threadNum;
    this->type = type;
    this->root = root;
    ui->setupUi(this);
    ui->tim->hide();
    ui->horizontalSpacer->changeSize(0,0);
    ui->replies->hide();
    this->setMinimumWidth(488);
    pathBase = "./"%board%"/" % ((type == PostType::Reply) ? threadNum : "index") % "/";
    connect(ui->hide,&ClickableLabel::clicked,this,&ThreadForm::hideClicked);
    connect(ui->com,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
    connect(ui->replies,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
    this->tab = parent;
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
    post = new Post(p,board);
    ui->no->setText(post->no);

    //set comment
    //TODO replace <span class="quote"> and <a href="url">
    ui->com->setText(post->com);
    quotelinks = Filter::findQuotes(post->com);

    //set subject
    ui->sub->setText(htmlParse(post->sub));

    //set name
    ui->name->setText(post->name);

    //set image
    //TODO clean if-else's
    //TODO possibly change file pointer
    //TODO use filedeleted image
    if(!post->tim.isNull() && !post->filedeleted){
        fileURL = this->board % "/" % post->tim % post->ext;
        filePath = pathBase%post->no%"-"%post->filename%post->ext;
        file = new QFile(filePath);
        QSettings settings;
        if((post->ext == QLatin1String(".jpg") || post->ext == QLatin1String(".png"))){
            //ui->horizontalSpacer->1
            loadIt = true;
            if(!file->exists()){
                if(settings.value("loadorig") == 1) getFile();
                else{
                    thumbURL = this->board % "/" % post->tim % "s.jpg";
                    thumbPath = pathBase%"thumbs/"%post->no%"-"%post->filename%"s.jpg";
                    thumb = new QFile(thumbPath);
                    if(!thumb->exists()) getThumb();
                    else loadImage(thumbPath);
                }
            }
            else{
                loadImage(filePath);
            }
        }
        else {
            loadIt = false;
            if(settings.value("loadorig")==1){
                if(!file->exists()){
                    getFile();
                }
            }
            thumbURL = this->board % "/" % post->tim % "s.jpg";
            thumbPath = pathBase%"thumbs/"%post->no%"-"%post->filename%"s.jpg";
            thumb = new QFile(thumbPath);
            if(!thumb->exists()) getThumb();
            else loadImage(thumbPath);
        }
        connect(ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
    }
    else{
        ui->pictureLayout->deleteLater();
    }
    this->show();
    //updateComHeight();
}

void ThreadForm::getFile(){
    qDebug() << QString("getting https://i.4cdn.org/")  % fileURL;
    replyImage = nc.manager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % fileURL)));
    gettingFile = true;
    connectionImage = connect(replyImage, &QNetworkReply::finished,this, &ThreadForm::getOrigFinished);
}

void ThreadForm::getThumb(){
    qDebug() << QString("getting https://i.4cdn.org/")  % thumbURL;
    replyThumb = nc.manager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % thumbURL)));
    connectionThumb = connect(replyThumb, &QNetworkReply::finished,this,&ThreadForm::getThumbFinished);
}

//TODO avoid copy pasted function
//TODO clean if-elses
void ThreadForm::loadOrig(){
    if(!post->tim.isNull()){
        if((post->ext == QLatin1String(".jpg") || post->ext == QLatin1String(".png"))){
            loadIt = true;
            if(!file->exists()) getFile();
            else loadImage(filePath);
        }
        else {
            loadIt = false;
            if(!file->exists()) getFile();
        }
    }
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
    gettingFile = false;
    if(replyImage->error() == 0)
    {
        file->open(QIODevice::WriteOnly);
        file->write(replyImage->readAll());
        file->close();
        qDebug() << "saved file "+filePath;
        if(loadIt){
            loadImage(filePath);
            QListIterator<ThreadForm*> i(clones);
            while(i.hasNext()){
                ThreadForm* cloned = i.next();
                cloned->loadImage(cloned->filePath);
            }
        }
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
    ui->tim->show();
    ui->horizontalSpacer->changeSize(250,0);
    ui->horizontalSpacer->invalidate();
    this->setMinimumWidth(738);
    ui->tim->setPixmap(scaled);
    ui->tim->setMaximumSize(scaled.size());
}

void ThreadForm::imageClicked(){
    qDebug() << "clicked "+post->filename;
    if(this->type == PostType::Reply){
        if(gettingFile){
            connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::openImage);
        }
        else if(!file->exists()){
            qDebug() << QString("getting https://i.4cdn.org/")  % fileURL;
            gettingFile=true;
            replyImage = nc.manager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % fileURL)));
            //connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::loadFromImageClicked);
            connectionImage = connect(replyImage, &QNetworkReply::finished,[=]() {
              this->getOrigFinished();
              this->openImage();
            });
        }
        else openImage();
    }
    else{
        mw->onNewThread(this,board,threadNum);
    }
}

void ThreadForm::hideClicked(){
    this->hide();
    QSettings settings;
    QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
    idFilters.append(threadNum);
    settings.setValue("filters/"+board+"/id",idFilters);
    qDebug() << "hide Clicked so "+threadNum+" filtered!";
    QListIterator<ThreadForm*> i(clones);
    while(i.hasNext()){
        i.next()->deleteLater();
    }
    QSet<QString> quotes = quotelinks;
    ThreadForm* replyTo;
    foreach (const QString &orig, quotes)
    {
        replyTo = ((ThreadTab*)tab)->tfMap.find(orig).value();
        if(replyTo != nullptr){
            //replyTo->replies.insert(tf->post->no);
            replyTo->replies.remove(post->no.toDouble());
            replyTo->setReplies();
        }
    }
    this->hidden = true;
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
    if(link.at(0)=='#' && this->type == PostType::Reply){
        ThreadForm* tf = ((ThreadTab*)tab)->findPost(link.mid(2));
        if(tf != nullptr && !tf->hidden) this->insert(tf);
    }
    else if(link.at(0)=='/'){
        mw->loadFromSearch(link,false);
    }
}

void ThreadForm::on_replies_linkHovered(const QString &link)
{
    //qDebug() << link;
    if(this->type == PostType::Reply){
        QRegularExpression postLink("#p(\\d+)");
        QRegularExpressionMatch match = postLink.match(link);
        //TODO hover new threadform
        if (match.hasMatch()) {
            //emit searchPost(ui->com->textCursor().position(),match.captured(1));
            //emit searchPost(match.captured(1),this);
        }
    }
}

void ThreadForm::insert(ThreadForm* tf){
    ThreadForm *newtf = tf->clone();
    ui->quotes->addWidget(newtf);
    newtf->show();
    //newtf->setMinimumWidth(newtf->sizeHint().width());
    //this->setMinimumWidth(this->sizeHint().width());
    //((ThreadTab*)tab)->updateWidth();
    //this->update();
    //((ThreadTab*)tab)->updateWidth();
}

ThreadForm* ThreadForm::clone(){
    ThreadForm* tfs = new ThreadForm(this->board,this->threadNum,this->type,false);
    tfs->tab = tab;
    tfs->post = this->post;
    tfs->ui->no->setText(post->no);
    tfs->ui->com->setText(post->com);
    tfs->ui->sub->setText(post->sub);
    tfs->ui->name->setText(post->name);
    if(this->board != "pol"){
        tfs->ui->country_name->hide();
    }
    else{
        tfs->ui->country_name->setText(post->country_name);
    }
    tfs->replies = replies;
    //TODO check and account for if original is still getting file
    if(!post->tim.isNull() && !post->filedeleted){
        tfs->fileURL = fileURL;
        tfs->filePath = filePath;
        tfs->file = file;
        tfs->thumbURL = thumbURL;
        tfs->thumbPath = thumbPath;
        tfs->thumb = thumb;
        if(post->ext == QLatin1String(".jpg") || post->ext == QLatin1String(".png")){
            tfs->loadIt = true;
            if(file->exists() && tfs->loadIt)tfs->loadImage(tfs->filePath);
            else tfs->loadImage(tfs->thumbPath);
        }
        else {
            tfs->loadIt = false;
            tfs->loadImage(tfs->thumbPath);
        }
        connect(tfs->ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
    }
    else
        tfs->ui->pictureLayout->deleteLater();
    tfs->setReplies();
    this->clones.append(tfs);
    disconnect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::hideClicked);
    //connect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::deleteLater);
    connect(tfs->ui->hide,&ClickableLabel::clicked,[=](){
        tfs->hide();
        tfs->deleteLater();
    });
    connect(tfs,&QObject::destroyed,[=](){this->clones.removeOne(tfs);});
    return tfs;
}

void ThreadForm::setReplies(){
    QString repliesString;
    QList<QString> list = replies.values();
    if(list.length()){
        ui->replies->show();
        foreach (const QString &reply, list)
        {
            repliesString+=" <a href=\"#p" % reply % "\">>>" % reply % "</a>";
        }
        repliesString = repliesString.mid(1);
        ui->replies->setText(repliesString);
        QListIterator<ThreadForm*> i(clones);
        while(i.hasNext()){
            i.next()->ui->replies->setText(repliesString);
        }
    }
    else{
        ui->replies->hide();
    }
}
