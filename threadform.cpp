#include "threadform.h"
#include "ui_threadform.h"
#include "filter.h"
#include "threadtab.h"
#include "mainwindow.h"
#include <QPixmap>
#include <QImageReader>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include <QSettings>
#include <QStringList>
#include <QListIterator>
#include <QPainter>

//TODO get rid of #include threadtab.h and mainwindow.h by using signals/slots
//TODO Possibly decouple the file and thumb getters to another class class
ThreadForm::ThreadForm(QString board, QString threadNum, PostType type, bool root, bool autoExpand, QWidget *parent, int replyLevel) :
	QWidget(parent), board(board), threadNum(threadNum), type(type), root(root), autoExpand(autoExpand), replyLevel(replyLevel), tab(parent),
	ui(new Ui::ThreadForm)
{
	if(root) rootTF = this;
	ui->setupUi(this);
	for(int i=0;i<replyLevel;i++){
		if(i == replyLevel-1){
			background.setRgb(darkness,darkness,darkness);
			ui->hide->setStyleSheet("padding: 0 10px; background-color:"+ background.name());
		}
		darkness = darkness*0.8;
	}
	background.setRgb(darkness,darkness,darkness);
	this->setStyleSheet("background-color:" + background.name() + "; color:#bbbbbb;");
	ui->quoteWidget->hide();
	ui->tim->hide();
	ui->fileInfo->hide();
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	setFontSize(settings.value("fontSize",14).toInt());
	//ui->replies->hide();
	//if(board != "pol") ui->country_name->hide();
	//this->setMinimumWidth(488);
	pathBase = "./" % board % "/" % ((type == PostType::Reply) ? threadNum : "index") % "/";
	connect(ui->hide,&ClickableLabel::clicked,this,&ThreadForm::hideClicked);
	connect(ui->com,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
	//connect(ui->replies,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
	connect(ui->info,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
	if(root) ui->hide->setStyleSheet("padding:0 10px; background-color: #191919;");

	//TODO quote for boardtab too
	//if(type==Reply)connect(ui->no,&ClickableLabel::clicked,this,&ThreadForm::appendQuote);
	//ui->replies->installEventFilter(this);
	ui->info->installEventFilter(this);
	ui->com->installEventFilter(this);
	this->installEventFilter(this);
	ui->tim->installEventFilter(this);
	//ui->name->installEventFilter(this);
	////ui->postLayout->installEventFilter(this);
	////ui->postLayout->installEventFilter(this);
}

void ThreadForm::appendQuote()
{
	static_cast<ThreadTab*>(tab)->quoteIt(">>"+post.no);
}

ThreadForm::~ThreadForm()
{
	//watcher.waitForFinished();
	if(gettingFile)replyImage->abort();
	if(gettingThumb)replyThumb->abort();
	//necessary because of the lambda functions?
	//disconnect(&watcher);
	delete ui;
}

void ThreadForm::setText(QString text)
{
	ui->com->setText(text);
}

QString ThreadForm::infoString(){
	return	"<span style=\"color: rgb(152, 125, 62); font-weight: bold;\">" + htmlParse(post.sub) + "</span> " +
			"<span style=\"color: rgb(163, 68, 67);\">" + post.name + "</span> " +
			((this->board == "pol") ? ("<span style=\"color: lightblue;\">" + post.country_name + "</span> ") : QString()) +
			"<span>" + post.realNow + "</span> " +
			"<a href=\"#op" % post.no % "\" style=\"color:#897399\">No." % post.no % "</a> " +
			repliesString;
}

void ThreadForm::load(QJsonObject &p)
{
	//set post number
	//post = new Post(p,board);
	post.load(p,board);
	ui->info->setText(infoString());
	/*ui->no->setText(post.no);

	//set subject
	if(post.sub==QString()) ui->sub->hide();
	else ui->sub->setText(htmlParse(post.sub));

	//set name
	ui->name->setText(post.name);
	ui->country_name->setText(post.country_name);*/

	//set comment
	//TODO replace <span class="quote"> and <a href="url">
	ui->com->setText(post.com);
	quotelinks = Filter::findQuotes(post.com);

	//set image
	//TODO clean if-else's
	//TODO use filedeleted image
	if(!post.tim.isNull() && !post.filedeleted) {
		fileURL = this->board % "/" % post.tim % post.ext;
		filePath = pathBase%post.no%"-"%post.filename%post.ext;
		file = new QFile(filePath,this);
		//TODO fsize in human readable format
		ui->fileInfo->show();
		QString infoText = post.filename % post.ext % " (" % QString("%1").arg(post.w) % "x" % QString("%1").arg(post.h) % ", " % QString("%1").arg(post.fsize/1024,0,'f',0) % " KB)";
		ui->fileInfo->setText(infoText);
		if((post.ext == QLatin1String(".jpg") || post.ext == QLatin1String(".png"))) {
			loadIt = true;
			if(!file->exists()) {
				if(autoExpand) getFile();
				thumbURL = this->board % "/" % post.tim % "s.jpg";
				thumbPath = pathBase%"thumbs/"%post.no%"-"%post.filename%"s.jpg";
				thumb = new QFile(thumbPath,this);
				if(!thumb->exists()) getThumb();
				else loadImage(thumbPath);
			}
			else{
				finished = true;
				loadImage(filePath);
			}
		}
		else {
			loadIt = false;
			if(autoExpand) {
				if(!file->exists()) {
					getFile();
				}
			}
			thumbURL = this->board % "/" % post.tim % "s.jpg";
			thumbPath = pathBase%"thumbs/"%post.no%"-"%post.filename%"s.jpg";
			thumb = new QFile(thumbPath,this);
			if(!thumb->exists()) getThumb();
			else loadImage(thumbPath);
		}
		connect(ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
	}
	else{
		delete ui->pictureLayout;
		ui->contentLayout->layout()->takeAt(0);
		this->hasImage = false;
		//delete ui->tim;
	}
	//updateComHeight();
}

void ThreadForm::getFile()
{
	qDebug().noquote() << QString("getting https://i.4cdn.org/") % fileURL;
	replyImage = nc.fileManager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % fileURL)));
	gettingFile = true;
	//connect(replyImage, &QNetworkReply::downloadProgress,this,&ThreadForm::downloading);
	connectionImage = connect(replyImage, &QNetworkReply::finished,this, &ThreadForm::getOrigFinished, Qt::UniqueConnection);
}

