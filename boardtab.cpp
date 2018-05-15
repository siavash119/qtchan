#include "boardtab.h"
#include "ui_boardtab.h"
#include "mainwindow.h"
#include <QSettings>
#include <QJsonArray>
#include <QJsonDocument>
#include <QScrollBar>
#include <QRegularExpressionMatch>

BoardTab::BoardTab(Chan *api, QString board, BoardType type, QString search, QWidget *parent) :
	QWidget(parent), api(api), board(board), type(type), search(search),
	ui(new Ui::BoardTab)
{
	ui->setupUi(this);
	ui->searchWidget->hide();
	//TODO check if actual board
	this->setWindowTitle("/"+board+"/"+search);
	if(type == BoardType::Index) boardUrl = api->boardURL(board);
	else boardUrl = api->catalogURL(board);
	helper.moveToThread(&workerThread);
	connect(&helper,&BoardTabHelper::newThread,this,&BoardTab::onNewThread,Qt::QueuedConnection);
	connect(&helper,&BoardTabHelper::newTF,this,&BoardTab::onNewTF,Qt::QueuedConnection);
	connect(&helper,&BoardTabHelper::clearMap,this,&BoardTab::clearMap,Qt::QueuedConnection);
	connect(&helper,&BoardTabHelper::removeTF,this,&BoardTab::removeTF,Qt::QueuedConnection);
	connect(&helper,&BoardTabHelper::showTF,this,&BoardTab::showTF,Qt::QueuedConnection);
	connect(mw,&MainWindow::reloadFilters,&helper,&BoardTabHelper::reloadFilters,Qt::DirectConnection);
	workerThread.start();
	myPostForm.setParent(this,Qt::Tool
						 | Qt::WindowMaximizeButtonHint
						 | Qt::WindowCloseButtonHint);
	myPostForm.load(api,board,"");
	connect(&myPostForm,&PostForm::loadThread,[=](QString threadNum){
		TreeItem *childOf = mw->model->getItem(mw->selectionModel->currentIndex());
		mw->onNewThread(mw,api,board,threadNum,QString(),childOf);
	});
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	QFont temp = ui->lineEdit->font();
	temp.setPointSize(settings.value("fontSize",14).toInt()-2);
	ui->label->setFont(temp);
	ui->lineEdit->setFont(temp);
	ui->pushButton->setFont(temp);
	this->setShortcuts();
	connect(mw,&MainWindow::setUse4chanPass,&myPostForm,&PostForm::usePass,Qt::QueuedConnection);
	connect(mw,&MainWindow::setFontSize,this,&BoardTab::setFontSize,Qt::QueuedConnection);
	connect(mw,&MainWindow::setImageSize,this,&BoardTab::setImageSize,Qt::QueuedConnection);
	helper.startUp(api,board,type,search,this);
}

void BoardTab::setFontSize(int fontSize){
	QFont temp = font();
	temp.setPointSize(fontSize);
	ui->label->setFont(temp);
	ui->lineEdit->setFont(temp);
	ui->pushButton->setFont(temp);
	myPostForm.setFontSize(fontSize);
	foreach(ThreadForm *tf, tfMap){
		tf->setFontSize(fontSize);
	}
}

void BoardTab::removeTF(ThreadForm *tf){
	tf->hide();
}

void BoardTab::showTF(ThreadForm *tf){
	tf->show();
}

void BoardTab::setImageSize(int imageSize){
	foreach(ThreadForm *tf, tfMap){
		tf->setImageSize(imageSize);
	}
}

BoardTab::~BoardTab()
{
	helper.abort = true;
	workerThread.quit();
	workerThread.wait();
	workerThread.terminate();
	delete ui;
	qDebug().noquote().nospace() << "deleting board /" << board+"/";
}

