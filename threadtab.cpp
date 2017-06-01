#include "threadtab.h"
#include "ui_threadtab.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>
#include <QKeyEvent>
#include <QMutableMapIterator>
#include <QProcess>
#include "netcontroller.h"
#include "mainwindow.h"
#include "threadform.h"

ThreadTab::ThreadTab(QString board, QString thread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadTab)
{
    this->updated = false;
    ui->setupUi(this);
    this->board = board;
    this->thread = thread;
    this->setWindowTitle("/"+board+"/"+thread);
    //QDir().mkpath(board+"/"+thread);
    QDir().mkpath(board+"/"+thread+"/thumbs");
    threadUrl = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
    qDebug() << threadUrl;
    request = QNetworkRequest(QUrl(threadUrl));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    reply = nc.jsonManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ThreadTab::loadPosts);
    //myProcess = new QProcess(parent);
    myPostForm = new PostForm(board,thread);
    this->setShortcuts();
    this->installEventFilter(this);
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
    refresh->setShortcutContext(Qt::ApplicationShortcut);
    connect(refresh, &QAction::triggered, this, &ThreadTab::getPosts);
    this->addAction(refresh);
    QAction *focusSearch = new QAction(this);
    focusSearch->setShortcut(QKeySequence("Ctrl+f"));
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
    qDebug().noquote() << "refreshing /" + board + "/" + thread;
    reply = nc.manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ThreadTab::loadPosts);
}

ThreadTab::~ThreadTab()
{
    //QCoreApplication::processEvents();
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        delete ((ThreadForm*)mapI.value());
        mapI.remove();
        //QCoreApplication::processEvents();
    }
    delete ui;
}

void ThreadTab::openPostForm(){
    myPostForm->show();
    myPostForm->activateWindow();
    myPostForm->raise();
}

void ThreadTab::gallery(){
    /*QString command = "feh"; QStringList arguments; arguments << QDir("./"+board+"/"+thread).absolutePath()
                                                              << "--auto-zoom"
                                                              << "--index-info" << "\"%n\n%S\n%wx%h\""
                                                              << "--borderless"
                                                              << "--image-bg" << "black"
                                                              << "--preload";*/
    QString command = "mpv"; QStringList arguments; arguments << QDir("./"+board+"/"+thread).absolutePath();
    QProcess().startDetached(command,arguments);
}

void ThreadTab::addPost(ThreadForm *tf){
    ui->threads->addWidget(tf);
}

void ThreadTab::addStretch(){
    ui->threads->addStretch(1);
}

int ThreadTab::getMinWidth(){
    return ui->scrollArea->minimumWidth();
}

void ThreadTab::updateWidth(){
    ui->scrollArea->setMinimumWidth(this->sizeHint().width());
}