void ThreadForm::getThumb() {
	qDebug().noquote() << QString("getting https://i.4cdn.org/")  % thumbURL;
	replyThumb = nc.thumbManager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % thumbURL)));
	gettingThumb = true;
	connectionThumb = connect(replyThumb, &QNetworkReply::finished,this,&ThreadForm::getThumbFinished,Qt::UniqueConnection);
}

void ThreadForm::clickImage(){
	if(QPointer<ClickableLabel>(ui->tim)) ui->tim->clicked();
}

/*void ThreadForm::downloading(qint64 read, qint64 total)
{
	qDebug() << "downloading"+read;
	QByteArray b = replyImage->readAll();
	QDataStream out(file);
	out << b;
}*/

//TODO avoid copy pasted function
//TODO clean if-elses
void ThreadForm::loadOrig()
{
	if(!post.tim.isNull()) {
		if((post.ext == QLatin1String(".jpg") || post.ext == QLatin1String(".png"))) {
			loadIt = true;
			if(!file->exists()) getFile();
			else loadImage(filePath);
		}
		else {
			loadIt = false;
			if(!file->exists()) getFile();
		}
	}
}

void ThreadForm::updateComHeight()
{
	//ui->scrollArea->height()
	/*const QSize newSize = ui->com->sizeHint();
	const QSize oldSize = ui->scrollArea->size();
	//ui->scrollArea->setMaximumHeight(newSize.height());
	//ui->com->setMinimumSize(newSize);
	if(newSize.height() > oldSize.height()) {
		if(type == PostType::Thread) {
			if(newSize.height() > 500) ui->scrollArea->setFixedHeight(500);
			else ui->scrollArea->setFixedHeight(newSize.height());
		}
		else ui->scrollArea->setFixedHeight(newSize.height());
	}*/
	//int docHeight = ui->com->document()->size().height();
	/*int docHeight = ui->com->height();
	int newHeight = docHeight+ui->sub->height()+ui->verticalLayout_2->BottomToTop;
	if(newHeight > this->height()) {
		if(type == PostType::Reply) {
			ui->com->setMinimumHeight(docHeight);
			this->setFixedHeight(newHeight);
			this->setMaximumHeight(newHeight);
		}
		else if(newHeight < 500) {
			ui->com->setMinimumHeight(docHeight);
			this->setFixedHeight(newHeight);
			this->setMaximumHeight(newHeight);
		}
		else{
			this->setFixedHeight(500);
			this->setMaximumHeight(500);
		}
	}*/
}

