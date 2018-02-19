#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "boardtab.h"
#include "threadtab.h"
#include "threadform.h"
#include "notificationview.h"
#include <QFile>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWidget>
#include <QStandardItem>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSettings>
#include <QShortcut>
#include <QDesktopServices>

//TODO decouple item model/view logic to another class
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->splitter->setStretchFactor(0,0);
	ui->splitter->setStretchFactor(1,1);
	QList<int> sizes;
	sizes << 200 << (ui->splitter->size().width() - 200);
	ui->splitter->setSizes(sizes);
	ui->pushButton->hide();
	ui->navBar->hide();
	ui->treeView->setModel(model);
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	int fontSize = settings.value("fontSize",14).toInt();
	QFont temp = ui->treeView->font();
	temp.setPointSize(fontSize);
	ui->treeView->setFont(temp);
	ui->navBar->setFont(temp);
	selectionModel = ui->treeView->selectionModel();
	selectionConnection = connect(selectionModel,&QItemSelectionModel::selectionChanged,this,
								  &MainWindow::onSelectionChanged, Qt::UniqueConnection);
	settingsView.setParent(this,Qt::Tool
						   | Qt::WindowMaximizeButtonHint
						   | Qt::WindowCloseButtonHint);
	connect(&settingsView,&Settings::update,[=](QString field, QVariant value){
		if(field == "use4chanPass" && value.toBool() == true){
			QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
			QString defaultCookies = QDir::homePath() + "/.config/qtchan/cookies";
			nc.loadCookies(settings.value("passFile",defaultCookies).toString());
		}
	});
	connect(ui->treeView,&TreeView::treeMiddleClicked,model,&TreeModel::removeTab,Qt::DirectConnection);
	connect(ui->treeView,&TreeView::hideNavBar,ui->navBar,&QWidget::hide,Qt::DirectConnection);
	connect(model,&TreeModel::selectTab,ui->treeView,&TreeView::selectTab,Qt::DirectConnection);
	connect(model,&TreeModel::loadFromSearch,this,&MainWindow::loadFromSearch);
	connect(model,&TreeModel::removingTab,this,&MainWindow::onRemoveTab);
	this->setShortcuts();
}

void MainWindow::onRemoveTab(TreeItem* tn){
	tn->tab->deleteLater();
	//ui->content->removeWidget(tn->tab);
	tabs.remove(tn->tab);
	QWidget *cw = currentWidget();
	if(!cw) return;
	Tab current = tabs.value(cw);
	if(!current.tn) return;
	QModelIndex ind = model->getIndex(current.tn);
	if(ind != ui->treeView->rootIndex())
		selectionModel->setCurrentIndex(ind,QItemSelectionModel::ClearAndSelect);
}

