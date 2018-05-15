#include "boardtabhelper.h"
#include "netcontroller.h"
#include <QJsonArray>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>

BoardTabHelper::BoardTabHelper() {

}

void BoardTabHelper::startUp(Chan *api, QString board, BoardType type, QString search, QWidget *parent)
{
	this->api = api;
	this->parent = parent;
	this->board = board;
	this->type = type;
	this->search = search;
	if(type == BoardType::Index) boardUrl = api->boardURL(board);
	else boardUrl = api->catalogURL(board);
	qDebug() << "boardURL is" << boardUrl;
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	this->expandAll = settings.value("autoExpand",false).toBool();
	QDir().mkpath(board+"/index/thumbs");
	qDebug() << boardUrl;
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

void BoardTabHelper::writeJson(QString &board, QByteArray &rep) {
	QFile jsonFile(board+"/index.json");
	jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	jsonFile.write(rep);
	jsonFile.close();
}

void BoardTabHelper::getPostsFinished() {
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if(!reply) return;
	gettingReply = false;
	if(reply->error()) {
		qDebug().noquote() << "loading post error:" << reply->errorString();
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
	QJsonArray threads = filterThreads(rep);
	int length = threads.size();
	qDebug().noquote() << "length of" << boardUrl << "is" << QString::number(length);
	QtConcurrent::run(&BoardTabHelper::writeJson,board,rep);
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
		QString threadNum = QString("%1").arg(p.value("no").toDouble(),0,'f',0);
		if (idFilters.contains(threadNum)){
			qDebug("threadNum %s filtered!",threadNum.toLatin1().constData());
			i++;
			continue;
		}
		Post post(p,board);
		if(filterMe.filterMatched2(&post)){
			post.filtered = true;
		}
		allPosts.append(post.no);
		ThreadFormStrings tfString(post,threadNum,"index");
		emit newThread(post,tfString,loadFile);
		if(type==BoardType::Index && showIndexReplies){
			for(int j=1;j<t.size();j++){
				p = t.at(j).toObject();
				Post replyPost(p,board);
				ThreadFormStrings replyStrings(replyPost,threadNum,"index");
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
	if(type==BoardType::Index) threads = QJsonDocument::fromJson(rep).object().value("threads").toArray();
	else if(search.isEmpty()){
		QJsonArray allThreads = QJsonDocument::fromJson(rep).array();
		int numPages = allThreads.size();
		QJsonArray pageThreads;
		//-5 for some reason catalog goes blank on last 5 pages
		for(int i=0;i<numPages-5;i++){
			pageThreads = allThreads.at(i).toObject().value("threads").toArray();
			int numThreads = pageThreads.size();
			for(int j=0;j<numThreads;j++){
				threads.append(pageThreads.at(j));
			}
		}
	}
	else{ //TODO put this is static concurrent function with future or stream?
		qDebug("searching %s",search.toLatin1().constData());
		QRegularExpression re(search,QRegularExpression::CaseInsensitiveOption);
		QRegularExpressionMatch match;
		QJsonArray allThreads = QJsonDocument::fromJson(rep).array();
		QJsonArray pageThreads;
		for(int i=0;i<allThreads.size();i++) {
			pageThreads = allThreads.at(i).toObject().value("threads").toArray();
			for(int j=0;j<pageThreads.size();j++) {
				match = re.match(pageThreads.at(j).toObject().value("sub").toString() % pageThreads.at(j).toObject().value("com").toString());
				if(match.hasMatch())threads.append(pageThreads.at(j));
			}
		}
	}
	return threads;
}