void ThreadForm::getOrigFinished()
{
	disconnect(connectionImage);
	gettingFile = false;
	if(replyImage->canReadLine() && replyImage->error() == 0)
	{
		disconnect(connectionThumb);
		finished = true;
		file->open(QIODevice::WriteOnly);
		file->write(replyImage->readAll());
		file->close();
		replyImage->deleteLater();
		qDebug().noquote() << "saved file "+filePath;
		if(loadIt) {
			loadImage(filePath);
			QListIterator<QPointer<ThreadForm>> i(rootTF->clones);
			QPointer<ThreadForm> cloned;
			while(i.hasNext()) {
				cloned = i.next();
				if(!cloned) continue;
				cloned->loadImage(filePath);
			}
		}
		emit fileFinished();
	}
	else {
		qDebug() << "getting file:" << filePath << "error. Reason:" << replyImage->errorString();
		replyImage->deleteLater();
	}
}

void ThreadForm::getThumbFinished()
{
	disconnect(connectionThumb);
	gettingThumb = false;
	if(replyThumb->error() == 0) {
		thumb->open(QIODevice::WriteOnly);
		thumb->write(replyThumb->readAll());
		thumb->close();
		replyThumb->deleteLater();
		qDebug().noquote() << "saved file "+thumbPath;
		//check if orig file somehow dl'd faster than thumbnail
		if(!finished) loadImage(thumbPath);
	}
	else {
		qDebug() << "getting file:" << filePath << "error. Reason:" << replyThumb->errorString();
		replyThumb->deleteLater();
	}
}

QImage ThreadForm::scaleImage(QString path, int scale)
{
	QImage pic;
	pic.load(path);
	QImage scaled = (pic.height() > pic.width()) ?
				pic.scaledToHeight(scale, Qt::SmoothTransformation) :
				pic.scaledToWidth(scale, Qt::SmoothTransformation);
	return scaled;
}

void ThreadForm::loadImage(QString path) {
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	/*QFuture<QImage> newImage = QtConcurrent::run(scaleImage,
												 path, settings.value("imageSize",250).toInt());
	connect(&watcher, &QFutureWatcherBase::finished,[=]()
	{
		QImage scaled = newImage.result();
		if(!scaled.isNull()) {
			ui->tim->show();
			//this->setMinimumWidth(738);
			ui->tim->setPixmap(QPixmap::fromImage(scaled));
			ui->tim->setFixedSize(scaled.size());
			//if(this->type == PostType::Reply) {
			//	static_cast<ThreadTab*>(tab)->checkScroll();
			//}
		}
	});
	watcher.setFuture(newImage);*/
	QImage scaled = scaleImage(path, settings.value("imageSize",250).toInt());
	if(!scaled.isNull()) {
		ui->tim->show();
		ui->tim->setPixmap(QPixmap::fromImage(scaled));
		ui->tim->setFixedSize(scaled.size());
	}
}

void ThreadForm::imageClicked()
{
	qDebug().noquote() << "clicked "+post.filename;
	if(this->type == PostType::Reply) {
		if(!QPointer<ClickableLabel>(ui->tim) || (ui->tim && ui->tim->isHidden())) return;
		if(!post.filename.isEmpty() && file && !file->exists() && !gettingFile) {
			qDebug().noquote() << QString("getting https://i.4cdn.org/")  % fileURL;
			gettingFile=true;
			replyImage = nc.fileManager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % fileURL)));
			//connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::loadFromImageClicked);
			connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::imageClickedFinished,Qt::UniqueConnection);
		}
		else if(gettingFile) {
			connect(this,&ThreadForm::fileFinished,this,&ThreadForm::imageClickedFinished,Qt::UniqueConnection);
		}
		else openImage();
	}
	else{
		TreeItem *childOf = mw->model->getItem(mw->selectionModel->currentIndex());
		mw->onNewThread(mw,board,threadNum,QString(),childOf);
	}
}

void ThreadForm::imageClickedFinished()
{
	getOrigFinished();
	openImage();
}

void ThreadForm::hideClicked()
{
	this->hide();
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
	idFilters.append(threadNum);
	settings.setValue("filters/"+board+"/id",idFilters);
	qDebug().noquote() << "hide Clicked so "+threadNum+" filtered!";
	if(this->type == Reply){
		QListIterator<QPointer<ThreadForm>> i(clones);
		while(i.hasNext()) {
			i.next()->deleteLater();
		}
		QSet<QString> quotes = quotelinks;
		ThreadForm *replyTo;
		foreach (const QString &orig, quotes)
		{
			replyTo = static_cast<ThreadTab*>(tab)->tfMap.find(orig).value();
			if(replyTo != nullptr) {
				//replyTo->replies.insert(tf->post.no);
				replyTo->replies.remove(post.no.toDouble());
				replyTo->setReplies();
			}
		}
	}
	this->hidden = true;
	this->close();
	emit removeMe(this);
}

