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
#include <QDesktopServices>
#include "boardtab.h"
#include "threadtab.h"
#include "threadform.h"

//TODO decouple item model/view logic to another class
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->splitter->setStretchFactor(0,0);
	ui->splitter->setStretchFactor(1,1);
	ui->treeView->setModel(model);
	ui->treeView->installEventFilter(this);
	selectionModel = ui->treeView->selectionModel();
	connect(selectionModel,&QItemSelectionModel::selectionChanged,this,
			&MainWindow::onSelectionChanged);
	settingsView.setParent(this,Qt::Tool
						 | Qt::WindowMaximizeButtonHint
						 | Qt::WindowCloseButtonHint);
	this->setShortcuts();
}

void MainWindow::setShortcuts()
{
	QAction *pTab = new QAction(this);
	pTab->setShortcut(QKeySequence("Ctrl+Shift+Tab"));
	pTab->setShortcutContext(Qt::ApplicationShortcut);
	connect(pTab, &QAction::triggered,this,&MainWindow::prevTab);
	this->addAction(pTab);

	QAction *nTab = new QAction(this);
	nTab->setShortcut(QKeySequence("Ctrl+Tab"));
	nTab->setShortcutContext(Qt::ApplicationShortcut);
	connect(nTab, &QAction::triggered,this,&MainWindow::nextTab);
	this->addAction(nTab);

	QAction *firstItem = new QAction(this);
	firstItem->setShortcut(QKeySequence("Ctrl+1"));
	firstItem->setShortcutContext(Qt::ApplicationShortcut);
	connect(firstItem, &QAction::triggered,[=]{
		if(!model->rowCount()) return;
		selectionModel->setCurrentIndex(model->index(0,0),
						QItemSelectionModel::ClearAndSelect);
	});
	this->addAction(firstItem);

	QAction *pParent = new QAction(this);
	pParent->setShortcut(QKeySequence("Ctrl+2"));
	pParent->setShortcutContext(Qt::ApplicationShortcut);
	connect(pParent, &QAction::triggered,this,&MainWindow::prevParent);
	this->addAction(pParent);

	QAction *nParent = new QAction(this);
	nParent->setShortcut(QKeySequence("Ctrl+3"));
	nParent->setShortcutContext(Qt::ApplicationShortcut);
	connect(nParent, &QAction::triggered,this,&MainWindow::nextParent);
	this->addAction(nParent);

	QAction *lastItem = new QAction(this);
	lastItem->setShortcut(QKeySequence("Ctrl+4"));
	lastItem->setShortcutContext(Qt::ApplicationShortcut);
	connect(lastItem, &QAction::triggered,[=]{
		if(!model->rowCount()) return;
		TreeItem *tm = model->getItem(model->index(model->rowCount(),0));
		while(tm->childCount()){
			tm = tm->child(tm->childCount()-1);
		}
		selectionModel->setCurrentIndex(model->getIndex(tm),
						QItemSelectionModel::ClearAndSelect);
	});
	this->addAction(lastItem);

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

	QAction *setAutoUpdate = new QAction(this);
	setAutoUpdate->setShortcut(QKeySequence("Ctrl+u"));
	setAutoUpdate->setShortcutContext(Qt::ApplicationShortcut);
	connect(setAutoUpdate, &QAction::triggered, this, &MainWindow::toggleAutoUpdate);
	this->addAction(setAutoUpdate);

	QAction *setAutoExpand = new QAction(this);
	setAutoExpand->setShortcut(QKeySequence("Ctrl+e"));
	setAutoExpand->setShortcutContext(Qt::ApplicationShortcut);
	connect(setAutoExpand, &QAction::triggered, this, &MainWindow::toggleAutoExpand);
	this->addAction(setAutoExpand);

	QAction *saveState = new QAction(this);
	saveState->setShortcut(QKeySequence(Qt::Key_F10));
	saveState->setShortcutContext(Qt::ApplicationShortcut);
	connect(saveState, &QAction::triggered, this, &MainWindow::saveSession);
	this->addAction(saveState);

	QAction *openExplorer = new QAction(this);
	openExplorer->setShortcut(QKeySequence("Ctrl+o"));
	openExplorer->setShortcutContext(Qt::ApplicationShortcut);
	connect(openExplorer, &QAction::triggered, this, &MainWindow::openExplorer);
	this->addAction(openExplorer);

	ui->actionSave->setShortcut(QKeySequence("Ctrl+s"));
	ui->actionSave->setShortcutContext(Qt::ApplicationShortcut);
	connect(ui->actionSave,&QAction::triggered,this,&MainWindow::saveSession);

	ui->actionReload->setShortcut(QKeySequence("Ctrl+r"));
	connect(ui->actionReload,&QAction::triggered,[=]{
		QMapIterator<QWidget*,Tab> i(tabs);
		while(i.hasNext()) {
			Tab tab = i.next().value();
			if(tab.type == Tab::TabType::Board) static_cast<BoardTab*>(tab.TabPointer)->helper.getPosts();
			else static_cast<ThreadTab*>(tab.TabPointer)->helper.getPosts();
		}
	});

	ui->actionSettings->setShortcut(QKeySequence("Ctrl+p"));
	connect(ui->actionSettings,&QAction::triggered,[=]{
		if(settingsView.isVisible()){
			qDebug() << "hiding settings window";
			settingsView.hide();
		}
		else{
			qDebug() << "showing settings window";
			settingsView.show();
		}
	});
}

