#include "threadtab.h"
#include "ui_threadtab.h"
#include "netcontroller.h"
#include "mainwindow.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>
#include <QKeyEvent>
#include <QMutableMapIterator>
#include <QProcess>
#include <QScrollBar>
#include <QTimer>
#include <QDesktopWidget>
#include <QScreen>
#include <QFuture>

ThreadTab::ThreadTab(Chan *api, QString board, QString thread, QWidget *parent, bool isFromSession) :
	QWidget(parent), api(api), board(board), thread(thread), isFromSession(isFromSession),
	ui(new Ui::ThreadTab)
{
	ui->setupUi(this);
	this->setWindowTitle("/"+board+"/"+thread);
	ui->searchWidget->hide();
	helper.startUp(api,board,thread, this, isFromSession);
	//this->ui->verticalLayout->insertWidget(0,&info);
	//info.setParent(this,Qt::Tool | Qt::FramelessWindowHint);
	info.setParent(this);
	info.move(this->width()-info.width()-20,this->height()-info.height()-20);
	info.show();
	helper.moveToThread(&workerThread);
	connect(&helper,&ThreadTabHelper::newTF,this,&ThreadTab::onNewTF,UniqueDirect);
	connect(&helper,&ThreadTabHelper::windowTitle,this,&ThreadTab::onWindowTitle,UniqueDirect);
	connect(&helper,&ThreadTabHelper::tabTitle,[=](QString tabTitle){
		tn->setData(0,tabTitle);
	});
	myPostForm.setParent(this,Qt::Tool
						 | Qt::WindowMaximizeButtonHint
						 | Qt::WindowCloseButtonHint);
	myPostForm.load(api,board,thread);
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	QFont temp = ui->lineEdit->font();
	temp.setPointSize(settings.value("fontSize",14).toInt()-2);
	ui->label->setFont(temp);
	ui->lineEdit->setFont(temp);
	ui->pushButton->setFont(temp);
	this->setShortcuts();
	this->installEventFilter(this);
	connectionAutoUpdate = connect(mw,&MainWindow::setAutoUpdate,&helper,&ThreadTabHelper::setAutoUpdate,UniqueDirect);
	connect(mw,&MainWindow::setUse4chanPass,&myPostForm,&PostForm::usePass,UniqueDirect);
	//connect(&helper,&ThreadTabHelper::addStretch,this,&ThreadTab::addStretch,UniqueDirect);
	connect(mw,&MainWindow::setFontSize,this,&ThreadTab::setFontSize,UniqueDirect);
	connect(mw,&MainWindow::setImageSize,this,&ThreadTab::setImageSize,UniqueDirect);
	connect(mw,&MainWindow::reloadFilters,[=](){
		filter = Filter();
	});
	//check visible thread forms
	QScrollBar *vBar = ui->scrollArea->verticalScrollBar();
	connect(&watcher,&QFutureWatcherBase::finished,[=]()
	{
		QList<ThreadForm*> newSeen = newImage.result();
		ThreadForm *curTF;
		for(int i=0; i< newSeen.size(); i++) {
			curTF = newSeen.at(i);
			if(!curTF->seen) {
				formsUnseen--;
				unseenList.removeOne(curTF);
				emit unseen(formsUnseen);
			}
			curTF->seen = true;
			//curTF->setStyleSheet("background-color: #c80808; color:#bbbbbb;");
		}
		//if(!formsUnseen) disconnect(connectionVisibleChecker);
	});
	connectionVisibleChecker = connect(vBar, &QScrollBar::sliderMoved,[=](int value)
	{
		(void)value;
		newImage = QtConcurrent::run(&ThreadTab::checkIfVisible, unseenList);
		watcher.setFuture(newImage);
	});

	//connect(&helper,&ThreadTabHelper::refresh,[=](ThreadForm *tf) {onRefresh(tf);});
}