void ThreadForm::openImage()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(QDir().absoluteFilePath(filePath)));
}

QString ThreadForm::htmlParse(QString &html)
{
	return html.replace("<br>","\n").replace("&amp;","&")
			.replace("&gt;",">").replace("&lt;","<")
			.replace("&quot;","\"").replace("&#039;","'")
			.replace("<wb>","\n").replace("<wbr>","\n");
}

void ThreadForm::setFontSize(int fontSize){
	QFont temp = this->ui->info->font();
	temp.setPointSize(fontSize-4);
	this->ui->fileInfo->setFont(temp);
	temp.setPointSize(fontSize-2);
	this->ui->info->setFont(temp);
	temp.setPointSize(fontSize);
	this->ui->com->setFont(temp);
	if(root && clones.size()){
		QListIterator<QPointer<ThreadForm>> i(clones);
		while(i.hasNext()) {
			QPointer<ThreadForm> next = i.next();
			if(!next) continue;
			next->setFontSize(fontSize);
		}
	}
}

void ThreadForm::setImageSize(int imageSize){
	if(file && file->exists()) loadImage(filePath);
	else if(thumb && thumb->exists()) loadImage(thumbPath);
	if(root && clones.size()){
		QListIterator<QPointer<ThreadForm>> i(clones);
		while(i.hasNext()) {
			QPointer<ThreadForm> next = i.next();
			if(!next) continue;
			next->setImageSize(imageSize);
		}
	}

}

QString ThreadForm::titleParse(QString &title)
{
	QRegularExpression htmlTag;
	htmlTag.setPattern("<span .*>");
	return title.replace(htmlTag,"").replace("<br>"," ").replace("&amp;","&")
			.replace("&gt;",">").replace("&lt;","<")
			.replace("&quot;","\"").replace("&#039;","'")
			.replace("<wb>"," ").replace("<wbr>"," ");
}

void ThreadForm::quoteClicked(const QString &link)
{
	qDebug().noquote() << link;
	//check size > 2 instead of isempty?
	if(link.startsWith("#p") && this->type == PostType::Reply) {
		ThreadForm *tf = static_cast<ThreadTab*>(tab)->findPost(link.mid(2));
		if(tf != nullptr && !tf->hidden) this->insert(tf);
	}
	else if(link.startsWith("#op")){
		if(this->type == PostType::Reply) static_cast<ThreadTab*>(tab)->quoteIt(">>"+post.no);
		else imageClicked();
	}
	else if(!link.isEmpty() && link.at(0)=='/') {
		mw->loadFromSearch(link,QString(),Q_NULLPTR,false);
	}
}

void ThreadForm::insert(ThreadForm *tf)
{
	ThreadForm *newtf = tf->clone(replyLevel);
	if(ui->quoteWidget->isHidden())ui->quoteWidget->show();
	ui->quotes->addWidget(newtf);
	newtf->show();
	//newtf->setMinimumWidth(newtf->sizeHint().width());
	//this->setMinimumWidth(this->sizeHint().width());
	//((ThreadTab*)tab)->updateWidth();
	//this->update();
}

void ThreadForm::addReply(ThreadForm *tf){
	if(ui->quoteWidget->isHidden())ui->quoteWidget->show();
	ui->quotes->addWidget(tf);
}