MainWindow::~MainWindow()
{
	saveSession();
	delete model;
	delete ui;
}

//TODO put toggle functions in 1 function with argument
void MainWindow::toggleAutoUpdate()
{
	QSettings settings;
	bool autoUpdate = !settings.value("autoUpdate").toBool();
	qDebug () << "setting autoUpdate to" << autoUpdate;
	settings.setValue("autoUpdate",autoUpdate);
	emit setAutoUpdate(autoUpdate);
	settingsView.refreshValues();
}

void MainWindow::toggleAutoExpand()
{
	QSettings settings;
	bool autoExpand = !settings.value("autoExpand").toBool();
	qDebug () << "setting autoExpand to" << autoExpand;
	settings.setValue("autoExpand",autoExpand);
	emit setAutoExpand(autoExpand);
	settingsView.refreshValues();
}

void MainWindow::updateSettings(QString setting, QVariant value){
	if(setting == "autoUpdate")
		emit setAutoUpdate(value.toBool());
	else if(setting == "autoExpand")
		emit setAutoExpand(value.toBool());
}

void MainWindow::openExplorer(){
	QString url;
	if(!ui->stackedWidget->count()) return;
	TreeItem *item = model->getItem(selectionModel->currentIndex());
	if(item->type == TreeItemType::thread){
		ThreadTab *tab = static_cast<ThreadTab*>(item->tab);
		url = QDir("./"+tab->board+"/"+tab->thread).absolutePath();
	}
	else{
		BoardTab *tab = static_cast<BoardTab*>(item->tab);
		url = QDir("./"+tab->board).absolutePath();
	}
	qDebug() << "Opening folder" << url;
	QDesktopServices::openUrl(QUrl::fromLocalFile(url));
}

//for next/prev tab just send up and down key and loop to beginning/end if no change in selection?
void MainWindow::nextTab()
{
	/*QKeyEvent event(QEvent::KeyPress,Qt::Key_Down,0);
	QApplication::sendEvent(ui->treeView, &event);*/
	QModelIndex qmi = ui->treeView->currentIndex();
	if(!model->rowCount(qmi.parent())) return;
	if(model->hasChildren(qmi) && ui->treeView->isExpanded(qmi)) {
		selectionModel->setCurrentIndex(model->index(0,0,qmi),QItemSelectionModel::ClearAndSelect);
		return;
	}
	while(qmi.row() == model->rowCount(qmi.parent())-1) { //is last child and no children
		qmi = qmi.parent();
	}
	if(qmi.row()+1 < model->rowCount(qmi.parent())) {
		selectionModel->setCurrentIndex(model->index(qmi.row()+1,0,qmi.parent()),QItemSelectionModel::ClearAndSelect);
		return;
	}
	selectionModel->setCurrentIndex(model->index(0,0),QItemSelectionModel::ClearAndSelect);
}

void MainWindow::prevTab()
{
	QModelIndex qmi = ui->treeView->currentIndex();
	if(!model->rowCount(qmi.parent())) return;
	if(qmi.row()-1 >= 0) {
		qmi = qmi.sibling(qmi.row()-1,0);
		while(model->hasChildren(qmi) && ui->treeView->isExpanded(qmi)) {
			qmi = qmi.child(model->rowCount(qmi)-1,0);
		}
		selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
		return;
	}
	qmi = qmi.parent();
	if(qmi.row() == -1) { //we're at the very first row so select last row
		qmi = model->index(model->rowCount()-1,0);
		while(model->hasChildren(qmi) && ui->treeView->isExpanded(qmi)) {
			qmi = qmi.child(model->rowCount(qmi)-1,0);
		}
	}
	selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
}

void MainWindow::prevParent(){
	int rowCount = model->rowCount();
	if(!rowCount) return;
	QModelIndex qmi = ui->treeView->currentIndex();
	if(qmi.parent() != QModelIndex()){
		while(qmi.parent() != QModelIndex()){
			qmi = qmi.parent();
		}
		selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
		return;
	}
	int row = qmi.row()-1;
	if(row == -1) row = rowCount-1;
	qmi = qmi.sibling(row,0);
	selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);

}

