#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWidget>
#include <QStandardItem>
#include <QEvent>
#include <QKeyEvent>
#include <QSettings>
#include <QShortcut>
#include <stdio.h>
#include "threadtab.h"
#include "threadform.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0,0);
    ui->splitter->setStretchFactor(1,1);
    /*QStringList headers;
    headers << "hi";
    QString data = "yo";
    model = new TreeModel(headers,data);*/
    model = new QStandardItemModel;
    model->setColumnCount(1);
    ui->treeView->setModel(model);
    ui->treeView->installEventFilter(this);
    connect(ui->treeView->selectionModel(),&QItemSelectionModel::selectionChanged,this,
            &MainWindow::onSelectionChanged);
    setShortcuts();
    //check binding sequence
    //qDebug() << QKeySequence::keyBindings(QKeySequence::PreviousChild);
}

void MainWindow::setShortcuts(){
    QAction *nTab = new QAction(this);
    nTab->setShortcut(QKeySequence::NextChild);
    nTab->setShortcutContext(Qt::ApplicationShortcut);
    connect(nTab, &QAction::triggered, this, &MainWindow::nextTab);
    this->addAction(nTab);
    QAction *pTab = new QAction(this);
    pTab->setShortcut(QKeySequence("Ctrl+Shift+Tab"));
    pTab->setShortcutContext(Qt::ApplicationShortcut);
    connect(pTab, &QAction::triggered, this, &MainWindow::prevTab);
    this->addAction(pTab);
    QAction *del = new QAction(this);
    del->setShortcut(QKeySequence::Delete);
    connect(del, &QAction::triggered, this, &MainWindow::deleteSelected);
    this->addAction(del);
    QAction *closeTab = new QAction(this);
    closeTab->setShortcut(QKeySequence::Close);
    closeTab->setShortcutContext(Qt::ApplicationShortcut);
    connect(closeTab, &QAction::triggered, this, &MainWindow::deleteSelected);
    this->addAction(closeTab);
    QAction *navBar = new QAction(this);
    navBar->setShortcut(QKeySequence("Ctrl+l"));
    navBar->setShortcutContext(Qt::ApplicationShortcut);
    connect(navBar, &QAction::triggered, this, &MainWindow::focusBar);
    this->addAction(navBar);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::nextTab(){
    int numRows = model->rowCount();
    if(!numRows) return;
    boardsSelected = ui->treeView->selectionModel()->selectedRows();
    int key = 0;
    if(boardsSelected.length()){
        QModelIndex qmi = boardsSelected.at(0);
        if(qmi.row()+1<numRows) key = qmi.row()+1;
    }
    ui->treeView->setCurrentIndex(model->index(key,0));
    ui->treeView->selectionModel()->setCurrentIndex(model->index(key,0),QItemSelectionModel::ClearAndSelect);
}

void MainWindow::prevTab(){
    int numRows = model->rowCount();
    if(!numRows) return;
    boardsSelected = ui->treeView->selectionModel()->selectedRows();
    int key = numRows-1;
    if(boardsSelected.length()){
        QModelIndex qmi = boardsSelected.at(0);
        if(qmi.row()-1>=0) key = qmi.row()-1;
    }
    ui->treeView->setCurrentIndex(model->index(key,0));
    ui->treeView->selectionModel()->setCurrentIndex(model->index(key,0),QItemSelectionModel::ClearAndSelect);
}

void MainWindow::on_pushButton_clicked()
{
    QString searchString = ui->lineEdit->text();
    loadFromSearch(searchString,true);
}

void MainWindow::loadFromSearch(QString searchString, bool select){
    QRegularExpression re("^(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?(\\w+)(?:\\/thread)?\\/(\\d+)(?:#p\\d+)?$",QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(searchString);
    QRegularExpressionMatch match2;
    QString displayString;
    BoardTab *bt;
    if (!match.hasMatch()) {
        QRegularExpression res("^(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?(\\w+)\\/(?:catalog#s=)?(.+)?$",QRegularExpression::CaseInsensitiveOption);
        match2 = res.match(searchString);
    }
    else{
        onNewThread(this,match.captured(1),match.captured(2));
        return;
    }
    if(match2.hasMatch()){
        bt = new BoardTab(match2.captured(1),BoardType::Catalog,match2.captured(2));
        displayString = "/"+match2.captured(1)+"/"+match2.captured(2);
    }
    else{
        //return;
        bt = new BoardTab(searchString,BoardType::Index);
        displayString = "/"+searchString+"/";
    }
    //ui->verticalLayout_3->addWidget(bt);
    ui->stackedWidget->addWidget(bt);
    //ui->stackedWidget
    Tab tab = {Tab::TabType::Board,bt,searchString};
    tabs.push_back(tab);
    /*QList<QVariant> rootData;
    rootData << displayString;
    //TreeItem* parent1 = new TreeItem();
    QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    model->insertRow(index.row()+1,index.parent());
    QModelIndex child = model->index(index.row()+1, 0, index.parent());
    model->setData(child,displayString);*/
    QStandardItem* parent1 = new QStandardItem(displayString);
    model->appendRow(parent1);
    pages++;
    parent1->setData(pages,Qt::UserRole);
    tabsNew.insert(pages,tab);
    //ui->stackedWidget->setCurrentWidget(bt);
    selectPage(pages,model);

    /*if(select){
        ui->treeView->setCurrentIndex(parent1->index());
        ui->stackedWidget->setCurrentIndex(parent1->index().row());
    }*/
}

void MainWindow::onNewThread(QWidget* parent, QString board, QString thread){
    qDebug() << "loading /"+board+"/"+thread;
    ThreadTab *tt = new ThreadTab(board,thread);
    //ui->verticalLayout_3->addWidget(tt);
    ui->stackedWidget->addWidget(tt);
    Tab tab = {Tab::TabType::Thread,tt,QString("/"+board+"/"+thread)};
    tabs.push_back(tab);
    QStandardItem* parent1 = new QStandardItem("/"+board+"/"+thread);
    model->appendRow(parent1);
    pages++;
    parent1->setData(pages,Qt::UserRole);
    tabsNew.insert(pages,tab);
    //ui->stackedWidget->setCurrentWidget(tt);
    selectPage(pages,model);
    //ui->treeView->selectionModel()->sel
    //tabsNew.insert((void*)tt,tab);
    //TreeItem* parent1 = new TreeItem();
    /*QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    model->insertRow(index.row()+1,index.parent());
    QModelIndex child = model->index(index.row()+1, 0, index.parent());
    model->setData(child,"/"+board+"/"+thread);*/
    //tt->hide();
}

void MainWindow::addTab(){
}

void MainWindow::on_treeView_clicked(QModelIndex index)
{
    int pageId = index.data(Qt::UserRole).toInt();
    ui->stackedWidget->setCurrentWidget((QWidget*)(tabsNew.find(pageId)->TabPointer));
    //ui->stackedWidget->setCurrentIndex(index.row());
    //show_one(index);
}

void MainWindow::show_one(QModelIndex index){
    int pageId = index.data(Qt::UserRole).toInt();
    ui->stackedWidget->setCurrentWidget((QWidget*)(tabsNew.find(pageId)->TabPointer));
    //((QWidget*)tabs.at(index.row()).TabPointer)->raise();
    //ui->stackedWidget->setCurrentIndex(index.row());
    /*int size = tabs.size();
    for(int i=0;i<size;i++){
        if(i == index.row()) continue;
        ((QWidget*)tabs.at(i).TabPointer)->hide();
        //((BoardTab*)bts.at(i))->hide();
    }
    Tab tab = tabs.at(index.row());
    ((QWidget*)tab.TabPointer)->show();*/
    /*if(tab.type == Tab::TabType::Thread && !((ThreadTab*)tab.TabPointer)->updated){
        ((ThreadTab*)tab.TabPointer)->updatePosts();
    }*/
}


void MainWindow::onSelectionChanged(){
    QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
    if(list.size()) {
        int pageId = list.at(0).data(Qt::UserRole).toInt();
        ui->stackedWidget->setCurrentWidget((QWidget*)(tabsNew.find(pageId)->TabPointer));
    }
    /*else{
        QModelIndex index = ui->treeView->currentIndex();
        ui->stackedWidget->setCurrentIndex(index.row());
        int pageId = index.data(Qt::UserRole).toInt();
        ui->stackedWidget->setCurrentWidget((QWidget*)(tabsNew.find(pageId)->TabPointer));
    }*/

    //boardsSelected = ui->treeView->selectionModel()->selectedRows();
    //if(boardsSelected.size()) ui->stackedWidget->setCurrentIndex(boardsSelected.at(0).row());
    //if(boardsSelected.size()) show_one(boardsSelected.at(0));
}

//TODO replace with regular QAction shortcuts
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();
        int mod = keyEvent->modifiers();
        qDebug("Ate key press %d", key);
        qDebug("Modifers %d", mod);
        if(key == 53){
            /*const QModelIndexList indexList = ui->treeView->selectionModel()->selectedRows();
            int row = indexList.at(0).row();
            qDebug() << (Tab::TabType)tabs.at(row).type;
            ((Tab::TabType)tabs.at(row).type) == Tab::TabType::Board ? ((BoardTab*)tabs.at(row).TabPointer)->updatePosts() :
                                                          ((ThreadTab*)tabs.at(row).TabPointer)->updatePosts();*/
            qDebug() << ui->treeView->indentation();
        }
        else if(key == 16777269){
            ui->lineEdit->setFocus();
        }
        else if(key == 16777266){
            qDebug("setting focus");
            ui->treeView->setFocus();
        }
        else if(key == 16777267){
            ui->scrollAreaWidgetContents->setFocus();
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

void MainWindow::deleteSelected(){
    const QModelIndexList indexList = ui->treeView->selectionModel()->selectedRows();
    if(!indexList.size()) return;
    int pageId = indexList.at(0).data(Qt::UserRole).toInt();
    removePage(pageId,model);
    tabsNew.remove(pageId);
    //QModelIndex item = indexList.at(0);
    /*model->removeRow(item.row());
    tabsNew.remove(item.data(Qt::UserRole).toInt());*/
    //ui->verticalLayout_3->removeWidget((QWidget*)tabs.at(row).TabPointer);
    //((QWidget*)tabs.at(row).TabPointer)->close();
    //((QWidget*)tabs.at(row).TabPointer)->deleteLater();
    //tabs.erase(tabs.begin()+row);
    //model->removeRow(ui->treeView->selectionModel()->sele)
}

bool MainWindow::removePage(int searchPage, QAbstractItemModel* model, QModelIndex parent) {
    for(int r = 0; r < model->rowCount(parent); ++r) {
        QModelIndex index = model->index(r, 0, parent);
        int pageId = index.data(Qt::UserRole).toInt();
        if(pageId == searchPage) return model->removeRow(r,parent);
        if( model->hasChildren(index) ) {
            removePage(searchPage, model, index);
        }
    }
}

void MainWindow::selectPage(int searchPage, QAbstractItemModel* model, QModelIndex parent) {
    for(int r = 0; r < model->rowCount(parent); ++r) {
        QModelIndex index = model->index(r, 0, parent);
        int pageId = index.data(Qt::UserRole).toInt();
        if(pageId == searchPage) return ui->treeView->selectionModel()->setCurrentIndex(model->index(r,0),QItemSelectionModel::ClearAndSelect);
        if( model->hasChildren(index) ) {
            removePage(searchPage, model, index);
        }
    }
}

void MainWindow::on_lineEdit_returnPressed()
{
    on_pushButton_clicked();
    ui->treeView->setFocus();
}

void MainWindow::focusTree(){
    ui->treeView->setFocus();
}

void MainWindow::focusBar(){
    ui->lineEdit->setFocus();
    ui->lineEdit->selectAll();
}

void MainWindow::saveSession(){
    QSettings settings;
    QStringList session;
    QMapIterator<int,Tab> i(tabsNew);
    while(i.hasNext()){
        session.append(i.next().value().searchString);
    }
    //int numTabs = tabs.size();
    /*for (int i=0; i < numTabs; i++) {
        session.append(((Tab)(tabs.at(i))).searchString);
    }*/
    settings.setValue("session",session);
    //QStringList List;
    //qDebug() << List;
    // save list
    //settings.setValue("session_new", QVariant::fromValue(List));
}

void MainWindow::loadSession(){
    QSettings settings;
    QStringList session = settings.value("session",QStringList()).toStringList();
    for(int i=0;i<session.length();i++){
        loadFromSearch(session.at(i),false);
    }
}
