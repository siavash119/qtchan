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
    this->setWindowTitle("4chan Browser");
    this->setStyleSheet("background-color: grey; color:white");

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
    //QString url = "https://a.4cdn.org/"+searchString+"/1.json";
    //qDebug() << QString("getting ")+url;
    BoardTab *bt = new BoardTab(searchString);
    ui->verticalLayout_3->addWidget(bt);
    QStandardItem* parent1 = new QStandardItem(searchString);
    model->appendRow(parent1);
    //bts.push_back(bt);
    Tab tab = {Tab::TabType::Board,bt};
    tabs.push_back(tab);
    //bts.push_back(bt);
    show_one(parent1->index());
}

void MainWindow::onNewThread(ThreadForm* tf, QString board, QString thread){
    ThreadTab *tt = new ThreadTab(board,thread);
    ui->verticalLayout_3->addWidget(tt);
    QStandardItem* parent1 = new QStandardItem(thread);
    model->appendRow(parent1);
    Tab tab = {Tab::TabType::Thread,tt};
    tabs.push_back(tab);
    show_one(parent1->index());
}

/*void QTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected){
    qDebug() << &selected;
}*/

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
    ((QWidget*)tabs.at(index.row()).TabPointer)->show();
    //((BoardTab*)bts.at(index.row()))->show();
    /*int size = bts.size();
    for(int i=0;i<size;i++){
        ((BoardTab*)bts.at(i))->hide();
    }
    ((BoardTab*)bts.at(index.row()))->show();*/
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
    int row = indexList.at(0).row();
    ((QWidget*)tabs.at(row).TabPointer)->close();
    //((BoardTab*)bts.at(row))->close();
    ui->verticalLayout_3->removeWidget((QWidget*)tabs.at(row).TabPointer);
    model->removeRow(row);
    tabs.erase(tabs.begin()+row);
    //bts.erase(bts.begin()+row);
    //onSelectionChanged();
    //if(bts.size() > 0) show_one(ui->treeView->selectionModel()->selectedRows().at(0));
}

/*void MainWindow::getThread(ThreadForm *tf, QString url){
    qDebug() << url;
}*/
