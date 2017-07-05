#include "threadform.h"
#include "ui_threadform.h"
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

//TODO Possibly refactor file checks and pointers to dir and file objects
//TODO Possibly decouple the file and thumb getters to the post class
ThreadForm::ThreadForm(QString board, QString threadNum, PostType type, bool root, bool autoExpand, QWidget *parent) :
	QWidget(parent), board(board), threadNum(threadNum), type(type), root(root), autoExpand(autoExpand), tab(parent),
	ui(new Ui::ThreadForm)
{
	ui->setupUi(this);
	ui->tim->hide();
	ui->horizontalSpacer->changeSize(0,0);
	//ui->replies->hide();
	//if(board != "pol") ui->country_name->hide();
	this->setMinimumWidth(488);
	pathBase = "./" % board % "/" % ((type == PostType::Reply) ? threadNum : "index") % "/";
	connect(ui->hide,&ClickableLabel::clicked,this,&ThreadForm::hideClicked);
	connect(ui->com,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
	//connect(ui->replies,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
	connect(ui->info,&QLabel::linkActivated,this,&ThreadForm::quoteClicked);
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
	watcher.waitForFinished();
	if(gettingFile)replyImage->abort();
	if(gettingThumb)replyThumb->abort();
	//necessary because of the lambda functions?
	disconnect(&watcher);
	delete ui;
}

void ThreadForm::setText(QString text)
{
	ui->com->setText(text);
}

QString ThreadForm::infoString(){
	return	"<span style=\"color: rgb(152, 125, 62); font-weight: bold;\">" + htmlParse(post.sub) + "</span> " +
			"<span style=\"color: rgb(163, 68, 67);\">" + post.name + "</span> " +
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
		//ui->pictureLayout->deleteLater();
		delete ui->pictureLayout;
	}
	this->show();
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
	qDebug().noquote() << QString("getting https://t.4cdn.org/")  % thumbURL;
	replyThumb = nc.thumbManager->get(QNetworkRequest(QUrl("https://t.4cdn.org/" % thumbURL)));
	gettingThumb = true;
	connectionThumb = connect(replyThumb, &QNetworkReply::finished,this,&ThreadForm::getThumbFinished,Qt::UniqueConnection);
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
			QListIterator<QPointer<ThreadForm>> i(clones);
			while(i.hasNext()) {
				ThreadForm *cloned = i.next();
				if(!cloned) return;
				cloned->loadImage(cloned->filePath);
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

QImage ThreadForm::scaleImage(QString path)
{
	int scale = 250;
	QImage pic;
	pic.load(path);
	QImage scaled = (pic.height() > pic.width()) ?
				pic.scaledToHeight(scale, Qt::SmoothTransformation) :
				pic.scaledToWidth(scale, Qt::SmoothTransformation);
	return scaled;
}

void ThreadForm::loadImage(QString path) {
	QFuture<QImage> newImage = QtConcurrent::run(scaleImage, path);
	connect(&watcher, &QFutureWatcherBase::finished,[=]()
	{
		QImage scaled = newImage.result();
		if(!scaled.isNull()) {
			ui->tim->show();
			//ui->horizontalSpacer->changeSize(250,0);
			//ui->horizontalSpacer->invalidate();
			this->setMinimumWidth(738);
			ui->tim->setPixmap(QPixmap::fromImage(scaled));
			ui->tim->setMaximumSize(scaled.size());
			/*if(this->type == PostType::Reply) {
				static_cast<ThreadTab*>(tab)->checkScroll();
			}*/
		}
	});
	watcher.setFuture(newImage);
}

void ThreadForm::imageClicked()
{
	qDebug().noquote() << "clicked "+post.filename;
	if(this->type == PostType::Reply) {
		if(!file->exists() && !gettingFile) {
			qDebug().noquote() << QString("getting https://i.4cdn.org/")  % fileURL;
			gettingFile=true;
			replyImage = nc.fileManager->get(QNetworkRequest(QUrl("https://i.4cdn.org/" % fileURL)));
			//connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::loadFromImageClicked);
			connectionImage = connect(replyImage, &QNetworkReply::finished,this,&ThreadForm::alreadyClicked,Qt::UniqueConnection);
		}
		else if(gettingFile) {
			connect(this,&ThreadForm::fileFinished,this,&ThreadForm::openImage,Qt::UniqueConnection);
		}
		else openImage();
	}
	else{
		TreeItem *childOf = mw->model->getItem(mw->selectionModel->currentIndex());
		mw->onNewThread(this,board,threadNum,QString(),childOf);
	}
}

void ThreadForm::alreadyClicked()
{
	getOrigFinished();
	openImage();
}

void ThreadForm::hideClicked()
{
	this->hide();
	QSettings settings;
	QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
	idFilters.append(threadNum);
	settings.setValue("filters/"+board+"/id",idFilters);
	qDebug().noquote() << "hide Clicked so "+threadNum+" filtered!";
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
		static_cast<ThreadTab*>(tab)->quoteIt(">>"+post.no);
	}
	else if(!link.isEmpty() && link.at(0)=='/') {
		mw->loadFromSearch(link,QString(),Q_NULLPTR,false);
	}
}

void ThreadForm::insert(ThreadForm *tf)
{
	ThreadForm *newtf = tf->clone();
	ui->quotes->addWidget(newtf);
	newtf->show();
	//newtf->setMinimumWidth(newtf->sizeHint().width());
	//this->setMinimumWidth(this->sizeHint().width());
	//((ThreadTab*)tab)->updateWidth();
	//this->update();
}

void ThreadForm::addReply(ThreadForm *tf){
	ui->quotes->addWidget(tf);
}

ThreadForm *ThreadForm::clone()
{
	//TODO? just tfs->load(post);
	ThreadForm *tfs = new ThreadForm(this->board,this->threadNum,this->type,false,false,tab);
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
		const QPixmap *px = this->ui->tim->pixmap();
		//From load image but don't have to scale again
		tfs->ui->tim->show();
		//tfs->ui->horizontalSpacer->changeSize(250,0);
		//tfs->ui->horizontalSpacer->invalidate();
		tfs->setMinimumWidth(738);
		if(px){
			tfs->ui->tim->setPixmap(*px);
			tfs->ui->tim->setMaximumSize(px->size());
			connect(tfs->ui->tim,&ClickableLabel::clicked,this,&ThreadForm::imageClicked);
		}
	} else {
		tfs->ui->pictureLayout->deleteLater();
	}
	if(repliesString.length()) {
		tfs->repliesString = repliesString;
		tfs->setRepliesString();
	}
	//tfs->setReplies();
	this->clones.append(tfs);
	disconnect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::hideClicked);
	connect(tfs->ui->hide,&ClickableLabel::clicked,tfs,&ThreadForm::deleteLater);
	connect(tfs,&ThreadForm::removeMe,this,&ThreadForm::removeClone);
	//TODO load and connect cross thread replies
	if(this->type == PostType::Reply)
		connect(tfs,&ThreadForm::floatLink,static_cast<ThreadTab*>(tab),&ThreadTab::floatReply);
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