void MainWindow::nextParent(){
	int rowCount = model->rowCount();
	if(!rowCount) return;
	QModelIndex qmi = ui->treeView->currentIndex();
	while(qmi.parent() != QModelIndex()){
		qmi = qmi.parent();
	}
	int row = qmi.row()+1;
	if(row == rowCount) row = 0;
	qmi = qmi.sibling(row,0);
	selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
}

void MainWindow::on_pushButton_clicked()
{
	QString searchString = ui->lineEdit->text();
	loadFromSearch(searchString,QString(),Q_NULLPTR,true);
}

TreeItem *MainWindow::loadFromSearch(QString query, QString display, TreeItem *childOf, bool select)
{
	QRegularExpression re("^(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?(\\w+)(?:\\/thread)?\\/(\\d+)(?:#p\\d+)?$",QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match = re.match(query);
	QRegularExpressionMatch match2;
	BoardTab *bt;
	if (!match.hasMatch()) {
		QRegularExpression res("^(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?(\\w+)\\/(?:catalog#s=)?(.+)?$",QRegularExpression::CaseInsensitiveOption);
		match2 = res.match(query);
	}
	else{
		TreeItem *tnParent;
		if(childOf == Q_NULLPTR)tnParent = model->root;
		else tnParent = childOf;
		return onNewThread(this,match.captured(1),match.captured(2),display,tnParent);
	}
	if(match2.hasMatch()) {
		bt = new BoardTab(match2.captured(1),BoardType::Catalog,match2.captured(2),this);
		if(!display.length()) display = "/"+match2.captured(1)+"/"+match2.captured(2);
	}
	else{
		bt = new BoardTab(query,BoardType::Index,"",this);
		if(!display.length()) display = "/"+query+"/";
	}
	qDebug().noquote() << "loading" << display;
	ui->stackedWidget->addWidget(bt);

	QList<QVariant> list;
	list.append(display);
	TreeItem *tnParent;
	if(childOf == Q_NULLPTR) tnParent = model->root;
	else tnParent = childOf;
	TreeItem *tnNew = new TreeItem(list,tnParent,bt, TreeItemType::board);
	tnNew->query = query;
	tnNew->display = display;
	Tab tab = {Tab::TabType::Board,bt,tnNew,query,display};
	tabs.insert(bt,tab);
	addTab(tnNew,tnParent,select);
	return tnNew;
}

TreeItem *MainWindow::onNewThread(QWidget *parent, QString board, QString thread,
								  QString display, TreeItem *childOf)
{
	(void)parent;
	qDebug().noquote().nospace()  << "loading /" << board << "/" << thread;
	ThreadTab *tt = new ThreadTab(board,thread,this);
	ui->stackedWidget->addWidget(tt);
	QString query = "/"+board+"/"+thread;
	if(!display.length()) display = query;
	QList<QVariant> list;
	list.append(display);

	TreeItem *tnNew = new TreeItem(list,model->root,tt, TreeItemType::thread);
	tnNew->query = query;
	tnNew->display = display;
	Tab tab = {Tab::TabType::Thread,tt,tnNew,query,display};
	tabs.insert(tt,tab);
	addTab(tnNew,childOf,false);
	return tnNew;
}

void MainWindow::addTab(TreeItem *child, TreeItem *parent, bool select)
{
	if(parent == Q_NULLPTR || parent == model->root) {
		model->addParent(child);
	}
	else{
		QModelIndex ind = model->getIndex(parent);
		model->addChild(ind,child);
	}
	if(select) {
		QModelIndex ind = model->getIndex(child);
		selectionModel->setCurrentIndex(ind,QItemSelectionModel::ClearAndSelect);
	}
}

void MainWindow::on_treeView_clicked(QModelIndex index)
{
	QWidget *tab = model->getItem(index)->tab;
	ui->stackedWidget->setCurrentWidget(tab);
	this->setWindowTitle(tab->windowTitle());
}

void MainWindow::onSelectionChanged()
{
	QModelIndexList list = selectionModel->selectedIndexes();
	if(list.size()) {
		QWidget *tab = model->getItem(list.at(0))->tab;
		ui->stackedWidget->setCurrentWidget(tab);
		this->setWindowTitle(tab->windowTitle());
	}
	QCoreApplication::processEvents();
}

//TODO? replace with regular QAction shortcuts
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		int key = keyEvent->key();
		//int mod = keyEvent->modifiers();
		//qDebug("Ate key press %d", key);
		//qDebug("Modifers %d", mod);
		if(key == 53) {
			/*const QModelIndexList indexList = ui->treeView->selectionModel()->selectedRows();
			int row = indexList.at(0).row();
			qDebug() << (Tab::TabType)tabs.at(row).type;
			((Tab::TabType)tabs.at(row).type) == Tab::TabType::Board ? ((BoardTab*)tabs.at(row).TabPointer)->updatePosts() :
														  ((ThreadTab*)tabs.at(row).TabPointer)->updatePosts();*/
		}
		else if(key == 16777269) {
			ui->lineEdit->setFocus();
		}
		else if(key == 16777266) {
			qDebug("setting focus");
			ui->treeView->setFocus();
		}
		else if(key == 16777267) {
			ui->scrollAreaWidgetContents->setFocus();
		}
		else {
			return QObject::eventFilter(obj, event);
		}
		return true;
	} else {
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}