void BoardTab::setShortcuts()
{
	QAction *refresh = new QAction(this);
	refresh->setShortcut(Qt::Key_R);
	connect(refresh, &QAction::triggered, &helper, &BoardTabHelper::getPosts, Qt::DirectConnection);
	this->addAction(refresh);

	QAction *postForm = new QAction(this);
	postForm->setShortcut(Qt::Key_Q);
	connect(postForm, &QAction::triggered, this, &BoardTab::openPostForm);
	this->addAction(postForm);

	QAction *selectPost = new QAction(this);
	selectPost->setShortcut(Qt::Key_O);
	connect(selectPost, &QAction::triggered,[=]{
		QWidget *selected = ui->scrollAreaWidgetContents->childAt(50,ui->scrollArea->verticalScrollBar()->value());
		while(selected && selected->parent()->objectName() != "scrollAreaWidgetContents") {
			selected = qobject_cast<QWidget*>(selected->parent());
		}
		if(selected && selected->objectName() == "ThreadForm"){
			static_cast<ThreadForm*>(selected)->imageClicked();
		}
	});
	this->addAction(selectPost);

	QAction *scrollUp = new QAction(this);
	scrollUp->setShortcut(Qt::Key_K);
	connect(scrollUp, &QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		QScrollBar *bar = ui->scrollArea->verticalScrollBar();
		if(ThreadForm *tf = tfAtTop()){
			ThreadForm *toTf = Q_NULLPTR;
			QListIterator<QObject*> i(ui->scrollAreaWidgetContents->children());
			while(i.hasNext()){
				if(i.next() == tf) break;
			}
			if(i.hasPrevious()) i.previous();
			while(i.hasPrevious() && vimNumber--){
				QObject *temp = i.previous();
				if(temp->objectName() == "ThreadForm"){
					toTf = qobject_cast<ThreadForm*>(temp);
					if(toTf->hidden) vimNumber++;
				}
			}
			if(toTf) bar->setValue(toTf->pos().y());
		}
		vimCommand = "";
	});
	this->addAction(scrollUp);

	QAction *scrollDown = new QAction(this);
	scrollDown->setShortcut(Qt::Key_J);
	connect(scrollDown, &QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		QScrollBar *bar = ui->scrollArea->verticalScrollBar();
		if(ThreadForm *tf = tfAtTop()){
			ThreadForm *toTf = Q_NULLPTR;
			QListIterator<QObject*> i(ui->scrollAreaWidgetContents->children());
			while(i.hasNext()){
				if(i.next() == tf) break;
			}
			while(i.hasNext() && vimNumber--){
				QObject *temp = i.next();
				if(temp->objectName() == "ThreadForm"){
					toTf = qobject_cast<ThreadForm*>(temp);
					if(toTf->hidden) vimNumber++;
				}

			}
			if(toTf) bar->setValue(((QWidget*)(toTf))->pos().y());
		}
		vimCommand = "";
	});
	this->addAction(scrollDown);

	QAction *scrollPercent = new QAction(this);
	scrollPercent->setShortcut(QKeySequence("Shift+G"));
	connect(scrollPercent, &QAction::triggered,[=]{
		int vimNumber = 100;
		if(!vimCommand.isEmpty())vimNumber = vimCommand.toInt();
		ui->scrollArea->verticalScrollBar()->setValue(ui->scrollAreaWidgetContents->height()*vimNumber/100);
		vimCommand = "";
	});
	this->addAction(scrollPercent);

	QAction *clearVim = new QAction(this);
	clearVim->setShortcut(Qt::Key_Minus);
	connect(clearVim, &QAction::triggered,[=]{
		vimCommand = "";
	});
	this->addAction(clearVim);

	for(int i = Qt::Key_0; i<=Qt::Key_9; i++){
		QAction *numberPressed = new QAction(this);
		numberPressed->setShortcut(i);
		connect(numberPressed, &QAction::triggered,this,&BoardTab::updateVim);
		this->addAction(numberPressed);
	}
}

void BoardTab::updateVim(){
	 QAction* action = qobject_cast<QAction*>(sender());
	 QKeySequence seq = action->shortcut();
	 vimCommand += seq.toString();
}

void BoardTab::openPostForm()
{
	myPostForm.show();
	myPostForm.activateWindow();
	myPostForm.raise();
}

//TODO, just search the whole JSON and find posts
void BoardTab::findText(const QString text)
{
	qDebug().noquote() << "searching " + text;
	if(text.isEmpty()){
		ui->searchWidget->hide();
		QMapIterator<QString,ThreadForm*> mapI(tfMap);
		while (mapI.hasNext()) {
			mapI.next();
			mapI.value()->show();
		}
		return;
	}
	QString temp(text);
	QRegularExpression re(temp.replace("\n",""),QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match;
	ThreadForm *tf;
	QMapIterator<QString,ThreadForm*> mapI(tfMap);
	while (mapI.hasNext()) {
		mapI.next();
		tf = mapI.value();
		QString toMatch(tf->matchThis());
		toMatch = Filter::toStrippedHtml(toMatch);
		match = re.match(toMatch);
		if(match.hasMatch()){
			if(!tf->hidden) tf->show();
		}
		else tf->hide();
	}
}

/*
void BoardTab::addStretch()
{
	ui->threads->removeItem(&space);
	ui->threads->insertItem(-1,&space);
}*/

void BoardTab::onNewTF(ThreadForm *tf, ThreadForm *thread)
{
	connect(tf,&ThreadForm::removeMe,tf,&ThreadForm::deleteLater,Qt::QueuedConnection);
	thread->addReply(tf);
	if(this == mw->currentTab) QCoreApplication::processEvents();
}

void BoardTab::onNewThread(ThreadForm *tf)
{
	if(tf->hidden){
		qDebug().noquote().nospace() << tf->post.no << " filtered from /" << board << "/!";
		tf->hide();
	}
	connect(tf,&ThreadForm::removeMe,[=]{
		filter.addFilter2("no",tf->post.no,"boards:"+board);
		tf->hide();
	});
	ui->threads->addWidget(tf);
	tfMap.insert(tf->post.no,tf);
	if(this == mw->currentTab) QCoreApplication::processEvents();
}

ThreadForm* BoardTab::tfAtTop(){
	QWidget *selected = ui->scrollAreaWidgetContents->childAt(50,ui->scrollArea->verticalScrollBar()->value());
	//try slight offset if selected a spaced/null region
	if(!selected || selected->objectName() == "scrollAreaWidgetContents"){
		selected = ui->scrollAreaWidgetContents->childAt(50,ui->scrollArea->verticalScrollBar()->value()+10);
	}
	while(selected && selected->parent()->objectName() != "scrollAreaWidgetContents") {
		selected = qobject_cast<QWidget*>(selected->parent());
	}
	if(selected && selected->objectName() == "ThreadForm"){
		return static_cast<ThreadForm*>(selected);
	}
	else return Q_NULLPTR;
}

void BoardTab::on_pushButton_clicked()
{
	findText(ui->lineEdit->text());
}

void BoardTab::on_lineEdit_returnPressed()
{
	findText(ui->lineEdit->text());
}

void BoardTab::focusIt()
{
	if(ui->searchWidget->isHidden())ui->searchWidget->show();
	ui->lineEdit->setFocus();
}

void BoardTab::clearMap(){
	tfMap.clear();
}