bool ThreadForm::eventFilter(QObject *obj, QEvent *event)
{
	//check cursor type instead?
	//qDebug() << cursor.shape();
	/*if (event->type() == QEvent::CursorChange) {
		QMouseEvent *mE = static_cast<QMouseEvent *>(event);
		if(cursor.shape()==Qt::PointingHandCursor) {
			qDebug() << "do it";
		}
		return QObject::eventFilter(obj, event);
	}*/
	if(event->type() == QEvent::MouseMove) {
		if(this->type == PostType::Reply)
			static_cast<ThreadTab*>(tab)->updateFloat();
	} else if(event->type() == QEvent::Leave) {
		if(this->type == PostType::Reply)
			static_cast<ThreadTab*>(tab)->deleteFloat();
	} else if(event->type() == QEvent::DragEnter) {
		return true;
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
}

void ThreadForm::paintEvent(QPaintEvent *e){
	QPainter painter(this);
	int x = 0;
	if(root){
		painter.setPen(QColor(100,100,100));
		x = ui->hide->width();
	}
	else
		painter.setPen(QColor(75,75,75));
	painter.drawRect(x,0,width()-x-1,height()-1);
	QWidget::paintEvent(e);
}

/*void ThreadForm::setBorder() {
	ui->com->setStyleSheet("bottom-border:3px solid black");
}

void ThreadForm::removeBorder() {
	ui->com->setStyleSheet("");
}*/

void ThreadForm::on_info_linkHovered(const QString &link)
{
	if(this->type == PostType::Reply) {
		if(link.startsWith("#op")){
			static_cast<ThreadTab*>(tab)->deleteFloat();
		}
		else if(link.startsWith("#p")) {
			//qDebug().noquote() << "hovering" << link;
			emit floatLink(link.mid(2));
		} else {
			//TODO check mouse cursor?
			static_cast<ThreadTab*>(tab)->deleteFloat();
		}
	}
}