//TODO make non-recursive version
void MainWindow::deleteSelected()
{
	QCoreApplication::processEvents();
	QModelIndexList indexList = selectionModel->selectedRows();
	QModelIndex ind;
	if(!indexList.size() && selectionModel->currentIndex().isValid()) {
		indexList.clear();
		indexList.append(selectionModel->currentIndex());
	}
	while(indexList.size()) {
		removeTabs(model->getItem(indexList.first()));
		indexList.pop_front();
	}
	QCoreApplication::processEvents();
	ind = selectionModel->currentIndex();
	if(ind.isValid())
		selectionModel->setCurrentIndex(ind,QItemSelectionModel::ClearAndSelect);
}

void MainWindow::removeTabs(TreeItem *tn) {
	if(tn == model->root) return;
	while(tn->childCount()) {
		removeTabs(tn->child(0));
	}
	model->removeItem(tn);
	ui->stackedWidget->removeWidget(tn->tab);
	tabs.remove(tn->tab);
	tn->tab->disconnect();
	tn->tab->deleteLater();
	delete tn;
	if(!ui->stackedWidget->count())
		this->setWindowTitle("qtchan");
}

QObject *MainWindow::currentWidget()
{
	return ui->stackedWidget->currentWidget();
}

void MainWindow::on_lineEdit_returnPressed()
{
	on_pushButton_clicked();
	ui->treeView->setFocus();
}

void MainWindow::focusTree()
{
	ui->treeView->setFocus();
}

void MainWindow::focusBar()
{
	ui->lineEdit->setFocus();
	ui->lineEdit->selectAll();
}

void MainWindow::saveSession()
{
	qDebug().noquote() << "Saving session.";
	QSettings settings;
	saveSessionToFile(settings.value("sessionFile","settings.txt").toString());
}

void MainWindow::saveSessionToFile(QString fileName)
{
	QFile data(fileName);
	data.open(static_cast<QFile::OpenMode>(QFile::WriteOnly | QFile::Truncate));
	QTextStream out(&data);
	QList<TreeItem*> parents;
	QList<int> lines;
	parents << model->root;
	lines << 0;
	int i;
	TreeItem *parent;
	TreeItem *child;
	QString indent = "\t";
	while(!parents.isEmpty()) {
		parent = parents.last();
		i = lines.last();
		while(i<parent->childCount()) {
			child = parent->child(i);
			out << indent.repeated(lines.size()-1);
			out << child->query << "\t" << child->display << endl;
			i++;
			lines.last()++;
			if(child->childCount()) {
				parents << child;
				lines << 0;
				break;
			}
		}
		while (!parents.isEmpty() && parents.last()->childCount() == lines.last()) {
			lines.pop_back();
			parents.pop_back();
		}
	}
}

void MainWindow::loadSession()
{
	QSettings settings;
	QString sessionFile = settings.value("sessionFile","settings.txt").toString();
	loadSessionFromFile(sessionFile);
}

void MainWindow::loadSessionFromFile(QString sessionFile)
{
	QFile session(sessionFile);
	session.open(QFile::ReadOnly);
	QTextStream in(&session);
	QString line;
	QList<TreeItem*> parents;
	parents.append(model->root);
	QList<int> indents;
	indents << 0;
	int position;
	while(!in.atEnd()) {
		line = in.readLine();
		if(line.isEmpty())continue;
		position = 0;
		while(position < line.length()) {
			if(line.at(position) != '\t') break;
			position++;
		}
		line = line.mid(position).trimmed();
		if(line.isEmpty())continue;
		QStringList columns = line.split("\t", QString::SkipEmptyParts);
		if(columns.size() == 1) columns.append(columns.at(0));
		if(position > indents.last()) {
			if(parents.last()->childCount() > 0) {
				parents << parents.last()->child(parents.last()->childCount()-1);
				indents << position;
			}
		}
		else{
			while(position < indents.last() && parents.count() > 0) {
				parents.pop_back();
				indents.pop_back();
			}
		}
		loadFromSearch(columns.at(0),columns.at(1),parents.last(),false);
	}
}