void MainWindow::setShortcuts()
{
	//hiding the menu bar disables other qmenu actions shortcuts
	QAction *toggleMenuBar = new QAction(this);
	toggleMenuBar->setShortcut(QKeySequence("F11"));
	toggleMenuBar->setShortcutContext(Qt::ApplicationShortcut);
	connect(toggleMenuBar, &QAction::triggered, [=]{
		ui->menuBar->isHidden() ? ui->menuBar->show() : ui->menuBar->hide();
	});
	this->addAction(toggleMenuBar);

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
	closeTab->setShortcut(QKeySequence("Ctrl+W"));
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

	//TODO select session saves slots
	QAction *saveState = new QAction(this);
	saveState->setShortcut(QKeySequence(Qt::Key_F10));
	saveState->setShortcutContext(Qt::ApplicationShortcut);
	connect(saveState, &QAction::triggered, this, &MainWindow::saveSession);
	this->addAction(saveState);

	//TODO select session saves slots
	QAction *loadState = new QAction(this);
	loadState->setShortcut(QKeySequence(Qt::Key_F12));
	loadState->setShortcutContext(Qt::ApplicationShortcut);
	connect(loadState, &QAction::triggered, this, &MainWindow::loadSession);
	this->addAction(loadState);

	QAction *openExplorer = new QAction(this);
	openExplorer->setShortcut(QKeySequence("Ctrl+o"));
	openExplorer->setShortcutContext(Qt::ApplicationShortcut);
	connect(openExplorer, &QAction::triggered, this, &MainWindow::openExplorer);
	this->addAction(openExplorer);

	QAction *zoomOut = new QAction(this);
	zoomOut->setShortcut(QKeySequence::ZoomOut);
	connect(zoomOut, &QAction::triggered, [=](){
		qDebug() << "decreasing text size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int fontSize = settings.value("fontSize",14).toInt()-2;
		if(fontSize < 4) fontSize = 4;
		settings.setValue("fontSize",fontSize);
		QFont temp = ui->treeView->font();
		temp.setPointSize(fontSize);
		ui->treeView->setFont(temp);
		ui->navBar->setFont(temp);
		emit setFontSize(fontSize);
	});
	this->addAction(zoomOut);

	QAction *zoomIn = new QAction(this);
	zoomIn->setShortcut(QKeySequence::ZoomIn);
	connect(zoomIn, &QAction::triggered, [=](){
		qDebug() << "increasing text size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int fontSize = settings.value("fontSize",14).toInt()+2;
		settings.setValue("fontSize",fontSize);
		QFont temp = ui->treeView->font();
		temp.setPointSize(fontSize);
		ui->treeView->setFont(temp);
		emit setFontSize(fontSize);
	});
	this->addAction(zoomIn);

	QAction *scaleImagesDown = new QAction(this);
	scaleImagesDown->setShortcut(QKeySequence("Ctrl+9"));
	connect(scaleImagesDown, &QAction::triggered, [=](){
		qDebug() << "decreasing image size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int imageSize = settings.value("imageSize",250).toInt()-25;
		if(imageSize < 25) imageSize = 25;
		settings.setValue("imageSize",imageSize);
		emit setImageSize(imageSize);
	});
	this->addAction(scaleImagesDown);

	QAction *scaleImagesUp = new QAction(this);
	scaleImagesUp->setShortcut(QKeySequence("Ctrl+0"));
	connect(scaleImagesUp, &QAction::triggered, [=](){
		qDebug() << "increasing image size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int imageSize = settings.value("imageSize",250).toInt()+25;
		settings.setValue("imageSize",imageSize);
		emit setImageSize(imageSize);
	});
	this->addAction(scaleImagesUp);

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
	ui->actionSettings->setShortcutContext(Qt::ApplicationShortcut);
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

	ui->actionExit->setShortcut(QKeySequence("Ctrl+q"));
	ui->actionExit->setShortcutContext(Qt::ApplicationShortcut);
	connect(ui->actionExit,&QAction::triggered,this,&QApplication::quit);

	QAction *sizeUp = new QAction(this);
	sizeUp->setShortcut(QKeySequence("Ctrl+8"));
	connect(sizeUp,&QAction::triggered,[=]{
		qDebug() << QString::number(qgetenv("QT_SCALE_FACTOR").toInt()+1).toStdString().c_str();
		qputenv("QT_SCALE_FACTOR",QString::number(qgetenv("QT_SCALE_FACTOR").toInt()+1).toStdString().c_str());
	});
	sizeUp->setShortcutContext(Qt::ApplicationShortcut);
	this->addAction(sizeUp);

	ui->actionReloadFilters->setShortcut(QKeySequence("F7"));
	ui->actionReloadFilters->setShortcutContext(Qt::ApplicationShortcut);
	connect(ui->actionReloadFilters,&QAction::triggered,[=](){
		emit reloadFilters();
	});

	QAction *focusSearch = new QAction(this);
	focusSearch->setShortcut(QKeySequence("Ctrl+f"));
	focusSearch->setShortcutContext(Qt::ApplicationShortcut);
	connect(focusSearch,&QAction::triggered,[=]{
		TreeItem *item = model->getItem(selectionModel->currentIndex());
		if(!item) return;
		if(item->type == TreeItemType::thread){
			ThreadTab *tab = static_cast<ThreadTab*>(item->tab);
			tab->focusIt();
		}
		else{
			BoardTab *tab = static_cast<BoardTab*>(item->tab);
			tab->focusIt();
		}
	});
	this->addAction(focusSearch);

	QAction *focusTree = new QAction(this);
	focusTree->setShortcut(Qt::Key_F3);
	focusTree->setShortcutContext(Qt::ApplicationShortcut);
	connect(focusTree,&QAction::triggered,this,&MainWindow::focusTree);
	this->addAction(focusTree);

	QAction *focusContent = new QAction(this);
	focusContent->setShortcut(Qt::Key_F4);
	focusContent->setShortcutContext(Qt::ApplicationShortcut);
	connect(focusContent,&QAction::triggered,[=]{
		if(QWidget *temp = currentWidget()) temp->setFocus();
	});
	this->addAction(focusContent);

	QAction *focusNavBar = new QAction(this);
	focusNavBar->setShortcut(Qt::Key_F6);
	focusNavBar->setShortcutContext(Qt::ApplicationShortcut);
	connect(focusNavBar, &QAction::triggered, this, &MainWindow::focusBar);
	this->addAction(focusNavBar);

	QAction *closeChildTabs = new QAction(this);
	closeChildTabs->setShortcut(QKeySequence("ctrl+k"));
	closeChildTabs->setShortcutContext(Qt::ApplicationShortcut);
	connect(closeChildTabs,&QAction::triggered,this,&MainWindow::removeChildTabs);
	this->addAction(closeChildTabs);

	//TODO clean-up and fix focus back to mainwindow
	QAction *toggleNotifications = new QAction(this);
	toggleNotifications->setShortcut(Qt::Key_F9);
	toggleNotifications->setShortcutContext(Qt::ApplicationShortcut);
	connect(toggleNotifications,&QAction::triggered,[=]{
		if(nv->isVisible()){
			nv->hide();
		}
		else{
			nv->move(nv->toMove());
			nv->show();
		}
	});
	this->addAction(toggleNotifications);

	QAction *hideNavBar = new QAction(this);
	hideNavBar->setShortcut(Qt::Key_Escape);
	hideNavBar->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(hideNavBar,&QAction::triggered,ui->navBar,&QLineEdit::hide,Qt::DirectConnection);
	this->addAction(hideNavBar);
}

