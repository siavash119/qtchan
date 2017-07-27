#include "boardtab.h"
#include "ui_boardtab.h"
#include "mainwindow.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QScrollBar>
#include <QRegularExpressionMatch>

BoardTab::BoardTab(QString board, BoardType type, QString search, QWidget *parent) :
	QWidget(parent), board(board), type(type), search(search),
	ui(new Ui::BoardTab)
{
	ui->setupUi(this);
	ui->searchWidget->hide();
	//TODO check if actual board
	this->setWindowTitle("/"+board+"/"+search);
	if(type == BoardType::Index) boardUrl = "https://a.4cdn.org/"+board+"/1.json";
	else boardUrl = "https://a.4cdn.org/"+board+"/catalog.json";
	helper.startUp(board, type, search, this);
	helper.moveToThread(&workerThread);
	connect(&helper,&BoardTabHelper::newThread,this,&BoardTab::onNewThread,UniqueDirect);
	connect(&helper,&BoardTabHelper::newTF,this,&BoardTab::onNewTF,UniqueDirect);
	connect(&helper,&BoardTabHelper::addStretch,this,&BoardTab::addStretch,UniqueDirect);
	connect(&helper,&BoardTabHelper::clearMap,this,&BoardTab::clearMap,UniqueDirect);

	myPostForm.setParent(this,Qt::Tool
						 | Qt::WindowMaximizeButtonHint
						 | Qt::WindowCloseButtonHint);
	myPostForm.load(board,"");
	connect(&myPostForm,&PostForm::loadThread,[=](QString threadNum){
		TreeItem *childOf = mw->model->getItem(mw->selectionModel->currentIndex());
		mw->onNewThread(mw,board,threadNum,QString(),childOf);
	});

	this->setShortcuts();
	connect(mw,&MainWindow::setUse4chanPass,&myPostForm,&PostForm::usePass,UniqueDirect);
}

BoardTab::~BoardTab()
{
	ui->threads->removeItem(&space);
	//qDeleteAll(tfMap);
	helper.abort = 1;
	disconnect(&helper);
	disconnect(&workerThread);
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

	QAction *focuser = new QAction(this);
	focuser->setShortcut(Qt::Key_F3);
	connect(focuser,&QAction::triggered,mw,&MainWindow::focusTree);
	this->addAction(focuser);

	QAction *focusBar = new QAction(this);
	focusBar->setShortcut(Qt::Key_F6);
	connect(focusBar,&QAction::triggered,mw,&MainWindow::focusBar);
	this->addAction(focusBar);

	QAction *focusSearch = new QAction(this);
	focusSearch->setShortcut(QKeySequence("Ctrl+f"));
	connect(focusSearch,&QAction::triggered,this,&BoardTab::focusIt);
	this->addAction(focusSearch);

	QAction *scrollUp = new QAction(this);
	scrollUp->setShortcut(Qt::Key_J);
	connect(scrollUp, &QAction::triggered,[=]{
		ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() - 150);
	});
	this->addAction(scrollUp);

	QAction *scrollDown = new QAction(this);
	scrollDown->setShortcut(Qt::Key_K);
	connect(scrollDown, &QAction::triggered,[=]{
		ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + 150);
	});
	this->addAction(scrollDown);

	QAction *selectPost = new QAction(this);
	selectPost->setShortcut(Qt::Key_O);
	connect(selectPost, &QAction::triggered,[=]{
		QWidget *selected = ui->scrollAreaWidgetContents->childAt(50,ui->scrollArea->verticalScrollBar()->value());
		qDebug() << selected;
		while(selected->parent()->objectName() != "scrollAreaWidgetContents") {
			  selected = qobject_cast<QWidget*>(selected->parent());
		}
		if(selected->objectName() == "ThreadForm"){
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
	if(text == "") ui->searchWidget->hide();
	QRegularExpression re(text,QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch match;
	ThreadForm *tf;
	bool pass = false;
	if (text == "") pass = true;
	qDebug().noquote() << "searching " + text;
	QMapIterator<QString,ThreadForm*> mapI(tfMap);
	while (mapI.hasNext()) {
		mapI.next();
		tf = mapI.value();
		if(pass) { tf->show(); continue;};
		match = re.match(tf->post.sub + tf->post.com);
		if(!match.hasMatch()) {
			tf->hide();
		}
		else qDebug().noquote().nospace() << "found " << text << " in thread #" << tf->post.no;
	}
}

void BoardTab::addStretch()
{
	ui->threads->removeItem(&space);
	ui->threads->insertItem(-1,&space);
}

void BoardTab::onNewTF(ThreadForm *tf, ThreadForm *thread)
{
	connect(thread,&ThreadForm::removeMe,tf,&ThreadForm::deleteLater,Qt::DirectConnection);
	thread->addReply(tf);
}

void BoardTab::onNewThread(ThreadForm *tf)
{
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
