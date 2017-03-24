#include "postform.h"
#include "ui_postform.h"
#include <QUrl>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include "netcontroller.h"
#include <iostream>
#include <QTimer>
#include <QFileDialog>
#include <QRegularExpression>
#include <QGraphicsItem>

PostForm::PostForm(QString board, QString thread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PostForm)
{
    ui->setupUi(this);
    ui->cancel->hide();
    this->board = board;
    this->thread = thread;
    this->setObjectName("PostForm");
    this->setWindowTitle("post to /" + board + "/" + thread);
    ui->com->setFocus();
    filename="";
    ui->com->installEventFilter(this);
    ui->browse->installEventFilter(this);
    //ui->browse->installEventFilter(this);
    submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
    //connect(this,&PostForm::dropEvent,this,&PostForm::droppedItem);
}

PostForm::~PostForm()
{
    delete ui;
}

void PostForm::postIt(){
    disconnect(submitConnection);
    this->removeEventFilter(this);
    qDebug() << "posting";
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
    qDebug() << ui->com->toPlainText().toStdString().c_str();
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
    postReply = nc.manager->post(request, multiPart);
    connect(postReply, &QNetworkReply::finished, this, &PostForm::postFinished);
    multiPart->setParent(postReply); // delete the multiPart with the reply
    // here connect signals etc.
}

void PostForm::postFinished(){
    QTextEdit *reply = new QTextEdit();
    reply->setWindowTitle("post to /"+board+"/"+thread+" response");
    reply->setMinimumSize(640,480);
    QByteArray temp = postReply->readAll();
    reply->setHtml(temp);
    qDebug() << temp;
    reply->show();
    if(reply->toPlainText().contains(QRegularExpression("uploaded.$|Post successful!$")))
    {
        QTimer::singleShot(1000, this, &PostForm::close);
        QTimer::singleShot(1000, reply, &QTextEdit::close);
    }
    else{
        this->installEventFilter(this);
        submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
    }
}

bool PostForm::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int mod = keyEvent->modifiers();
        int key = keyEvent->key();
        qDebug("Ate key press %d", key);
        if(mod == 33554432 && key == 16777220){
            postIt();
            return true;
        }
        if(key==53){
            qDebug() << filename;
        }
        return QObject::eventFilter(obj, event);
    }
    if(event->type() == QEvent::DragEnter){
        ((QDragEnterEvent*)event)->acceptProposedAction();
        return true;
    }

    if(event->type() == QEvent::Drop){
            const QMimeData *mimeData = ((QDropEvent*)event)->mimeData();
            fileChecker(mimeData);
            qDebug() << "DROPPED";
            return true;
    }
    return QObject::eventFilter(obj, event);
}

void PostForm::fileChecker(const QMimeData *mimeData){
    qDebug()<< "checking file";
   if (mimeData->hasImage()) {
       //ui->label->setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
   } else if (mimeData->hasHtml()) {
       ui->com->setPlainText(mimeData->text());
       //setTextFormat(Qt::RichText);
   } else if (mimeData->hasText()) {
       qDebug() << mimeData->text();
       filename = mimeData->text().mid(7);
       filename.remove(QRegExp("[\\n\\t\\r]"));
       ui->filename->setText(filename);
       ui->cancel->show();
   } else if (mimeData->hasUrls()) {
       QList<QUrl> urlList = mimeData->urls();
       QString text;
       for (int i = 0; i < urlList.size() && i < 32; ++i)
           text += urlList.at(i).path() + QLatin1Char('\n');
       ui->com->setPlainText(text);
   } else {
       ui->com->setPlainText("Cannot display data");
   }
   qDebug() << filename;
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
    qDebug() << filename;
}

void PostForm::on_cancel_clicked()
{
    filename="";
    ui->filename->setText("No file selected");
    ui->cancel->hide();
}

void PostForm::droppedItem(){
    qDebug() << "dropped!";
}