MainWindow::~MainWindow()
{
	disconnect(selectionConnection);
	saveSession();
	you.saveYou();
	delete ui;
	delete model;
	Chans::deleteAPIs();
}

//TODO put toggle functions in 1 function with argument
void MainWindow::toggleAutoUpdate()
{
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	bool autoUpdate = !settings.value("autoUpdate").toBool();
	qDebug () << "setting autoUpdate to" << autoUpdate;
	settings.setValue("autoUpdate",autoUpdate);
	emit setAutoUpdate(autoUpdate);
	settingsView.refreshValues();
}

void MainWindow::toggleAutoExpand()
{
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	bool autoExpand = !settings.value("autoExpand").toBool();
	qDebug () << "setting autoExpand to" << autoExpand;
	settings.setValue("autoExpand",autoExpand);
	emit setAutoExpand(autoExpand);
	settingsView.refreshValues();
}

void MainWindow::updateSettings(QString field, QVariant value){
	if(field == "autoUpdate")
		emit setAutoUpdate(value.toBool());
	else if(field == "autoExpand")
		emit setAutoExpand(value.toBool());
	else if(field == "use4chanPass"){
		emit setUse4chanPass(value.toBool());
		if(value.toBool()){
			QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
			QString defaultCookies = QDir::homePath() + "/.config/qtchan/cookies";
			nc.loadCookies(settings.value("passFile",defaultCookies).toString());
		}
		else{
			nc.removeCookies();
		}
	}
}

void MainWindow::openExplorer(){
	QString url;
	if(!ui->content->count()) return;
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
	QString searchString = ui->navBar->text();
	ui->navBar->hide();
	loadFromSearch(searchString,QString(),Q_NULLPTR,true);
}

