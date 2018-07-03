#include "archivetab.h"
#include "ui_archivetab.h"
#include "filter.h"
#include "chans.h"
#include <QDir>
#include <QFile>
#include <QPushButton>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QAction>

ArchiveTab::ArchiveTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ArchiveTab)
{
	ui->setupUi(this);
	QHeaderView *headerView = new QHeaderView(Qt::Horizontal, ui->table);
	ui->table->setHorizontalHeader(headerView);
	headerView->setSectionResizeMode(0, QHeaderView::Fixed);
	headerView->setSectionResizeMode(1, QHeaderView::Fixed);
	headerView->setSectionResizeMode(2, QHeaderView::Stretch);
	headerView->setSectionResizeMode(3, QHeaderView::Fixed);
	headerView->setSectionsClickable(true);
	ui->table->setColumnWidth(0,96);
	ui->table->setColumnWidth(1,150);
	connect(ui->table,&QTableWidget::cellClicked,this,&ArchiveTab::tableClicked);
	fillAPIs();
	QAction *refresh = new QAction(this);
	refresh->setShortcut(Qt::Key_F5);
	refresh->setShortcutContext(Qt::WindowShortcut);
	connect(refresh,&QAction::triggered,this,&ArchiveTab::fillAPIs);
	this->addAction(refresh);
}

void ArchiveTab::fillAPIs(){
	clearLayout(ui->apis);
	QStringList apiList = Chans::apiList();
	foreach(QString api, apiList){
		if(!QDir(api).exists()) apiList.removeOne(api);
	}
	if(apiList.size()==1){
		this->api = apiList.at(0);
		fillBoards();
	}
	else{
		int size = apiList.size();
		for(int i=0;i<size;i++){
			QString api = apiList.at(i);
			QPushButton *button = new QPushButton(this);
			button->setText(api);
			connect(button,&QPushButton::clicked,this,&ArchiveTab::apiClicked);
			ui->apis->insertWidget(i,button);
		}
	}
}

void ArchiveTab::apiClicked(){
	QPushButton* buttonSender = qobject_cast<QPushButton*>(sender());
	if(buttonSender){
		QString buttonText = buttonSender->text();
		this->api = buttonText;
		fillBoards();
	}
}

void ArchiveTab::fillBoards(){
	clearLayout(ui->boards);
	QStringList entries = QDir(api).entryList(QStringList(),QDir::AllDirs | QDir::NoDotAndDotDot);
	entries.removeOne("flags");
	int size = entries.size();
	for(int i=0;i<size;i++){
		QString board = entries.at(i);
		if(!QDir(api+"/"+board).entryList().contains("index")){
			continue;
		}
		QPushButton *button = new QPushButton(this);
		button->setText(board);
		connect(button,&QPushButton::clicked,this,&ArchiveTab::boardClicked);
		ui->boards->insertWidget(i,button);
	}
}

void ArchiveTab::boardClicked(){
	QPushButton* buttonSender = qobject_cast<QPushButton*>(sender());
	if(buttonSender){
		QString buttonText = buttonSender->text();
		fillTable(buttonText);
	}
}

void ArchiveTab::fillTable(QString board){
	this->board = board;
	ui->table->setRowCount(0);
	QDir tech(api+"/"+board);
	QStringList entries = tech.entryList(QStringList(),QDir::AllDirs | QDir::NoDotAndDotDot);
	entries.removeOne("index");
	for(int i=0;i<entries.size();i++){
		QFile json(api+"/"+board+"/"+entries.at(i)+"/"+entries.at(i)+".json");
		if(json.exists()){
			json.open(QIODevice::ReadOnly);
			QByteArray postData = json.readAll();
			json.close();
			ui->table->insertRow(0);
			ui->table->setRowHeight(0,96);
			QString num = entries.at(i);
			ui->table->setItem(0,1,new QTableWidgetItem(num));
			Chan *chanAPI = Chans::get(api);
			QJsonArray posts = chanAPI->postsArray(postData);
			QString temp;
			Post p = chanAPI->post(posts.at(0).toObject(),board,temp);
			QString sub = p.sub;
			if(sub.isEmpty()){
				sub = p.com;
				sub = Filter::replaceQuoteStrings(sub);
			}
			sub = Filter::titleParse(sub);
			if(p.files.size()){
				QString filename = p.files.at(0).tnPath;
				qDebug() << filename;
				QString thumbPath(api % '/' % board % '/' % num % '/' % filename);
				QImage *img = new QImage();
				bool loaded = img->load(thumbPath);
				if(loaded){
					QTableWidgetItem *thumbnail = new QTableWidgetItem;
					thumbnail->setData(Qt::DecorationRole, QPixmap::fromImage(img->scaled(96,96,Qt::KeepAspectRatio)));
					ui->table->setItem(0,0,thumbnail);
				}
				delete img;
			}
			ui->table->setItem(0,2,new QTableWidgetItem(sub));
			QTableWidgetItem* numPosts = new QTableWidgetItem;
			numPosts->setData(Qt::DisplayRole,posts.size());
			ui->table->setItem(0,3,numPosts);
			QTableWidgetItem *del = new QTableWidgetItem("X");
			del->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			del->setTextColor(QColor(255,0,0));
			ui->table->setItem(0,4,del);
		}
	}
}

void ArchiveTab::tableClicked(int row, int column){
	if(column == 0){
		QString threadNum = ui->table->item(row,1)->data(Qt::DisplayRole).toString();
		emit loadThread(api+'/'+board+'/'+threadNum);
	}
	else if(column == 4){
		QString threadNum = ui->table->item(row,1)->data(Qt::DisplayRole).toString();
		QDir dir(api+'/'+board+'/'+threadNum);
		if(dir.exists()) dir.removeRecursively();
		ui->table->removeRow(row);
	}
}

void ArchiveTab::clearLayout(QHBoxLayout *layout){
	QLayoutItem* item;
	while ((item = layout->takeAt(0)) != NULL)
	{
		delete item->widget();
		delete item;
	}
}

ArchiveTab::~ArchiveTab()
{
	delete ui;
}
