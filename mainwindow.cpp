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

	ui->pushButton->hide();
	ui->navBar->hide();
	ui->treeView->setModel(model);
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	int fontSize = settings.value("fontSize",14).toInt();
	QFont temp = ui->treeView->font();
	temp.setPointSize(fontSize);
	ui->treeView->setFont(temp);
	ui->navBar->setFont(temp);

	//instructions
	QFile file(":/readstartup.md");
	if(file.open(QIODevice::ReadOnly)){
		QByteArray dump = file.readAll();
		file.close();
		ui->help->setFont(temp);
		ui->help->setText(QString(dump));
	}
	QList<int> sizes;
	sizes << 150 << this->width()-150;
	ui->splitter->setSizes(sizes);

	selectionModel = ui->treeView->selectionModel();
	selectionConnection = connect(selectionModel,&QItemSelectionModel::selectionChanged,this,
								  &MainWindow::onSelectionChanged, Qt::UniqueConnection);
	settingsView.setParent(this,Qt::Tool
						   | Qt::WindowMaximizeButtonHint
						   | Qt::WindowCloseButtonHint);
	connect(&settingsView,&Settings::update,[=](QString field, QVariant value){
		if(field == "use4chanPass" && value.toBool() == true){
			QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
			QString defaultCookies = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/cookies";
			nc.loadCookies(settings.value("passFile",defaultCookies).toString());
		}
	});
	connect(ui->treeView,&TreeView::treeMiddleClicked,model,&TreeModel::removeTab,Qt::DirectConnection);
	connect(ui->treeView,&TreeView::hideNavBar,ui->navBar,&QWidget::hide,Qt::DirectConnection);
	connect(model,&TreeModel::selectTab,ui->treeView,&TreeView::selectTab,Qt::DirectConnection);
	connect(model,&TreeModel::loadFromSearch,this,&MainWindow::loadFromSearch);
	connect(model,&TreeModel::removingTab,this,&MainWindow::onRemoveTab);
	connect(&aTab,&ArchiveTab::loadThread,[=](QString threadString){
		loadFromSearch(threadString,QString(),Q_NULLPTR,true);
	});
	this->setShortcuts();
}

void MainWindow::closeEvent(QCloseEvent *event){
	(void)event;
	QApplication::closeAllWindows();
}

void MainWindow::onRemoveTab(TreeItem* tn){
	tn->tab->deleteLater();
	tabs.remove(tn->tab);
	QWidget *cw = currentWidget();
	if(!cw) return;
	Tab current = tabs.value(cw);
	if(!current.tn) return;
	QModelIndex ind = model->getIndex(current.tn);
	if(ind != ui->treeView->rootIndex()){
		selectionModel->setCurrentIndex(ind,QItemSelectionModel::ClearAndSelect);
		ui->treeView->setCurrentIndex(ind);
	}
}