void ThreadTab::setFontSize(int fontSize){
	QFont temp = font();
	temp.setPointSize(fontSize-2);
	ui->label->setFont(temp);
	ui->lineEdit->setFont(temp);
	ui->pushButton->setFont(temp);
	temp.setPointSize(fontSize);
	//TODO fix info setFontSize
	//info.setFont(temp);
	myPostForm.setFontSize(fontSize);
	foreach(ThreadForm *tf, tfMap){
		tf->setFontSize(fontSize);
	}
}

void ThreadTab::setImageSize(int imageSize){
	foreach(ThreadForm *tf, tfMap){
		tf->setImageSize(imageSize);
	}
}

QList<ThreadForm*> ThreadTab::checkIfVisible(QList<ThreadForm*> &unseenList)
{
	QList<ThreadForm*> seenList;
	int size = unseenList.size();
	for(int i=0;i<size;i++) {
		ThreadForm *tf = unseenList.at(i);
		if(!tf->visibleRegion().isEmpty() && !(tf->seen)) {
			seenList.append(tf);
		}
	}
	return seenList;
}

/*void ThreadTab::checkScroll() {
	QScrollBar *vBar = ui->scrollArea->verticalScrollBar();
	int height = ui->scrollAreaWidgetContents->size().height();
	int scrollPos = vBar->value() + vBar->height();
	qDebug() << height;
	qDebug() << scrollPos;
	qDebug() << "----------";
	if(scrollPos >= height - 500) vBar->triggerAction(QScrollBar::SliderToMaximum);
	//vBar->setSliderDown();
}*/

void ThreadTab::setShortcuts()
{
	QAction *gallery = new QAction(this);
	gallery->setShortcut(Qt::Key_G);
	connect(gallery, &QAction::triggered, this, &ThreadTab::gallery);
	this->addAction(gallery);

	QAction *postForm = new QAction(this);
	postForm->setShortcut(Qt::Key_Q);
	connect(postForm, &QAction::triggered, this, &ThreadTab::openPostForm);
	this->addAction(postForm);

	QAction *expandAll = new QAction(this);
	expandAll->setShortcut(Qt::Key_E);
	connect(expandAll, &QAction::triggered,&helper,&ThreadTabHelper::loadAllImages,UniqueDirect);
	this->addAction(expandAll);

	QAction *refresh = new QAction(this);
	refresh->setShortcut(Qt::Key_R);
	refresh->setShortcutContext(Qt::ApplicationShortcut);
	connect(refresh, &QAction::triggered,&helper,&ThreadTabHelper::getPosts,UniqueDirect);
	this->addAction(refresh);

	QAction *focusBar = new QAction(this);
	focusBar->setShortcut(Qt::Key_F6);
	connect(focusBar,&QAction::triggered,mw,&MainWindow::focusBar);
	this->addAction(focusBar);

	QAction *selectPost = new QAction(this);
	selectPost->setShortcut(Qt::Key_O);
	connect(selectPost, &QAction::triggered,[=]{
		if(ThreadForm *tf = tfAtTop()) tf->imageClicked();
	});
	this->addAction(selectPost);

	//NOTE: for scrollUp and scrollDown, prevYou, nextYou
	//you can swap tfAtTop/tfAtBottom to your scroll preference
	QAction *scrollUp = new QAction(this);
	scrollUp->setShortcut(Qt::Key_K);
	connect(scrollUp, &QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		QMapIterator<QString,ThreadForm*> i(tfMap);
		if(ThreadForm *tf = tfAtTop()){
			if(i.findNext(tf)){
				if(i.hasPrevious()) i.previous();
				while(vimNumber-- && i.hasPrevious()){
					i.previous();
					if(i.value()->isHidden())vimNumber++;
				};
				ui->scrollArea->verticalScrollBar()->setValue(i.value()->pos().y());
			}
		}
		vimCommand = "";
	});
	this->addAction(scrollUp);

	QAction *scrollDown = new QAction(this);
	scrollDown->setShortcut(Qt::Key_J);
	connect(scrollDown, &QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		QMapIterator<QString,ThreadForm*> i(tfMap);
		if(ThreadForm *tf = tfAtTop()){
			if(i.findNext(tf)){
				while(vimNumber-- && i.hasNext()){
					i.next();
					if(i.value()->isHidden()) vimNumber++;
				}
				ui->scrollArea->verticalScrollBar()->setValue(
							i.value()->pos().y());
			}
		}
		vimCommand = "";
	});
	this->addAction(scrollDown);

	QAction *prevYou = new QAction(this);
	prevYou->setShortcut(Qt::Key_H);
	connect(prevYou,&QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		QMapIterator<QString,ThreadForm*> i(tfMap);
		if(ThreadForm *tf = tfAtTop()){
			if(i.findNext(tf)){
				if(i.hasPrevious()) i.previous();
				ThreadForm *found = Q_NULLPTR;
				while(vimNumber && i.hasPrevious()){
					i.previous();
					if(i.value()->post.hasYou && !i.value()->isHidden()){
						found = i.value();
						vimNumber--;
					}
				}
				if(found) ui->scrollArea->verticalScrollBar()->setValue(found->pos().y());
			}
		}
		vimCommand = "";
	});
	this->addAction(prevYou);


	QAction *nextYou = new QAction(this);
	nextYou->setShortcut(Qt::Key_L);
	connect(nextYou,&QAction::triggered,[=]{
		int vimNumber = 1;
		if(!vimCommand.isEmpty()) vimNumber = vimCommand.toInt();
		QMapIterator<QString,ThreadForm*> i(tfMap);
		if(ThreadForm *tf = tfAtTop()){
			if(i.findNext(tf)){
				ThreadForm *found = Q_NULLPTR;
				while(vimNumber && i.hasNext()){
					i.next();
					if(i.value()->post.hasYou && !i.value()->isHidden()){
						found = i.value();
						vimNumber--;
					}
				}
				if(found) ui->scrollArea->verticalScrollBar()->setValue(
							found->pos().y());
			}
		}
		vimCommand = "";
	});
	this->addAction(nextYou);


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
		connect(numberPressed, &QAction::triggered,this,&ThreadTab::updateVim);
		this->addAction(numberPressed);
	}
}

