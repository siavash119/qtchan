#include "threadtab.h"
#include "ui_threadtab.h"
#include "netcontroller.h"
#include "notificationview.h"
#include "mainwindow.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QShortcut>
#include <QKeySequence>
#include <QDir>
#include <QKeyEvent>
#include <QProcess>
#include <QScrollBar>
#include <QDesktopWidget>
#include <QScreen>
#include <QFuture>
#include <QAction>

ThreadTab::ThreadTab(Chan *api, QString &board, QString &thread, QWidget *parent, bool isFromSession) :
	QWidget(parent), api(api), board(board), thread(thread), isFromSession(isFromSession),
	ui(new Ui::ThreadTab)
{
	ui->setupUi(this);
	this->setWindowTitle("/"+board+"/"+thread);
	ui->searchWidget->hide();
	//this->ui->verticalLayout->insertWidget(0,&info);
	//info.setParent(this,Qt::Tool | Qt::FramelessWindowHint);
	info.setParent(this);
	info.move(this->width()-info.width()-20,this->height()-info.height()-20);
	info.show();
	QSettings settings;
	vsb = ui->scrollArea->verticalScrollBar();
	connect(vsb,&QScrollBar::sliderMoved,[=](int value){
		if(value == vsb->maximum()) atBottom = true;
		else atBottom = false;
	});
	connect(vsb,&QScrollBar::rangeChanged,[=](int min, int max){
		QSettings settings;
		(void)min;
		if(atBottom &&
				((this == mw->currentTab && settings.value("autoScrollActive",false).toBool())
				 || (settings.value("autoScrollBackground",false).toBool() && this != mw->currentTab)))
			vsb->setValue(max);
	});
	updateTimer.setInterval(60000);
	connect(&updateTimer,&QTimer::timeout,this,&ThreadTab::getPosts);
	if(settings.value("autoUpdate").toBool()) {
		updateTimer.start();
	}
	helper.moveToThread(&workerThread);
	connect(this,&ThreadTab::startHelper,&helper,&ThreadTabHelper::startUp,Qt::QueuedConnection);
	connect(&helper,&ThreadTabHelper::newTF,this,&ThreadTab::onNewTF);
	connect(&helper,&ThreadTabHelper::windowTitle,this,&ThreadTab::onWindowTitle,Qt::QueuedConnection);
	connect(&helper,&ThreadTabHelper::tabTitle,this,&ThreadTab::setTabTitle,Qt::QueuedConnection);
	connect(&helper,&ThreadTabHelper::addReply,this,&ThreadTab::onAddReply,Qt::QueuedConnection);
	connect(&helper,&ThreadTabHelper::addNotification,this,&ThreadTab::onAddNotification,Qt::QueuedConnection);
	connect(&helper,&ThreadTabHelper::threadStatus,this,&ThreadTab::onThreadStatus);
	connect(&helper,&ThreadTabHelper::getPosts,this,&ThreadTab::getPosts);
	connect(&helper,&ThreadTabHelper::getFlags,this,&ThreadTab::onGetFlags);
	connect(&helper,&ThreadTabHelper::setRegion,this,&ThreadTab::onSetRegion);

	//probably trash; maybe put allPosts hash in helper
	connect(mw,&MainWindow::reloadFilters,&helper,&ThreadTabHelper::reloadFilters);
	connect(&helper,&ThreadTabHelper::startFilterTest,this,&ThreadTab::reloadFilters);
	connect(this,&ThreadTab::testFilters,&helper,&ThreadTabHelper::filterTest);
	connect(&helper,&ThreadTabHelper::filterTested,this,&ThreadTab::onFilterTest);
	connect(mw,&MainWindow::updateStyles,[=](QString key, QString value){
		if(key == "ThreadForm"){
			setStyleSheet(value);
			foreach(ThreadForm *tf, tfMap){
				tf->setBackground();
				foreach(QPointer<ThreadForm> clone, tf->clones){
					if(clone) clone->setBackground();
				}
			}
		}
		else if(key == "MainWindow"){
			foreach(ThreadForm *tf, tfMap){
				tf->setBackground();
				foreach(QPointer<ThreadForm> clone, tf->clones){
					if(clone) clone->setBackground();
				}
			}
		}
	});

	workerThread.start();

	myPostForm.setParent(this,Qt::Tool
						 | Qt::WindowMaximizeButtonHint
						 | Qt::WindowCloseButtonHint);
	myPostForm.load(api,board,thread);
	QFont temp = ui->lineEdit->font();
	temp.setPointSize(settings.value("fontSize",14).toInt()-2);
	ui->label->setFont(temp);
	ui->lineEdit->setFont(temp);
	ui->pushButton->setFont(temp);
	setStyleSheet(settings.value("style/ThreadForm","color:#bbbbbb;").toString());

	this->setShortcuts();
	this->installEventFilter(this);

	connect(mw,&MainWindow::setUse4chanPass,&myPostForm,&PostForm::usePass);
	connect(mw,&MainWindow::setFontSize,this,&ThreadTab::setFontSize);
	connect(mw,&MainWindow::setImageSize,this,&ThreadTab::setImageSize);
	connect(mw,&MainWindow::setAutoUpdate,this,&ThreadTab::setAutoUpdate);
	connect(mw,&MainWindow::setAutoExpand,[=](bool expand){
		if(expand){
			foreach(ThreadForm *tf,tfMap){
				if(tf) tf->getFiles();
			}
		}
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
	emit startHelper(api,board,thread,this,isFromSession);
	//helper.startUp(api,board,thread,this,isFromSession);
}

void ThreadTab::setAutoUpdate(bool update) {
	if(status == "404" || status == "closed" || status == "archived") return;
	if(update) {
		qDebug() << "starting update timer for" << helper.title;
		updateTimer.start();
	}
	else{
		qDebug() << "stopping update timer for" << helper.title;
		updateTimer.stop();
	}
}

void ThreadTab::setTabTitle(QString tabTitle){
	tn->setData(0,tabTitle);
}

void ThreadTab::onThreadStatus(QString status, QString value){
	(void)value;
	this->status = status;
	if(status == "404" || status == "closed" || status == "archived"){
		qDebug() << "stopping update timer for" << helper.title;
		if(updateTimer.isActive()) updateTimer.stop();
	}
}

void ThreadTab::getPosts(){
	qDebug().noquote().nospace() << "getting " << helper.title;
	postsReply = nc.jsonManager->get(helper.request);
	connect(postsReply,&QNetworkReply::finished,&helper,&ThreadTabHelper::getPostsFinished,Qt::UniqueConnection);
}

void ThreadTab::onGetFlags(QByteArray data){
	qDebug().noquote().nospace() << "getting extra flags for " << helper.title;
	flagsReply = nc.fileManager->post(helper.requestFlags,data);
	connect(flagsReply,&QNetworkReply::finished,&helper,&ThreadTabHelper::loadExtraFlags,Qt::UniqueConnection);
}

void ThreadTab::onSetRegion(QString post_nr, QString region){
	QPointer<ThreadForm> tf = tfMap.value(post_nr);
	if(tf && tf->post.troll_country.isEmpty()) tf->setRegion(region);
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
	QSettings settings;
	settings.beginGroup("keybinds");

	QAction *gallery = new QAction(this);
	gallery->setShortcut(QKeySequence(settings.value("gallery","g").toString()));
	connect(gallery, &QAction::triggered,this,&ThreadTab::gallery);
	this->addAction(gallery);

	QAction *postForm = new QAction(this);
	postForm->setShortcut(QKeySequence(settings.value("reply","q").toString()));
	connect(postForm, &QAction::triggered,this,&ThreadTab::openPostForm);
	this->addAction(postForm);

	QAction *expandAll = new QAction(this);
	expandAll->setShortcut(QKeySequence(settings.value("toggleExpandTab","e").toString()));
	connect(expandAll, &QAction::triggered,this,&ThreadTab::loadAllImages);
	this->addAction(expandAll);

	QAction *refresh = new QAction(this);
	refresh->setShortcut(QKeySequence(settings.value("refreshTab","r").toString()));
	refresh->setShortcutContext(Qt::ApplicationShortcut);
	connect(refresh, &QAction::triggered,this,&ThreadTab::getPosts);
	//&helper,&ThreadTabHelper::getPosts,Qt::DirectConnection);
	this->addAction(refresh);

	/*QAction *focusBar = new QAction(this);
	focusBar->setShortcut(Qt::Key_F6);
	connect(focusBar,&QAction::triggered,mw,&MainWindow::focusBar);
	this->addAction(focusBar);*/

	QAction *selectPost = new QAction(this);
	selectPost->setShortcut(QKeySequence(settings.value("openSelected","o").toString()));
	connect(selectPost, &QAction::triggered,[=]{
		if(ThreadForm *tf = tfAtTop()) tf->imageClicked();
	});
	this->addAction(selectPost);

	//NOTE: for scrollUp and scrollDown, prevYou, nextYou
	//you can swap tfAtTop/tfAtBottom to your scroll preference
	QAction *scrollUp = new QAction(this);
	scrollUp->setShortcut(QKeySequence(settings.value("scrollUp","k").toString()));
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
	scrollDown->setShortcut(QKeySequence(settings.value("scrollDown","j").toString()));
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
	prevYou->setShortcut(QKeySequence(settings.value("prevReply","h").toString()));
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
	nextYou->setShortcut(QKeySequence(settings.value("nextReply","l").toString()));
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
	scrollPercent->setShortcut(QKeySequence(settings.value("scrollTo","shift+g").toString()));
	connect(scrollPercent, &QAction::triggered,[=]{
		int vimNumber = 100;
		if(!vimCommand.isEmpty())vimNumber = vimCommand.toInt();
		ui->scrollArea->verticalScrollBar()->setValue(ui->scrollAreaWidgetContents->height()*vimNumber/100);
		vimCommand = "";
	});
	this->addAction(scrollPercent);

	QAction *clearVim = new QAction(this);
	clearVim->setShortcut(QKeySequence(settings.value("clearVim","-").toString()));
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
	helper.abort = true;
	workerThread.quit();
	workerThread.wait();
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
	QString command = "mpv"; QStringList arguments; arguments << QDir(api->name()+'/'+board+'/'+thread).absolutePath();
	QProcess().startDetached(command,arguments);
}

bool ThreadTab::vsbAtMax(){
	if(vsb->isEnabled() && vsb->value() == vsb->maximum()) return true;
	else return false;
}

void ThreadTab::onNewTF(Post post, ThreadFormStrings strings, bool loadFile){
	ThreadForm *tf = new ThreadForm(api,strings,true,loadFile,this);
	tf->load(post);
	if(post.filtered){
		qDebug().noquote().nospace() << post.no << " filtered from " << this->windowTitle() << "!";
		tf->hide();
		info.hidden++;
	}
	atBottom = vsbAtMax();
	ui->threads->addWidget(tf);
	tfMap.insert(post.no,tf);
	connect(tf,&ThreadForm::floatLink,this,&ThreadTab::floatReply);
	connect(tf,&ThreadForm::removeMe,this,&ThreadTab::removeTF);
	connect(tf,&ThreadForm::deleteFloat,this,&ThreadTab::deleteFloat);
	connect(tf,&ThreadForm::updateFloat,this,&ThreadTab::updateFloat);
	unseenList.append(tf);
	formsTotal++;
	formsUnseen++;
	info.posts++;
	info.files += post.files.size();
	info.unseen++;
	info.updateFields();
}

void ThreadTab::onAddReply(QString orig, QString no, bool isYou){
	QPointer<ThreadForm> tf = tfMap.value(orig);
	if(!tf) return;
	tf->replies.insert(no.toDouble(),no);
	tf->addReplyLink(no,isYou);
}

void ThreadTab::onAddNotification(QString no){
	QPointer<ThreadForm> tf = tfMap.value(no);
	if(!tf) return;
	ThreadForm *cloned = tf->clone(0);
	nv->addNotification(cloned);
}

void ThreadTab::removeTF(ThreadForm *tf){
	atBottom = vsbAtMax();
	tf->hide();
	if(!tf->seen) {
		formsUnseen--;
		unseenList.removeOne(tf);
	}
	info.updateFields();
}

void ThreadTab::showTF(ThreadForm *tf){
	tf->show();
	if(!tf->seen){
		formsUnseen++;
		unseenList.append(tf);
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
	helper.expandAll = !helper.expandAll;
	if(!helper.expandAll) return;
	foreach(ThreadForm *tf,tfMap){
		tf->getFiles();
	}
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
		match = re.match(toMatch);
		if(match.hasMatch()){
			if(!tf->hidden) tf->show();
		}
		else tf->hide();
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

//TODO make post smaller if doesn't fit
void ThreadTab::floatReply(const QString &link, int replyLevel)
{
	deleteFloat();
	QPointer<ThreadForm> tf = tfMap.value(link);
	if(!tf) return;
	floating = tf->clone(replyLevel);
	floating->deleteHideLayout();
	floating->setParent(this);
	floating->setObjectName("reply");
	floating->setWindowFlags(Qt::ToolTip);
	floating->setWindowTitle("reply");
	floating->setAttribute(Qt::WA_DeleteOnClose);

	QPoint globalCursorPos = QCursor::pos();
	QSize sizeHint = floating->sizeHint();
	QRect rec = QApplication::desktop()->availableGeometry(this);
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
		floating->close();
		floating = Q_NULLPTR;
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
		//floating->setGeometry(x,y,sizeHint.width(),sizeHint.height());
		floating->move(x,y);
	}
}

void ThreadTab::reloadFilters(){
	foreach(ThreadForm *tf,tfMap){
		emit testFilters(tf->post);
	}
}

void ThreadTab::onFilterTest(QString no, bool filtered){
	QPointer<ThreadForm> tf = tfMap.value(no);
	if(!tf) return;
	tf->post.filtered = filtered;
	bool hidden = tf->isHidden();
	if(filtered && !hidden) removeTF(tf);
	else if(!filtered && hidden) showTF(tf);
}
