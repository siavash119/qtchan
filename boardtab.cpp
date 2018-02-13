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
	helper.startUp(api,board, type, search, this);
	QCoreApplication::processEvents();
	helper.moveToThread(&workerThread);
	connect(&helper,&BoardTabHelper::newThread,this,&BoardTab::onNewThread,UniqueDirect);
	connect(&helper,&BoardTabHelper::newTF,this,&BoardTab::onNewTF,UniqueDirect);
	connect(&helper,&BoardTabHelper::clearMap,this,&BoardTab::clearMap,UniqueDirect);

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
	this->installEventFilter(this);
	this->setShortcuts();
	connect(mw,&MainWindow::setUse4chanPass,&myPostForm,&PostForm::usePass,UniqueDirect);
	connect(mw,&MainWindow::setFontSize,this,&BoardTab::setFontSize,UniqueDirect);
	connect(mw,&MainWindow::setImageSize,this,&BoardTab::setImageSize,UniqueDirect);
	connect(mw,&MainWindow::reloadFilters,[=](){
		filter = Filter();
	});

}

void BoardTab::setFontSize(int fontSize){
	QFont temp = ui->lineEdit->font();
	temp.setPointSize(fontSize);
	ui->label->setFont(temp);
	ui->lineEdit->setFont(temp);
	ui->pushButton->setFont(temp);
	myPostForm.setFontSize(fontSize);
	foreach(ThreadForm *tf, tfMap){
		tf->setFontSize(fontSize);
	}
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
	delete ui;
	qDebug().noquote().nospace() << "deleting board /" << board+"/";
}

void BoardTab::setShortcuts()
{
	QAction *refresh = new QAction(this);
	refresh->setShortcut(Qt::Key_R);
	connect(refresh, &QAction::triggered, &helper, &BoardTabHelper::getPosts, UniqueDirect);
	this->addAction(refresh);

	QAction *postForm = new QAction(this);
	postForm->setShortcut(Qt::Key_Q);
	connect(postForm, &QAction::triggered, this, &BoardTab::openPostForm, UniqueDirect);
	this->addAction(postForm);

	QAction *scrollUp = new QAction(this);
	scrollUp->setShortcut(Qt::Key_K);
	connect(scrollUp, &QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() - vimNumber*150);
		vimCommand = "";
	});
	this->addAction(scrollUp);

	QAction *scrollDown = new QAction(this);
	scrollDown->setShortcut(Qt::Key_J);
	connect(scrollDown, &QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + vimNumber*150);
		vimCommand = "";
	});
	this->addAction(scrollDown);

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
}

void BoardTab::openPostForm()
{
	myPostForm.show();
	myPostForm.activateWindow();
	myPostForm.raise();
}

void BoardTab::findText(const QString text)
{
	bool pass = false;
	if(text.isEmpty()){
		ui->searchWidget->hide();
		pass = true;
	}
	QRegularExpression re(text,QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match;
	ThreadForm *tf;
	qDebug().noquote() << "searching " + text;
	QMapIterator<QString,ThreadForm*> mapI(tfMap);
	while (mapI.hasNext()) {
		mapI.next();
		tf = mapI.value();
		if(pass){
			tf->show();
			continue;
		};
		QString toMatch(tf->post.sub + " " + tf->post.com);
		toMatch.replace("<br>"," ");
		toMatch.remove(QRegExp("<[^>]*>"));
		match = re.match(toMatch);
		if(match.hasMatch()){
			tf->show();
			qDebug().noquote().nospace() << "found " << text << " in thread #" << tf->post.no;
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
	connect(thread,&ThreadForm::removeMe,tf,&ThreadForm::deleteLater,Qt::DirectConnection);
	thread->addReply(tf);
}

void BoardTab::onNewThread(ThreadForm *tf)
{
	QString temp = tf->post.com % tf->post.sub % tf->post.name;
	if(filter.filterMatched(temp)){
		tf->hidden=true;
		//tf->hide();
	}
	ui->threads->addWidget(tf);
	tfMap.insert(tf->post.no,tf);
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

bool BoardTab::eventFilter(QObject *obj, QEvent *event)
{
	switch(event->type())
	{
	case QEvent::KeyPress: {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		int mod = keyEvent->modifiers();
		int key = keyEvent->key();
		if(key >= Qt::Key_0 && key <= Qt::Key_9){
			vimCommand += QKeySequence(key).toString();
			qDebug() << vimCommand;
		}
		else if(mod == Qt::KeyboardModifier::ShiftModifier && key == Qt::Key_G){
			int vimNumber = 100;
			if(!vimCommand.isEmpty())vimNumber = vimCommand.toInt();
			ui->scrollArea->verticalScrollBar()->setValue(ui->scrollAreaWidgetContents->height()*vimNumber/100);
			vimCommand = "";
		}
		else if(key == Qt::Key_Escape){
			vimCommand = "";
		}
		return QObject::eventFilter(obj, event);
	}
	default:
		return QObject::eventFilter(obj, event);
	}
}
