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
#include <QScrollBar>
#include <QTimer>
#include "netcontroller.h"
#include "mainwindow.h"

ThreadTab::ThreadTab(QString board, QString thread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThreadTab)
{
    this->updated = false;
    ui->setupUi(this);
    this->board = board;
    this->thread = thread;
    this->setWindowTitle("/"+board+"/"+thread);
    //startUp();
    workerThread = new QThread;
    helper.startUp(board,thread, this);
    helper.moveToThread(workerThread);
    connect(&helper,&ThreadTabHelper::newTF,this,&ThreadTab::onNewTF);
    connect(&helper,&ThreadTabHelper::windowTitle,this,&ThreadTab::onWindowTitle);
    //myPostForm = new PostForm(board,thread,this);
    myPostForm.load(board,thread);
    this->setShortcuts();
    this->installEventFilter(this);
    space = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
    connect(mw,&MainWindow::setAutoUpdate,[=](bool update){helper.setAutoUpdate(update);});
    connect(&helper,&ThreadTabHelper::addStretch,[=](){this->addStretch();});
}

/*void ThreadTab::checkScroll(){
    QScrollBar* vBar = ui->scrollArea->verticalScrollBar();
    int height = ui->scrollAreaWidgetContents->size().height();
    int scrollPos = vBar->value() + vBar->height();
    qDebug() << height;
    qDebug() << scrollPos;
    qDebug() << "----------";
    if(scrollPos >= height - 500) vBar->triggerAction(QScrollBar::SliderToMaximum);
    //vBar->setSliderDown();
}*/

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
    connect(expandAll, &QAction::triggered,[=](){helper.loadAllImages();});
    this->addAction(expandAll);
    QAction *refresh = new QAction(this);
    refresh->setShortcut(Qt::Key_R);
    refresh->setShortcutContext(Qt::ApplicationShortcut);
    connect(refresh, &QAction::triggered,[=](){helper.getPosts();});
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

ThreadTab::~ThreadTab()
{
    QMutableMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        delete mapI.value();
        mapI.remove();
    }
    helper.abort = 1;
    workerThread->quit();
    workerThread->wait();
    //delete myPostForm;
    workerThread->deleteLater();
    delete ui;
}

void ThreadTab::openPostForm(){
    myPostForm.show();
    myPostForm.activateWindow();
    myPostForm.raise();
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

void ThreadTab::addStretch(){
    //ui->threads->addStretch(1);
    ui->threads->removeItem(space);
    ui->threads->insertItem(-1,space);
}

int ThreadTab::getMinWidth(){
    return ui->scrollArea->minimumWidth();
}

void ThreadTab::updateWidth(){
    ui->scrollArea->setMinimumWidth(this->sizeHint().width());
}

void ThreadTab::onNewTF(ThreadForm* tf){
    ui->threads->addWidget(tf);
    tfMap.insert(tf->post.no,tf);
    connect(tf,&ThreadForm::floatLink,this,&ThreadTab::floatReply);
}

void ThreadTab::onWindowTitle(QString title){
    this->setWindowTitle(title);
    if(mw->currentWidget() == this) mw->setWindowTitle(title);
}

void ThreadTab::loadAllImages(){
    updated = false;
    QMapIterator<QString,ThreadForm*> mapI(tfMap);
    while (mapI.hasNext()) {
        mapI.next();
        //cout << i.key() << ": " << i.value() << endl;
        //((ThreadForm*)mapI.value())->loadOrig();
        static_cast<ThreadForm *>(mapI.value())->loadOrig();
        //mapI.remove();
    }
}

ThreadForm* ThreadTab::findPost(QString postNum){
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
        match = re.match(tf->post.com);
        if(!match.hasMatch()){
            tf->hide();
        }
        else qDebug().noquote().nospace() << "found " << text << " in thread #" << tf->post.no;
    }
}

void ThreadTab::on_pushButton_clicked()
{
    findText(ui->lineEdit->text());
}

void ThreadTab::quoteIt(QString text){
    myPostForm.appendText(text);
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
    if(floating){
        delete floating;
        //floating->hide();
        //floating->deleteLater();
    }
}

void ThreadTab::updateFloat(){
    if(floating){
        QPoint globalCursorPos = QCursor::pos();
        QSize sizeHint = floating->sizeHint();
        floating->setGeometry(globalCursorPos.x()+10,globalCursorPos.y()+10,sizeHint.width(),sizeHint.height());
        //floating->move(globalCursorPos.x()+10,globalCursorPos.y()+10);
        floating->updateGeometry();
        floating->update();
    }
}