void ThreadTab::loadPosts(){
    if(reply->error()){
        qDebug().noquote() << "loading post error:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    //write to file and make json array
    QByteArray rep = reply->readAll();
    QFile jsonFile(board+"/"+thread+"/"+thread+".json"); // "des" is the file path to the destination file
    jsonFile.open(QIODevice::WriteOnly);
    QDataStream out(&jsonFile);
    out << rep;
    QJsonArray posts = QJsonDocument::fromJson(rep).object().value("posts").toArray();

    //make json arrays
    int length = posts.size();
    qDebug().noquote() << QString("length is ").append(QString::number(length));
    for(int i=tfMap.size();i<length;i++){
        QJsonObject p = posts.at(i).toObject();
        ThreadForm *tf = new ThreadForm(board,thread,PostType::Reply,true,this);
        ui->threads->addWidget(tf);
        tf->load(p);
        connect(tf,&ThreadForm::floatLink,this,&ThreadTab::floatReply);
        //todo on hide clicked remove from map and update replies
        tfMap.insert(tf->post->no,tf);
        if(i==0){
            if(tf->post->sub.length())this->setWindowTitle("/"+board+"/"+thread + " - " + tf->post->sub);
            else if(tf->post->com.length()) this->setWindowTitle("/"+board+"/"+thread + " - " + ThreadForm::htmlParse(tf->post->com));
        }
        //seg faults
        //connect(tf,&ThreadForm::destroyed,[=](){tfMap.remove(tf->post->no);});
        QSet<QString> quotes = tf->quotelinks;
        //QCoreApplication::processEvents();
        ThreadForm* replyTo;
        foreach (const QString &orig, quotes)
        {
            replyTo = tfMap.find(orig).value();
            if(replyTo != tfMap.end().value()){
                //replyTo->replies.insert(tf->post->no);
                replyTo->replies.insert(tf->post->no.toDouble(),tf->post->no);
                replyTo->setReplies();
            }
            //QCoreApplication::processEvents();
        }
    }
    ui->threads->addStretch(1);
    reply->deleteLater();
}

void ThreadTab::updatePosts(){
    /*updated = false;
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        //cout << i.key() << ": " << i.value() << endl;
        ((ThreadForm*)mapI.value())->updateComHeight();
        //mapI.remove();
    }*/
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

ThreadForm* ThreadTab::findPost(QString postNum){
    //TODO return 0 if not there
    //already does it?
    return tfMap.value(postNum);
}

void ThreadTab::findText(const QString text){
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
        match = re.match(tf->post->com);
        if(!match.hasMatch()){
            tf->hide();
        }
        else qDebug().noquote().nospace() << "found " << text << " in thread #" << tf->post->no;
    }
}

void ThreadTab::on_pushButton_clicked()
{
    findText(ui->lineEdit->text());
}

void ThreadTab::quoteIt(QString text){
    myPostForm->appendText(text);
    openPostForm();
}

bool ThreadTab::eventFilter(QObject *obj, QEvent *event)
{
    switch(event->type())
    {
    case QEvent::KeyPress:{
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        //int mod = keyEvent->modifiers();
        int key = keyEvent->key();
        //qDebug("Ate modifier %d",mod);
        //qDebug("Ate key press %d", key);
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
        break;
    }
    //check cursor change instead?
    case QEvent::Leave:
    case QEvent::Wheel:
        deleteFloat();
    default:
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

void ThreadTab::floatReply(const QString &link){
    deleteFloat();
    QPointer<ThreadForm> tf = findPost(link);
    if(!tf) return;
    floating = tf->clone();
    floating->deleteHideLayout();
    floating->setParent(this);
    floating->setObjectName("reply");
    floating->setWindowFlags(Qt::ToolTip);
    floating->setWindowTitle("reply");
    QPoint globalCursorPos = QCursor::pos();
    QSize sizeHint = floating->sizeHint();
    floating->setGeometry(globalCursorPos.x()+10,globalCursorPos.y()+10,sizeHint.width(),sizeHint.height());
    //floating->setStyleSheet("border-style:solid;border-width: 4px;");
    floating->setStyleSheet(QString::fromUtf8("QWidget#ThreadForm\n"
    "{\n"
    "    border: 3px solid black;\n"
    "}\n"
    ""));
    //floating->move(globalCursorPos.x()+10,globalCursorPos.y()+10);
    floating->updateGeometry();
    floating->update();
    floating->show();
}

void ThreadTab::deleteFloat(){
    if(floating && floating->post){
        floating->hide();
        floating->deleteLater();
    }
}

void ThreadTab::updateFloat(){
    if(floating && floating->post){
        QPoint globalCursorPos = QCursor::pos();
        QSize sizeHint = floating->sizeHint();
        floating->setGeometry(globalCursorPos.x()+10,globalCursorPos.y()+10,sizeHint.width(),sizeHint.height());
        //floating->move(globalCursorPos.x()+10,globalCursorPos.y()+10);
        floating->updateGeometry();
        floating->update();
    }
}
