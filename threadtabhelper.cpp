#include "threadtabhelper.h"
#include "netcontroller.h"
#include "you.h"
#include "threadformstrings.h"
#include <QJsonArray>

ThreadTabHelper::ThreadTabHelper() {
	qRegisterMetaType<Post>("Post");
	qRegisterMetaType<ThreadFormStrings>("ThreadFormStrings");
}

void ThreadTabHelper::startUp(Chan *api, QString board, QString thread, QWidget *parent, bool isFromSession = false)
{
	this->parent = parent;
	this->thread = thread;
	this->board = board;
	this->isFromSession = isFromSession;
	this->api = api;
	postKeys = api->postKeys();
	//TODO check type
	this->threadUrl = api->threadURL(board,thread);
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	this->expandAll = settings.value("autoExpand",false).toBool();
	QDir().mkpath(board+"/"+thread+"/thumbs");
	qDebug() << threadUrl;

	filterMe.filters2 = filter.filterMatchedPerTab(board,"thread");

	//self-archive check
	QFile jsonFile(board+"/"+thread+"/"+thread+".json");
	if(jsonFile.exists() && jsonFile.open(QIODevice::ReadOnly)){
		QByteArray archive = jsonFile.readAll();
		loadPosts(archive,false);
	}

	//get posts
	request = QNetworkRequest(QUrl(threadUrl));
	if(api->requiresUserAgent()) request.setHeader(QNetworkRequest::UserAgentHeader,api->requiredUserAgent());
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
	if(settings.value("extraFlags/enable",false).toBool()){
		requestFlags.setUrl(QUrl("https://flagtism.drunkensailor.org/int/get_flags_api2.php"));
		requestFlags.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	}
	emit getPosts();
}

ThreadTabHelper::~ThreadTabHelper() {
	abort = true;
}

void ThreadTabHelper::writeJson(QString &board, QString &thread, QByteArray &rep) {
	QFile jsonFile(board+"/"+thread+"/"+thread+".json");
	jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	jsonFile.write(rep);
	jsonFile.close();
}

void ThreadTabHelper::getExtraFlags(){
	if(abort || !(board == "int" || board == "pol" || board == "sp" || board == "int")) return;
	QStringList postNums;
	foreach(QString no, allPosts){
		if(abort) return;
		if(gottenFlags.contains(no)) continue;
		gottenFlags.insert(no);
		postNums.append(no);
	}
	if(!postNums.size()) return;
	QString data = "board=" % board % "&post_nrs=" % postNums.join("%2C");
	qDebug() << data;
	emit getFlags(data.toUtf8());
	/*replyFlags = nc.fileManager->post(requestFlags,data.toUtf8());
	flagsConnection = connect(replyFlags,SIGNAL(finished()),this,SLOT(loadExtraFlags()),UniqueDirect);*/
}

void ThreadTabHelper::loadExtraFlags(){
	QNetworkReply *replyFlags = qobject_cast<QNetworkReply*>(sender());
	if(abort || !replyFlags) return;
	if(replyFlags->error()){
		qDebug() << "checking flags errror:" << replyFlags->errorString();
		return;
	}
	QByteArray answer = replyFlags->readAll();
	replyFlags->deleteLater();
	if(answer.isEmpty()) return;
	QJsonArray extraFlags = QJsonDocument::fromJson(answer).array();
	int length = extraFlags.size();
	if(!length) return;
	qDebug() << "loading" << length << "extra flag posts";
	QJsonObject ef;
	for(int i=0;i<length;i++){
		ef = extraFlags.at(i).toObject();
		QString post_nr = ef.value("post_nr").toString();
		//gottenFlags.insert(post_nr);
		QString region = ef.value("region").toString();
		emit setRegion(post_nr,region);
	}
}

void ThreadTabHelper::getPostsFinished() {
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if(abort || !reply) return;
	gettingReply = false;
	if(reply->error()) {
		qDebug().noquote() << "loading post error:" << reply->errorString();
		if(reply->error() == QNetworkReply::ContentNotFoundError) {
			emit threadStatus("404");
		}
		//QT bug (network does not refresh after resume from suspend) workaround
		else if(reply->error() == QNetworkReply::UnknownNetworkError){
			nc.refreshManagers();
		}
		return;
	}
	QByteArray rep = reply->readAll();
	reply->deleteLater();
	loadPosts(rep);
}

void ThreadTabHelper::loadPosts(QByteArray &postData, bool writeIt){
	QJsonArray posts = api->postsArray(postData,"thread");
	//QJsonArray posts = QJsonDocument::fromJson(postData).object().value("posts").toArray();
	int length = posts.size();
	qDebug().noquote() << QString("length of ").append(threadUrl).append(" is ").append(QString::number(length));
	//check OP if archived or closed
	QJsonObject p;
	if(length) {
		p = posts.at(0).toObject();
		Post OP(p,postKeys,board);
		if(OP.archived) {
			qDebug().nospace() << "Stopping timer for " << threadUrl <<". Reason: Archived";
			emit threadStatus("archived",OP.archived_on);
		}
		else if(OP.closed) {
			qDebug().nospace() << "Stopping timer for " << threadUrl <<". Reason: Closed";
			emit threadStatus("closed");
		}
	}
	else return;
	//write to file
	if(writeIt) QtConcurrent::run(&ThreadTabHelper::writeJson, board, thread, postData);
	//load new posts
	int i = allPosts.size();
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	bool loadFile = settings.value("autoExpand",false).toBool() || this->expandAll;
	while(i<length) {
		Post post(posts.at(i).toObject(),postKeys,board);
		if(filterMe.filterMatched2(&post)){
			post.filtered = true;
		}
		ThreadFormStrings tfString(post,thread,thread);
		allPosts.append(post.no);

		emit newTF(post,tfString,loadFile);
		if(post.hasYou){
			emit addNotification(post.no);
		}

		//Update windowTitle with OP info
		if(i==0) {
			if(post.sub.length()){
				emit windowTitle("/"+board+"/"+thread + " - " + post.sub);
				emit tabTitle(post.sub);
			}
			else if(post.com.length()) {
				QString temp = post.com;
				//TODO clean this
				temp = Filter::htmlParse(temp)
						.replace("\n"," ")
						.remove(QRegExp("<[^>]*>"));
				emit windowTitle("/"+board+"/"+thread + " - " + temp);
				if(!isFromSession) emit tabTitle(temp);
			}
		}
		foreach (const QString &orig, post.quotelinks) {
			emit addReply(orig,post.no,post.isYou);
		}
		i++;
	}
	if(settings.value("extraFlags/enable",false).toBool()) getExtraFlags();
	//emit scrollIt();
}

void ThreadTabHelper::reloadFilters(){
	filterMe.filters2 = filter.filterMatchedPerTab(this->board,"thread");
	emit startFilterTest();
}

void ThreadTabHelper::filterTest(Post p){
	emit filterTested(p.no,filterMe.filterMatched2(&p));
}
