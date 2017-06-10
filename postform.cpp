#include "postform.h"
#include "ui_postform.h"
#include <QUrl>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QTimer>
#include <QFileDialog>
#include <QRegularExpression>
#include <QGraphicsItem>
#include <QShortcut>
#include <iostream>
#include <QGraphicsEffect>
#include "netcontroller.h"

PostForm::PostForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PostForm)
{
    ui->setupUi(this);
    ui->cancel->hide();
    this->setObjectName("PostForm");
    ui->com->setFocus();
    ui->com->installEventFilter(this);
    ui->browse->installEventFilter(this);
    submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
    setShortcuts();
}

void PostForm::load(QString &board, QString thread){
    this->board = board;
    this->thread = thread;
    this->setWindowTitle("post to /" + board + "/" + thread);
}

PostForm::~PostForm()
{
    delete ui;
}

void PostForm::setShortcuts(){
    //override application shortcuts
    //new QShortcut(QKeySequence::NextChild,this);
    //new QShortcut(QKeySequence("Ctrl+Shift+Tab"),this);
    new QShortcut(QKeySequence::Delete,this);
    new QShortcut(Qt::Key_R,this);

    //rest in the eventfilter
}

void PostForm::appendText(QString &text){
    ui->com->textCursor().insertText(text);
}

void PostForm::postIt(){
    addOverlay();
    disconnect(submitConnection);
    this->removeEventFilter(this);
    qDebug().noquote() << "posting";
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart mode;
    mode.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"mode\""));
    mode.setBody("regist");
    QHttpPart resto;
    resto.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"resto\""));
    resto.setBody(thread.toStdString().c_str());
    QHttpPart name;
    name.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"name\""));
    name.setBody(ui->name->toPlainText().toStdString().c_str());
    QHttpPart email;
    email.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
    email.setBody(ui->email->toPlainText().toStdString().c_str());
    QHttpPart com;
    com.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"com\""));
    qDebug().noquote() << ui->com->toPlainText().toStdString().c_str();
    com.setBody(ui->com->toPlainText().toStdString().c_str());
    /*QHttpPart recaptcha;
    com.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"g-recaptcha-response\""));
    com.setBody("");*/

    multiPart->append(mode);
    multiPart->append(resto);
    multiPart->append(name);
    multiPart->append(email);
    multiPart->append(com);
    //multiPart->append(recaptcha);

    if(filename != ""){
        QHttpPart uploadFile;
        uploadFile.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
        uploadFile.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"upfile\"; filename=\""+filename+"\""));
        QFile *upQFile = new QFile(filename);
        upQFile->open(QIODevice::ReadOnly);
        uploadFile.setBodyDevice(upQFile);
        upQFile->setParent(multiPart);
        multiPart->append(uploadFile);
    }

    QUrl url = QUrl("https://sys.4chan.org/"+board+"/post");
    QNetworkRequest request(url);
    //request.1
    postReply = nc.jsonManager->post(request, multiPart);
    connect(postReply, &QNetworkReply::finished, this, &PostForm::postFinished);
    multiPart->setParent(postReply); // delete the multiPart with the reply
    // here connect signals etc.
}

void PostForm::postFinished(){
    QTextEdit reply;
    reply.setWindowTitle("post to /"+board+"/"+thread+" response");
    //reply.setMinimumSize(640,480);
    QByteArray temp = postReply->readAll();
    reply.setHtml(temp);
    reply.setGeometry(0,0,this->width(),this->height());
    qDebug().noquote() << temp;
    if(reply.toPlainText().contains(QRegularExpression("uploaded.$|Post successful!$")))
    {
        overlay->displayText = "Post successful!";
        this->ui->com->clear();
        QTimer::singleShot(1000, this, &PostForm::close);
        //QTimer::singleShot(1000, reply, &QTextEdit::close);
    }
    else{
        reply.show();
    }
    QTimer::singleShot(1000, this,&PostForm::removeOverlay);
    this->installEventFilter(this);
    submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
}

void PostForm::addOverlay(){
    qDebug().noquote() << "adding overlay";
    overlay = new Overlay(this);
    overlay->setObjectName("overlay");
    overlay->setParent(this);
    overlay->show();
    focused = this->focusWidget();
    ui->com->setFocusPolicy(Qt::NoFocus);
    ui->filename->setFocus();
}

void PostForm::removeOverlay(){
    qDebug().noquote() << "removing overlay";
    delete overlay;
    ui->com->setFocusPolicy(Qt::StrongFocus);
    focused->setFocus();
}



bool PostForm::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int mod = keyEvent->modifiers();
        int key = keyEvent->key();
        //qDebug("Ate key press %d", key);
        //qDebug("Ate modifier press %d", mod);
        //shift+enter to post
        if(mod == 33554432 && key == 16777220){
            postIt();
            return true;
        }
        if(key==53){
            addOverlay();
            QTimer::singleShot(1000, this, &PostForm::removeOverlay);
            //qDebug() << filename;
        }
        return QObject::eventFilter(obj, event);
    }
    if(event->type() == QEvent::DragEnter){
        static_cast<QDragEnterEvent*>(event)->acceptProposedAction();
        return true;
    }

    if(event->type() == QEvent::Drop){
            const QMimeData *mimeData = static_cast<QDropEvent*>(event)->mimeData();
            fileChecker(mimeData);
            qDebug().noquote() << "DROPPED";
            return true;
    }
    return QObject::eventFilter(obj, event);
}

void PostForm::fileChecker(const QMimeData *mimeData){
    qDebug().noquote() << "checking file";
   if (mimeData->hasImage()) {
       //ui->label->setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
   } else if (mimeData->hasHtml()) {
       ui->filename->setText(mimeData->text());
       //setTextFormat(Qt::RichText);
   } else if (mimeData->hasText()) {
       qDebug().noquote() << mimeData->text();
       filename = mimeData->text().mid(7); //remove file://
       filename.remove(QRegularExpression("[\\n\\t\\r]"));
       ui->filename->setText(filename);
   } else if (mimeData->hasUrls()) {
       QList<QUrl> urlList = mimeData->urls();
       QString text;
       for (int i = 0; i < urlList.size() && i < 32; ++i)
           text += urlList.at(i).path() + QLatin1Char('\n');
       ui->filename->setText(text);
   } else {
       ui->filename->setText("Cannot display data");
   }
   qDebug().noquote() << filename;
   ui->cancel->show();
}


void PostForm::on_browse_clicked()
{
    dialog = new QFileDialog(this);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->show();
    connect(dialog,&QFileDialog::fileSelected,this,&PostForm::fileSelected);
}

void PostForm::fileSelected(const QString &file){
    filename=file;
    //qDebug() << dialog->;
    //qDebug() << dialog->getOpenFileName();
    if(filename == ""){
        ui->filename->setText("No file selected");
        ui->cancel->hide();
    }
    else{
        ui->filename->setText(file);
        ui->cancel->show();
    }
    dialog->close();
    qDebug().noquote() << filename;
}

void PostForm::on_cancel_clicked()
{
    filename="";
    ui->filename->setText("No file selected");
    ui->cancel->hide();
}

void PostForm::droppedItem(){
    qDebug().noquote() << "dropped!";
}
