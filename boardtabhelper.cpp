#include "boardtabhelper.h"
#include "netcontroller.h"
#include <QJsonArray>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>

BoardTabHelper::BoardTabHelper() {
	qRegisterMetaType<Post>("Post");
	qRegisterMetaType<ThreadFormStrings>("ThreadFormStrings");
}

void BoardTabHelper::startUp(Chan *api, QString board, BoardType type, QString search, QWidget *parent)
{
	this->api = api;
	this->parent = parent;
	this->board = board;
	this->type = type;
	this->search = search;
	this->postKeys = api->postKeys();
	if(type == BoardType::Index) boardUrl = api->boardURL(board);
	else boardUrl = api->catalogURL(board);
	title = api->name() % '/' % board;
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	this->expandAll = settings.value("autoExpand",false).toBool();
	filesPath = api->name() + '/' + board + "/index/";
	QDir().mkpath(filesPath+"thumbs");
	request = QNetworkRequest(QUrl(boardUrl));
	if(api->requiresUserAgent()){
		request.setHeader(QNetworkRequest::UserAgentHeader,api->requiredUserAgent());
	}
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
	filterMe.filters2 = filter.filterMatchedPerTab(board,"board");
	/*updateTimer = new QTimer();
	updateTimer->setInterval(60000);
	updateTimer->start();
	if(settings.value("autoUpdate").toBool()) {
		connectionUpdate = connect(updateTimer, &QTimer::timeout,
								   this,&BoardTabHelper::getPosts,UniqueDirect);
	}*/
	emit getPosts();
}

BoardTabHelper::~BoardTabHelper() {
	abort = true;
	disconnect(connectionPost);
}

void BoardTabHelper::setAutoUpdate(bool update) {
	(void)update;
	/*if(update) {
		connectionUpdate = connect(updateTimer, &QTimer::timeout,
								   this,&BoardTabHelper::getPosts,UniqueDirect);
	}*/
}

void BoardTabHelper::writeJson(QString &path, QByteArray &rep) {
	QFile jsonFile(path+"index.json");
	jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	jsonFile.write(rep);
	jsonFile.close();
}

void BoardTabHelper::getPostsFinished() {
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if(!reply) return;
	gettingReply = false;
	if(reply->error()) {
		qDebug().noquote() << "error" << title << ": " << reply->errorString();
		if(reply->error() == QNetworkReply::ContentNotFoundError) {
			//qDebug() << "Stopping timer for" << threadUrl;
			//updateTimer->stop();
			emit boardStatus("404");
		}
		//QT bug (network does not refresh after resume from suspend) workaround
		else if(reply->error() == QNetworkReply::UnknownNetworkError){
			nc.refreshManagers();
		}
		reply->deleteLater();
		return;
	}
	//write to file and make json array
	QByteArray rep = reply->readAll();
	reply->deleteLater();
	if(rep.isEmpty()) return;
	emit clearMap();
	api->replacements(rep);
	QJsonArray threads = filterThreads(rep);
	int length = threads.size();
	qDebug().noquote().nospace() << "got " << title << ": length is " << QString::number(length);
	QtConcurrent::run(&BoardTabHelper::writeJson,filesPath,rep);
	//load new posts
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	bool loadFile = settings.value("autoExpand",false).toBool() || this->expandAll;
	bool showIndexReplies = settings.value("showIndexReplies",false).toBool();
	QStringList idFilters = settings.value("filters/"+board+"/id").toStringList();
	QJsonArray t;
	QJsonObject p;
	int i = 0;
	while(i<length){
		if(type==BoardType::Index){
			t = threads.at(i).toObject().value("posts").toArray();
			p = t.at(0).toObject();
		}
		else{
			p = threads.at(i).toObject();
		}
		//deprecated
		QString threadNum = QString("%1").arg(p.value(api->postKeys().no).toDouble(),0,'f',0);
		if (idFilters.contains(threadNum)){
			qDebug("threadNum %s filtered!",threadNum.toLatin1().constData());
			i++;
			continue;
		}
		//
		//TODO post doesn't need threadnum?
		QString empty;
		Post post = api->post(p,board,empty);
		if(filterMe.filterMatched2(&post)){
			post.filtered = true;
		}
		QString thread = post.no;
		allPosts.append(thread);
		ThreadFormStrings tfString(api,post,thread,"index");
		emit newThread(post,tfString,loadFile);
		if(type==BoardType::Index && showIndexReplies){
			for(int j=1;j<t.size();j++){
				p = t.at(j).toObject();
				Post replyPost = api->post(p,board,thread);
				ThreadFormStrings replyStrings(api,replyPost,thread,"index");
				emit newReply(replyPost,replyStrings,threadNum);
			}
		}
		i++;
	}
}

void BoardTabHelper::reloadFilters(){
	filterMe.filters2 = filter.filterMatchedPerTab(this->board,"thread");
	emit startFilterTest();
}

void BoardTabHelper::filterTest(Post p){
	emit filterTested(p.no,filterMe.filterMatched2(&p));
}


QJsonArray BoardTabHelper::filterThreads(QByteArray &rep){
	QJsonArray threads;
	if(type==BoardType::Index){
		threads = api->threadsArray(rep);
	}
	else if(search.isEmpty()){
		QJsonArray allThreads = api->catalogArray(rep);
		int numPages = allThreads.size();
		//-5 for some reason catalog goes blank on last 5 pages
		for(int i=0;i<numPages-5;i++){
			QJsonArray pageThreads = api->catalogPageArray(allThreads,i);
			int numThreads = pageThreads.size();
			for(int j=0;j<numThreads;j++){
				threads.append(pageThreads.at(j));
			}
		}
	}
	else{ //TODO put this is static concurrent function with future or stream?
		QJsonArray allThreads = api->catalogArray(rep);
		int numPages = allThreads.size();
		qDebug("searching %s",search.toLatin1().constData());
		QRegularExpression re(search,QRegularExpression::CaseInsensitiveOption);
		for(int i=0;i<numPages;i++) {
			QJsonArray pageThreads = api->catalogPageArray(allThreads,i);
			for(int j=0;j<pageThreads.size();j++) {
				QJsonObject post = pageThreads.at(j).toObject();
				QRegularExpressionMatch match = re.match(post.value(api->postKeys().sub).toString()
								 % post.value(api->postKeys().com).toString());
				if(match.hasMatch())threads.append(pageThreads.at(j));
			}
		}
	}
	return threads;
}
