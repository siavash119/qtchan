#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QVariant>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStackedWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QTextEdit>
#include <QStandardItem>
#include "threadform.h"
#include <QEvent>
#include <QKeyEvent>
#include <stdio.h>
#include "threadtab.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0,0);
    ui->splitter->setStretchFactor(1,1);

    model = new QStandardItemModel;
    model->setColumnCount(1);
    ui->treeView->setModel(model);
    ui->treeView->installEventFilter(this);
    connect(ui->treeView->selectionModel(),&QItemSelectionModel::selectionChanged,this,
            &MainWindow::onSelectionChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString searchString = ui->lineEdit->text();
    QRegularExpression re("(?:(?:https?:\\/\\/)?boards.4chan.org\\/)?(\\w+)(?:\\/thread)?\\/(\\d+)");
    QRegularExpressionMatch match = re.match(searchString);
    QRegularExpressionMatch match2;
    BoardTab *bt;
    if (!match.hasMatch()) {
        QRegularExpression res("(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?(\\w+)\\/(\\w+)?");
        match2 = res.match(searchString);
    }
    else{
        onNewThread(this,match.captured(1),match.captured(2));
        return;
    }
    if(match2.hasMatch()){
        bt = new BoardTab(match2.captured(1),BoardType::Catalog,match2.captured(2));
    }
    else{
        bt = new BoardTab(searchString,BoardType::Index);
    }
    ui->verticalLayout_3->addWidget(bt);
    Tab tab = {Tab::TabType::Board,bt};
    tabs.push_back(tab);
    QStandardItem* parent1 = new QStandardItem("/"+searchString+"/");
    model->appendRow(parent1);
    ui->treeView->selectionModel()->clearSelection();
    ui->treeView->selectionModel()->select(parent1->index(),QItemSelectionModel::Select);
    //show_one(parent1->index());
}

void MainWindow::onNewThread(QWidget* parent, QString board, QString thread){
    //qDebug() << "adding "+thread+" from /"+tf->board+"/"+tf->threadNum;

    ThreadTab *tt = new ThreadTab(board,thread);
    ui->verticalLayout_3->addWidget(tt);
    QStandardItem* parent1 = new QStandardItem("/"+board+"/"+thread);
    Tab tab = {Tab::TabType::Thread,tt};
    tabs.push_back(tab);
    model->appendRow(parent1);
    tt->hide();
    //ui->treeView->selectionModel()->select();
    //show_one(parent1->index());
}

void MainWindow::addTab(){
}

void MainWindow::on_treeView_clicked(QModelIndex index)
{
    show_one(index);
}

void MainWindow::show_one(QModelIndex index){
    int size = tabs.size();
    for(int i=0;i<size;i++){
        ((QWidget*)tabs.at(i).TabPointer)->hide();
        //((BoardTab*)bts.at(i))->hide();
    }
    Tab tab = tabs.at(index.row());
    if(tab.type == Tab::TabType::Thread && !((ThreadTab*)tab.TabPointer)->updated){
        ((ThreadTab*)tab.TabPointer)->show();
        ((ThreadTab*)tab.TabPointer)->updatePosts();
    }
    else{
        ((BoardTab*)tab.TabPointer)->show();
    }
}


void MainWindow::onSelectionChanged(){
    boardsSelected = ui->treeView->selectionModel()->selectedRows();
    if(boardsSelected.size()) show_one(boardsSelected.at(0));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();
        qDebug("Ate key press %d", key);
        if(key == 16777223){
            deleteSelected();
        }
        else if(key == 16777220){
            show_one(ui->treeView->selectionModel()->selectedRows().at(0));
        }
        else if(key == 53){
            const QModelIndexList indexList = ui->treeView->selectionModel()->selectedRows();
            int row = indexList.at(0).row();
            qDebug() << (Tab::TabType)tabs.at(row).type;
            ((Tab::TabType)tabs.at(row).type) == Tab::TabType::Board ? ((BoardTab*)tabs.at(row).TabPointer)->updatePosts() :
                                                          ((ThreadTab*)tabs.at(row).TabPointer)->updatePosts();
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

void MainWindow::deleteSelected(){
    const QModelIndexList indexList = ui->treeView->selectionModel()->selectedRows();
    if(!indexList.size()) return;
    int row = indexList.at(0).row();
    ui->verticalLayout_3->removeWidget((QWidget*)tabs.at(row).TabPointer);
    ((QWidget*)tabs.at(row).TabPointer)->close();
    ((QWidget*)tabs.at(row).TabPointer)->deleteLater();
    model->removeRow(row);
    tabs.erase(tabs.begin()+row);
}

void MainWindow::on_lineEdit_returnPressed()
{
    on_pushButton_clicked();
    ui->treeView->setFocus();
}