void ThreadTab::updateVim(){
	 QAction* action = qobject_cast<QAction*>(sender());
	 QKeySequence seq = action->shortcut();
	 vimCommand += seq.toString();
}

ThreadForm* ThreadTab::tfAtTop(){
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

ThreadForm* ThreadTab::tfAtBottom(){
	QWidget *selected = ui->scrollAreaWidgetContents->childAt(50,ui->scrollArea->verticalScrollBar()->value()+ui->scrollArea->height());
	//try slight offset if selected a spaced/null region
	if(!selected || selected->objectName() == "scrollAreaWidgetContents"){
		selected = ui->scrollAreaWidgetContents->childAt(50,ui->scrollArea->verticalScrollBar()->value()+ui->scrollArea->height()-10);
	}
	while(selected && selected->parent()->objectName() != "scrollAreaWidgetContents") {
		selected = qobject_cast<QWidget*>(selected->parent());
	}
	if(selected && selected->objectName() == "ThreadForm"){
		qDebug() << static_cast<ThreadForm*>(selected)->post.no;
		return static_cast<ThreadForm*>(selected);
	}
	else return Q_NULLPTR;
}

ThreadTab::~ThreadTab()
{
	//ui->threads->removeItem(&space);
	helper.abort = true;
	workerThread.quit();
	workerThread.wait();
	/*disconnect(&helper);
	disconnect(&workerThread);*/
	disconnect(connectionAutoUpdate);
	delete ui;
	qDebug().noquote().nospace() << "deleting tab /" << board+"/"+thread;
}

void ThreadTab::openPostForm()
{
	myPostForm.show();
	myPostForm.activateWindow();
	myPostForm.raise();
}

void ThreadTab::gallery()
{
	/*QString command = "feh"; QStringList arguments; arguments << QDir("./"+board+"/"+thread).absolutePath()
															  << "--auto-zoom"
															  << "--index-info" << "\"%n\n%S\n%wx%h\""
															  << "--borderless"
															  << "--image-bg" << "black"
															  << "--preload";*/
	QString command = "mpv"; QStringList arguments; arguments << QDir("./"+board+"/"+thread).absolutePath();
	QProcess().startDetached(command,arguments);
}

/*void ThreadTab::addStretch()
{
	ui->threads->removeItem(&space);
	ui->threads->insertItem(-1,&space);
}*/

int ThreadTab::getMinWidth()
{
	return ui->scrollArea->minimumWidth();
}

void ThreadTab::updateWidth()
{
	ui->scrollArea->setMinimumWidth(this->sizeHint().width());
}

void ThreadTab::onNewTF(ThreadForm *tf)
{
	if(filter.filterMatched(tf->matchThis())){
		tf->hidden=true;
		tf->hide();
		info.hidden++;
	}
	ui->threads->addWidget(tf);
	tfMap.insert(tf->post.no,tf);
	connect(tf,&ThreadForm::floatLink,this,&ThreadTab::floatReply);
	connect(tf,&ThreadForm::removeMe,this,&ThreadTab::removeTF, Qt::DirectConnection);
	connect(tf,&ThreadForm::deleteFloat,this,&ThreadTab::deleteFloat,Qt::DirectConnection);
	connect(tf,&ThreadForm::updateFloat,this,&ThreadTab::updateFloat,Qt::DirectConnection);
	unseenList.append(tf);
	formsTotal++;
	formsUnseen++;
	info.posts++;
	if(!tf->post.tim.isEmpty()) info.files++;
	info.unseen++;
	info.updateFields();
}

void ThreadTab::removeTF(ThreadForm *tf)
{
	//tfMap.remove(tf->post.no);
	if(!tf->seen) {
		formsUnseen--;
		unseenList.removeOne(tf);
	}
	info.updateFields();
}

void ThreadTab::onWindowTitle(QString title)
{
	this->setWindowTitle(title);
	if(mw->currentWidget() == this) mw->setWindowTitle(title);
	myPostForm.setWindowTitle("post to " + title);
}

void ThreadTab::loadAllImages()
{
	updated = false;
	QMapIterator<QString,ThreadForm*> mapI(tfMap);
	while (mapI.hasNext()) {
		mapI.next();
		static_cast<ThreadForm *>(mapI.value())->getFile();
	}
}

ThreadForm *ThreadTab::findPost(QString postNum)
{
	return tfMap.value(postNum);
}

//TODO, just search the whole JSON and find posts
void ThreadTab::findText(const QString &text)
{
	if(text.isEmpty()){
		qDebug().noquote() << "clearing search";
		ui->searchWidget->hide();
		QMapIterator<QString,ThreadForm*> mapI(tfMap);
		while (mapI.hasNext()) {
			mapI.next();
			mapI.value()->show();
		}
		return;
	}
	qDebug().noquote() << "searching " + text;
	QString temp(text);
	QRegularExpression re(temp.replace("\n",""),QRegularExpression::CaseInsensitiveOption);
	qDebug() << re;
	QRegularExpressionMatch match;
	ThreadForm *tf;
	QMapIterator<QString,ThreadForm*> mapI(tfMap);
	while (mapI.hasNext()) {
		mapI.next();
		tf = mapI.value();
		QString toMatch(tf->matchThis());
		toMatch = Filter::toStrippedHtml(toMatch);
		qDebug() << toMatch;
		match = re.match(toMatch);
		//can keep applying filters over and over again
		//good or bad?
		if(!match.hasMatch()) {
			tf->hide();
		}
		else qDebug().noquote().nospace() << "found " << text << " in thread #" << tf->post.no;
	}
}

void ThreadTab::on_pushButton_clicked()
{
	findText(ui->lineEdit->text());
}

void ThreadTab::quoteIt(QString text) {
	myPostForm.appendText(text);
	openPostForm();
}

//TODO put these keybinds as QAction shortcuts
bool ThreadTab::eventFilter(QObject *obj, QEvent *event)
{
	switch(event->type())
	{
	case QEvent::Resize: {
		info.move(this->width()-info.width()-20,this->height()-info.height()-20);
		[[fallthrough]];
	}
	case QEvent::Wheel: {
		newImage = QtConcurrent::run(&ThreadTab::checkIfVisible, unseenList);
		watcher.setFuture(newImage);
		[[fallthrough]];
	}
	case QEvent::Leave: {
		deleteFloat();
		[[fallthrough]];
	}
	default:{
		return QObject::eventFilter(obj, event);
	}
	}
	return QObject::eventFilter(obj, event);
}

void ThreadTab::focusIt()
{
	if(ui->searchWidget->isHidden())ui->searchWidget->show();
	ui->lineEdit->setFocus();
}

void ThreadTab::on_lineEdit_returnPressed()
{
	findText(ui->lineEdit->text());
}

void ThreadTab::floatReply(const QString &link, int replyLevel)
{
	deleteFloat();
	QPointer<ThreadForm> tf = findPost(link);
	if(!tf) return;
	floating = tf->clone(replyLevel);
	floating->deleteHideLayout();
	floating->setParent(this);
	floating->setObjectName("reply");
	floating->setWindowFlags(Qt::ToolTip);
	floating->setWindowTitle("reply");
	QRect rec = QApplication::desktop()->availableGeometry(this);
	QPoint globalCursorPos = QCursor::pos();
	QSize sizeHint = floating->sizeHint();
	/*qDebug() << sizeHint;
	if(floating->hasImage && sizeHint.width() < 850){
		sizeHint.setWidth(850);
		sizeHint.setHeight(floating->heightForWidth(850));
	}
	else if(!floating->hasImage && sizeHint.width() < 600){
			sizeHint.setWidth(600);
			sizeHint.setHeight(floating->heightForWidth(600));
	}
	qDebug() << sizeHint;*/
	//floating->setFixedSize(sizeHint);
	int x = -1, y = -1;
	if(globalCursorPos.x()  - rec.topLeft().x() + sizeHint.width() + 10 > rec.width()) {
		x = globalCursorPos.x() - sizeHint.width() - 10;
	}
	if(x<0) x = globalCursorPos.x()+10;
	if(globalCursorPos.y() - rec.topLeft().y() + sizeHint.height() + 10 > rec.height()) {
		y = globalCursorPos.y() - sizeHint.height() - 10;
	}
	if(y<0) y = globalCursorPos.y()+10;
	floating->setGeometry(x,y,sizeHint.width(),sizeHint.height());
	floating->update();
	/*floating->setStyleSheet(floating->styleSheet()
							+QString::fromUtf8(" QWidget#ThreadForm{border: 3px solid black;}"));*/
	floating->show();
}

void ThreadTab::deleteFloat()
{
	if(floating) {
		floating->deleteLater();
	}
}

void ThreadTab::updateFloat()
{
	if(floating) {
		QRect rec = QApplication::desktop()->availableGeometry(this);
		QPoint globalCursorPos = QCursor::pos();
		QSize sizeHint = floating->sizeHint();
		int x = -1, y = -1;
		if(globalCursorPos.x() - rec.topLeft().x() + sizeHint.width() + 10 > rec.width()) {
			x = globalCursorPos.x() - sizeHint.width() - 10;
		}
		if(x<0) x = globalCursorPos.x()+10;
		if(globalCursorPos.y() - rec.topLeft().y() + sizeHint.height() + 10 > rec.height()) {
			y = globalCursorPos.y() - sizeHint.height() - 10;
		}
		if(y<0) y = globalCursorPos.y()+10;
		floating->move(x,y);
	}
}