TreeItem *MainWindow::loadFromSearch(QString query, QString display, TreeItem *childOf, bool select)
{
	Chan *api = Chans::stringToType(query);
	QRegularExpression re(api->regToThread(),QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match = re.match(query);
	QRegularExpressionMatch match2;
	BoardTab *bt;
	if (!match.hasMatch()) {
		QRegularExpression res(api->regToCatalog(),QRegularExpression::CaseInsensitiveOption);
		match2 = res.match(query);
	}
	else{
		TreeItem *tnParent;
		if(childOf == Q_NULLPTR)tnParent = model->root;
		else tnParent = childOf;
		return onNewThread(this,api,match.captured(1),match.captured(2),display,tnParent);
	}
	if(match2.hasMatch()) {
		bt = new BoardTab(api, match2.captured(1),BoardType::Catalog,match2.captured(2),this);
		if(!display.length()) display = "/"+match2.captured(1)+"/"+match2.captured(2);
	}
	else{
		bt = new BoardTab(api, query,BoardType::Index,"",this);
		if(!display.length()) display = "/"+query+"/";
	}
	qDebug().noquote() << "loading" << display;
	ui->content->addWidget(bt);

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
	model->addTab(tnNew,tnParent,select);
	return tnNew;
}

TreeItem *MainWindow::onNewThread(QWidget *parent, Chan *api, QString board, QString thread,
								  QString display, TreeItem *childOf)
{
	(void)parent;
	qDebug().noquote().nospace()  << "loading /" << board << "/" << thread;
	QString query = "/"+board+"/"+thread;
	if(!display.length()) display = query;
	bool isFromSession = (display == query) ? false : true;
	ThreadTab *tt = new ThreadTab(api,board,thread,this,isFromSession);
	ui->content->addWidget(tt);
	QList<QVariant> list;
	list.append(display);

	TreeItem *tnNew = new TreeItem(list,model->root,tt, TreeItemType::thread);
	tnNew->query = query;
	tnNew->display = display;
	tt->tn = tnNew;
	Tab tab = {Tab::TabType::Thread,tt,tnNew,query,display};
	tabs.insert(tt,tab);
	connect(tt,&ThreadTab::unseen,this,&MainWindow::updateSeen);
	model->addTab(tnNew,childOf,false);
	return tnNew;
}

void MainWindow::updateSeen(int formsUnseen){
	TreeItem *tn = static_cast<ThreadTab*>(sender())->tn;
	tn->setData(1,formsUnseen);
	tn->unseen = formsUnseen;
}

void MainWindow::onSelectionChanged()
{
	QModelIndexList list = ui->treeView->selected();
	if(list.size()) {
		QPointer<QWidget> tab = model->getItem(list.at(0))->tab;
		if(tab){
			ui->content->setCurrentWidget(tab);
			setWindowTitle(tab->windowTitle());
		}
	}
	else setWindowTitle("qtchan");
}

void MainWindow::deleteSelected()
{
	if(!tabs.size()) return;
	QModelIndexList list = ui->treeView->selected();
	if(!list.size()){
		Tab tab = tabs.value(ui->content->currentWidget());
		if(tab.tn) tab.tn->deleteLater();
	}
	else foreach(QModelIndex index, list) {
		model->removeTab(index);
	}
}

void MainWindow::removeChildTabs(){
	foreach(QModelIndex index, ui->treeView->selected()) {
		model->removeChildren(index);
	}
}

QWidget *MainWindow::currentWidget()
{
	if(tabs.size() && ui->content->count())
		return ui->content->currentWidget();
	else return Q_NULLPTR;
}

void MainWindow::on_navBar_returnPressed()
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
	if(ui->tst->isHidden()) ui->tst->show();
	if(ui->navBar->isHidden()) ui->navBar->show();
	ui->navBar->setFocus();
	ui->navBar->selectAll();
}

void MainWindow::saveSession()
{
	qDebug().noquote() << "Saving session.";
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	model->saveSessionToFile(settings.value("sessionFile","session.txt").toString());
}

void MainWindow::loadSession()
{
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	QString sessionFile = settings.value("sessionFile","session.txt").toString();
	model->loadSessionFromFile(sessionFile);
}