ThreadForm *ThreadForm::clone(int replyLevel)
{
	//TODO? just tfs->load(post);
	ThreadForm *tfs = new ThreadForm(this->board,this->threadNum,this->type,false,false,tab,replyLevel+1);
	tfs->rootTF = this->rootTF;
	tfs->tab = tab;
	tfs->post = this->post;
	//tfs->ui->no->setText(post.no);
	tfs->ui->com->setText(post.com);
	//if(post.sub==QString()) tfs->ui->sub->hide();
	//else tfs->ui->sub->setText(this->ui->sub->text());
	//tfs->ui->name->setText(post.name);
	/*if(this->board != "pol") {
		tfs->ui->country_name->hide();
	} else {
		tfs->ui->country_name->setText(post.country_name);
	}*/
	tfs->ui->info->setText(ui->info->text());
	tfs->replies = replies;
	//TODO check and account for if original is still getting file
	if(!post.tim.isNull() && !post.filedeleted) {
		tfs->fileURL = fileURL;
		tfs->filePath = filePath;
		tfs->file = file;
		tfs->thumbURL = thumbURL;
		tfs->thumbPath = thumbPath;
		tfs->thumb = thumb;
		tfs->ui->fileInfo->show();
		tfs->ui->fileInfo->setText(ui->fileInfo->text());
		const QPixmap *px = this->ui->tim->pixmap();
		//From load image but don't have to scale again
		tfs->ui->tim->show();
		//tfs->setMinimumWidth(738);
		if(px){
			tfs->ui->tim->setPixmap(*px);
			tfs->ui->tim->setFixedSize(px->size());
			connect(tfs->ui->tim,&ClickableLabel::clicked,rootTF,&ThreadForm::imageClicked);
		}
	} else {
		tfs->ui->pictureLayout->deleteLater();
		tfs->ui->contentLayout->layout()->takeAt(0);
		tfs->hasImage = false;
		//tfs->ui->tim->deleteLater();
	}
	if(repliesString.length()) {
		tfs->repliesString = repliesString;
		tfs->setRepliesString();
	}
	//tfs->setReplies();
	rootTF->clones.append(tfs);
	disconnect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::hideClicked);
	connect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::deleteLater);
	connect(tfs,&ThreadForm::removeMe,rootTF,&ThreadForm::removeClone);
	//TODO load and connect cross thread replies
	if(this->type == PostType::Reply){
		ThreadTab* temp = static_cast<ThreadTab*>(tab);
		connect(tfs,&ThreadForm::floatLink,temp,&ThreadTab::floatReply);
		connect(tfs,&ThreadForm::updateFloat,temp,&ThreadTab::updateFloat);
		connect(tfs,&ThreadForm::deleteFloat,temp,&ThreadTab::deleteFloat);
	}
	return tfs;
}

//TODO don't run this if destroying the whole threadtab
void ThreadForm::removeClone(QPointer<ThreadForm> tf)
{
	if(tf) clones.removeOne(tf);
}
void ThreadForm::addReplyLink(QString &reply){
	QString temp =" <a href=\"#p" % reply % "\" style=\"color:#897399\">>>" % reply % "</a>";
	repliesString += temp;
	ui->info->setText(infoString());
}

void ThreadForm::setReplies()
{
	repliesString = "";
	QList<QString> list = replies.values();
	if(list.length()) {
		//ui->replies->show();
		foreach (const QString &reply, list)
		{
			repliesString+=" <a href=\"#p" % reply % "\" style=\"color:#897399\">>>" % reply % "</a>";
		}
		repliesString = repliesString.mid(1);
		ui->info->setText(infoString());
		//ui->replies->setText(repliesString);
		//also set replies for all clones
		QListIterator<QPointer<ThreadForm>> i(clones);
		//qDebug() << clones;
		while(i.hasNext()) {
			QPointer<ThreadForm> next = i.next();
			if(!next) continue;
			next->repliesString = repliesString;
			next->setInfoString();
			//next->setRepliesString();
		}
	}
	else{
		//ui->replies->hide();
	}
}

void ThreadForm::setRepliesString()
{
	//ui->replies->show();
	//ui->replies->setText(repliesString);
}

void ThreadForm::setInfoString()
{
	ui->info->setText(infoString());
}

//showing and supposedToShow to save states and restore floating replies and quotes
void ThreadForm::on_info_linkHovered(const QString &link)
{
	if(this->type == PostType::Reply) {
		if(link.startsWith("#op")){
			emit deleteFloat();
		}
		else if(link.startsWith("#p")) {
			qDebug().noquote() << "hovering" << link;
			emit floatLink(link.mid(2),replyLevel);
		} else {
			//TODO check mouse cursor?
			emit deleteFloat();
		}
	}
}

bool ThreadForm::eventFilter(QObject *obj, QEvent *event)
{
	if(this->type == PostType::Reply){
		if(event->type() == QEvent::MouseMove) {
			emit updateFloat();
		}
		else if(event->type() == QEvent::Leave) {
			emit deleteFloat();
		}
	}
	return QObject::eventFilter(obj, event);
}

void ThreadForm::on_com_linkHovered(const QString &link)
{
	on_info_linkHovered(link);
}

void ThreadForm::deleteHideLayout()
{
	delete this->ui->hideLayout;
	delete this->ui->quoteWidget;
	hideButtonShown = false;
}

void ThreadForm::paintEvent(QPaintEvent *){
	QPainter painter(this);
	int x = 0;
	if(hideButtonShown){
		painter.setPen(QColor(100,100,100));
		x = ui->hide->width();
	}
	else
		painter.setPen(QColor(75,75,75));
	painter.drawRect(x,0,width()-x-1,height()-1);
	painter.fillRect(x+1,1,width()-x-2,height()-2,background);
}

/*void ThreadForm::setBorder() {
	ui->com->setStyleSheet("bottom-border:3px solid black");
}

void ThreadForm::removeBorder() {
	ui->com->setStyleSheet("");
}*/
