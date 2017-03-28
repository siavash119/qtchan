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
#include <QKeyEvent>
#include "mainwindow.h"
#include <QMutableMapIterator>

ThreadTab::ThreadTab(QString board, QString thread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadTab)
{
    this->updated = false;
    ui->setupUi(this);
    this->board = board;
    this->thread = thread;
    //QDir().mkpath(board+"/"+thread);
    QDir().mkpath(board+"/"+thread+"/thumbs");
    threadUrl = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
    request = QNetworkRequest(QUrl(threadUrl));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    reply = nc.manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ThreadTab::loadPosts);
    myProcess = new QProcess(parent);
    myPostForm = new PostForm(board,thread);
    this->setShortcuts();
}

void ThreadTab::setShortcuts(){
    QAction *foo = new QAction(this);
    foo->setShortcut(Qt::Key_G);
    connect(foo, &QAction::triggered, this, &ThreadTab::gallery);
    this->addAction(foo);
    QAction *postForm = new QAction(this);
    postForm->setShortcut(Qt::Key_Q);
    connect(postForm, &QAction::triggered, this, &ThreadTab::openPostForm);
    this->addAction(postForm);
    QAction *expandAll = new QAction(this);
    expandAll->setShortcut(Qt::Key_E);
    connect(expandAll, &QAction::triggered, this, &ThreadTab::loadAllImages);
    this->addAction(expandAll);
    QAction *refresh = new QAction(this);
    refresh->setShortcut(Qt::Key_R);
    connect(refresh, &QAction::triggered, this, &ThreadTab::getPosts);
    this->addAction(refresh);
    QAction *focusSearch = new QAction(this);
    focusSearch->setShortcut(Qt::ControlModifier+Qt::Key_F);
    connect(focusSearch,&QAction::triggered,this,&ThreadTab::focusIt);
    this->addAction(focusSearch);
    QAction *focusTree = new QAction(this);
    focusTree->setShortcut(Qt::Key_F3);
    connect(focusTree,&QAction::triggered,mw,&MainWindow::focusTree);
    this->addAction(focusTree);
    QAction *focusBar = new QAction(this);
    focusBar->setShortcut(Qt::Key_F6);
    connect(focusBar,&QAction::triggered,mw,&MainWindow::focusBar);
    this->addAction(focusBar);
}

void ThreadTab::getPosts(){
    qDebug() << "refreshing /" + board + "/" + thread;
    reply = nc.manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ThreadTab::loadPosts);
}

ThreadTab::~ThreadTab()
{
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        ((ThreadForm*)mapI.value())->deleteLater();
        mapI.remove();
    }
    delete ui;
}

void ThreadTab::openPostForm(){
    myPostForm->show();
    myPostForm->activateWindow();
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
    for(int i=tfMap.size();i<length;i++){
        QJsonObject p = posts.at(i).toObject();
        ThreadForm *tf = new ThreadForm(board,thread);
        ui->threads->addWidget(tf);
        connect(tf,&ThreadForm::searchPost,this,&ThreadTab::findPost);
        tf->load(p);
        tfMap.insert(tf->post->no,tf);
        QSet<QString> quotes = tf->quotelinks;
        ThreadForm* replyTo;
        foreach (const QString &orig, quotes)
        {
            replyTo = tfMap.find(orig).value();
            if(replyTo != nullptr){
                replyTo->replies.insert(tf->post->no);
                replyTo->setReplies();
            }
        }
    }
    ui->threads->addStretch(1);
    reply->deleteLater();
}

void ThreadTab::updatePosts(){
    updated = false;
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        //cout << i.key() << ": " << i.value() << endl;
        ((ThreadForm*)mapI.value())->updateComHeight();
        //mapI.remove();
    }
}

void ThreadTab::loadAllImages(){
    updated = false;
    QMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        //cout << i.key() << ": " << i.value() << endl;
        ((ThreadForm*)mapI.value())->loadOrig();
        //mapI.remove();
    }
}

void ThreadTab::findPost(QString postNum, ThreadForm* thetf){
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    ThreadForm *tf;
    while (mapI.hasNext()) {
        mapI.next();
        tf = mapI.value();
        if(tf->post->no == postNum)
            thetf->insert(tf);
    }
}

void ThreadTab::findText(const QString text){
    QRegularExpression re(text,QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    ThreadForm* tf;
    bool pass = false;
    if (text == "") pass = true;
    qDebug() << "searching " + text;
    QMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        tf = mapI.value();
        qDebug() << tf->post->com;
        if(pass) { tf->show(); continue;};
        match = re.match(tf->post->com);
        if(!match.hasMatch()){
            tf->hide();
        }
    }
}

void ThreadTab::on_pushButton_clicked()
{
    findText(ui->lineEdit->text());
}

bool ThreadTab::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int mod = keyEvent->modifiers();
        int key = keyEvent->key();
        qDebug("Ate modifier %d",mod);
        qDebug("Ate key press %d", key);
        qDebug() << obj->objectName();
        if(mod == 67108864 && key == 70){
            //ui->lineEdit->setFocus();
        }
        qDebug("Ate key press %d", key);
        if(key == 16777220){
            on_pushButton_clicked();
        }
        else if(key == 16777269){
            ui->lineEdit->setFocus();
        }
        else{
            return QObject::eventFilter(obj, event);
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void ThreadTab::focusIt(){
    ui->lineEdit->setFocus();
}

void ThreadTab::on_lineEdit_returnPressed()
{
    findText(ui->lineEdit->text());
}