void MainWindow::setShortcuts()
{
	//hiding the menu bar disables other qmenu actions shortcuts
	addShortcut(Qt::Key_F11,this,[=]{
		ui->menuBar->isHidden() ? ui->menuBar->show() : ui->menuBar->hide();
	});

	addShortcut(QKeySequence("ctrl+shift+tab"),this,&MainWindow::prevTab);
	addShortcut(QKeySequence("ctrl+tab"),this,&MainWindow::nextTab);

	addShortcut(QKeySequence("ctrl+1"),this, [=]{
		if(!model->rowCount()) return;
		selectionModel->setCurrentIndex(model->index(0,0),
										QItemSelectionModel::ClearAndSelect);
		ui->treeView->setCurrentIndex(model->index(0,0));
	});

	addShortcut(QKeySequence("ctrl+2"),this,&MainWindow::prevParent);
	addShortcut(QKeySequence("ctrl+3"),this,&MainWindow::nextParent);

	addShortcut(QKeySequence("ctrl+4"),this,[=]{
		if(!model->rowCount()) return;
		TreeItem *tm = model->getItem(model->index(model->rowCount(),0));
		while(tm->childCount()){
			tm = tm->child(tm->childCount()-1);
		}
		QModelIndex qmi = model->getIndex(tm);
		selectionModel->setCurrentIndex(qmi,
										QItemSelectionModel::ClearAndSelect);
		ui->treeView->setCurrentIndex(qmi);
	});

	addShortcut(QKeySequence::Delete, this, &MainWindow::deleteSelected,
				Qt::AutoConnection,Qt::WindowShortcut);
	addShortcut(QKeySequence("ctrl+w"),this,&MainWindow::deleteSelected);
	addShortcut(QKeySequence("Ctrl+l"),this,&MainWindow::focusBar);
	addShortcut(QKeySequence("Ctrl+u"),this,&MainWindow::toggleAutoUpdate);
	addShortcut(QKeySequence("Ctrl+e"),this,&MainWindow::toggleAutoExpand);

	addShortcut(QKeySequence("ctrl+o"),this,&MainWindow::openExplorer);

	addShortcut(QKeySequence::ZoomOut,this,[=](){
		qDebug() << "decreasing text size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int fontSize = settings.value("fontSize",14).toInt()-2;
		if(fontSize < 4) fontSize = 4;
		settings.setValue("fontSize",fontSize);
		QFont temp = ui->treeView->font();
		temp.setPointSize(fontSize);
		ui->treeView->setFont(temp);
		ui->navBar->setFont(temp);
		ui->help->setFont(temp);
		emit setFontSize(fontSize);
	});

	addShortcut(QKeySequence::ZoomIn,this,[=](){
		qDebug() << "increasing text size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int fontSize = settings.value("fontSize",14).toInt()+2;
		settings.setValue("fontSize",fontSize);
		QFont temp = ui->treeView->font();
		temp.setPointSize(fontSize);
		ui->treeView->setFont(temp);
		ui->navBar->setFont(temp);
		ui->help->setFont(temp);
		emit setFontSize(fontSize);
	});

	addShortcut(QKeySequence("ctrl+9"),this,[=](){
		qDebug() << "decreasing image size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int imageSize = settings.value("imageSize",250).toInt()-25;
		if(imageSize < 25) imageSize = 25;
		settings.setValue("imageSize",imageSize);
		emit setImageSize(imageSize);
	});

	addShortcut(QKeySequence("ctrl+0"),this,[=](){
		qDebug() << "increasing image size";
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int imageSize = settings.value("imageSize",250).toInt()+25;
		settings.setValue("imageSize",imageSize);
		emit setImageSize(imageSize);
	});

	ui->actionSave->setShortcut(QKeySequence("Ctrl+s"));
	ui->actionSave->setShortcutContext(Qt::ApplicationShortcut);
	connect(ui->actionSave,&QAction::triggered,[=]{saveSession();});

	ui->actionReload->setShortcut(QKeySequence("Ctrl+r"));
	connect(ui->actionReload,&QAction::triggered,[=]{
		QMapIterator<QWidget*,Tab> i(tabs);
		while(i.hasNext()) {
			Tab tab = i.next().value();
			if(tab.type == Tab::TabType::Board) static_cast<BoardTab*>(tab.TabPointer)->getPosts();
			else static_cast<ThreadTab*>(tab.TabPointer)->getPosts();
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

	ui->actionReloadFilters->setShortcut(Qt::Key_F7);
	ui->actionReloadFilters->setShortcutContext(Qt::ApplicationShortcut);
	connect(ui->actionReloadFilters,&QAction::triggered,[=](){
		filter.loadFilterFile2();
		emit reloadFilters();
	});
	addShortcut(QKeySequence("ctrl+f"),this,[=]{
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

	addShortcut(QKeySequence("ctrl+k"),this,&MainWindow::removeChildTabs);

	//TODO clean-up and fix focus back to mainwindow
	addShortcut(Qt::Key_F9,this,[=]{
		if(nv->isVisible()){
			nv->hide();
			activateWindow();
		}
		else{
			nv->reAdjust();
			nv->show();
		}
	});

	addShortcut(Qt::Key_Escape,ui->navBar,&QLineEdit::hide,
				Qt::DirectConnection,Qt::WidgetWithChildrenShortcut);
	addShortcut(Qt::Key_F1,this,&MainWindow::showHelp);
	addShortcut(Qt::Key_F3,this,&MainWindow::focusTree);
	addShortcut(Qt::Key_F4,this,[=]{
		if(QWidget *temp = currentWidget()) temp->setFocus();
	});
	//addShortcut(Qt::Key_F6,this,&MainWindow::focusBar);
	addShortcut(Qt::Key_F8,&aTab,&ArchiveTab::show);

	//session shortcuts
	for(int i=Qt::Key_F1, j=0; i<=Qt::Key_F4; i++, j++){
		QAction *saveShortcut = new QAction(this);
		saveShortcut->setShortcut(QKeySequence(Qt::ControlModifier+i));
		connect(saveShortcut, &QAction::triggered,[=]{
			qDebug() << "saving shortcut";
			saveSession(QString::number(j));
		});
		this->addAction(saveShortcut);
		QAction *loadShortcut = new QAction(this);
		loadShortcut->setShortcut(QKeySequence(Qt::ShiftModifier+i));
		connect(loadShortcut, &QAction::triggered,[=]{
			loadSession(QString::number(j));
		});
		this->addAction(loadShortcut);
	}
	addShortcut(Qt::Key_F5,this,[=]{saveSession();});
	addShortcut(Qt::Key_F6,this,[=]{loadSession();});
	addShortcut(QKeySequence("ctrl+F5"),this,[=]{
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int slot = settings.value("sessionFileSlot",0).toInt();
		if(--slot == 0) slot = 9;
		settings.setValue("sessionFileSlot",slot);
		qDebug() << "current session slot:" << slot;
	});
	addShortcut(QKeySequence("ctrl+F6"),this,[=]{
		QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
		int slot = settings.value("sessionFileSlot",0).toInt();
		if(++slot == 10) slot = 0;
		settings.setValue("sessionFileSlot",slot);
		qDebug() << "current session slot:" << slot;
	});
}

void MainWindow::showHelp(){
	ui->content->setCurrentIndex(0);
	setWindowTitle("qtchan - help");
}

template<typename T, typename F>
void MainWindow::addShortcut(QKeySequence key,const T connectTo, F func,
							 Qt::ConnectionType type, Qt::ShortcutContext context){
	QAction *newAction = new QAction(this);
	newAction->setShortcut(key);
	newAction->setShortcutContext(context);
	connect(newAction,&QAction::triggered,connectTo,func,type);
	this->addAction(newAction);
}

MainWindow::~MainWindow()
{
	disconnect(selectionConnection);
	disconnect(model,&TreeModel::removingTab,this,&MainWindow::onRemoveTab);
	saveSession();
	you.saveYou(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/you");
	aTab.close();
	aTab.deleteLater();
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
			QString defaultCookies = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/cookies";
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
		url = QDir(tab->api->name()+'/'+tab->board+'/'+tab->thread).absolutePath();
	}
	else{
		BoardTab *tab = static_cast<BoardTab*>(item->tab);
		url = QDir(tab->api->name()+'/'+tab->board).absolutePath();
	}
	qDebug() << "Opening folder" << url;
	QDesktopServices::openUrl(QUrl::fromLocalFile(url));
}

void MainWindow::nextTab()
{
	QModelIndex qmi = ui->treeView->currentIndex();
	QKeyEvent event(QEvent::KeyPress,Qt::Key_Down,0);
	QApplication::sendEvent(ui->treeView, &event);
	if(qmi == ui->treeView->currentIndex()){
		qmi = model->index(0,0);
		selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
		ui->treeView->setCurrentIndex(qmi);
	}
}

void MainWindow::prevTab()
{
	QModelIndex qmi = ui->treeView->currentIndex();
	QKeyEvent event(QEvent::KeyPress,Qt::Key_Up,0);
	QApplication::sendEvent(ui->treeView, &event);
	if(qmi == ui->treeView->currentIndex()){
		qmi = model->index(model->rowCount()-1,0);
		while(model->hasChildren(qmi) && ui->treeView->isExpanded(qmi)) {
			qmi = qmi.child(model->rowCount(qmi)-1,0);
		}
		selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
		ui->treeView->setCurrentIndex(qmi);
	}
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
		ui->treeView->setCurrentIndex(qmi);
		return;
	}
	int row = qmi.row()-1;
	if(row == -1) row = rowCount-1;
	qmi = qmi.sibling(row,0);
	selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
	ui->treeView->setCurrentIndex(qmi);

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
	ui->treeView->setCurrentIndex(qmi);
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
	ui->treeView->setExpanded(model->getIndex(tnNew).parent(),true);
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
	ThreadTab *tt = new ThreadTab(api,board,thread,ui->content,isFromSession);
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
	ui->treeView->setExpanded(model->getIndex(childOf),true);
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
			currentTab = tab;
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

void MainWindow::saveSession(QString slot)
{
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	if(slot.isEmpty()) slot = settings.value("sessionSlot","0").toString();
	QString sessionPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/sessions/";
	QDir().mkpath(sessionPath);
	QString sessionFile = sessionPath + settings.value("sessionFile","session").toString();
	model->saveSessionToFile(sessionFile,slot,ui->treeView->currentIndex());
}

void MainWindow::loadSession(QString slot)
{
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	if(slot.isEmpty()) slot = settings.value("sessionSlot","0").toString();
	QString sessionFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/sessions/" + settings.value("sessionFile","session").toString();
	QModelIndex qmi = model->loadSessionFromFile(sessionFile,slot);
	selectionModel->setCurrentIndex(qmi,QItemSelectionModel::ClearAndSelect);
	ui->treeView->setCurrentIndex(qmi);
}
