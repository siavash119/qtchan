#include "threadtabhelper.h"
#include "netcontroller.h"
#include "you.h"
#include "notificationview.h"
#include <QJsonArray>

ThreadTabHelper::ThreadTabHelper() {
	quotesRegExp.setPattern("class=\\\\\"quote\\\\\"");
	quotelinksRegExp.setPattern("class=\\\\\"quotelink\\\\\"");
}

void ThreadTabHelper::startUp(Chan *api, QString &board, QString &thread, QWidget *parent, bool isFromSession = false)
{
	this->parent = parent;
	this->thread = thread;
	this->board = board;
	this->isFromSession = isFromSession;
	this->api = api;
	//TODO check type
	this->threadUrl = api->threadURL(board,thread);
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	this->expandAll = settings.value("autoExpand",false).toBool();
	QDir().mkpath(board+"/"+thread+"/thumbs");
	qDebug() << threadUrl;

	//update timer
	updateTimer = new QTimer(parent);
	updateTimer->setInterval(60000);
	updateTimer->start();
	if(settings.value("autoUpdate").toBool()) {
		connectionUpdate = connect(updateTimer,SIGNAL(timeout()),this,SLOT(getPosts()),UniqueDirect);
	}

	filterMe.filterMatchedPerTab(board,"thread");

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
	getPosts();
}

ThreadTabHelper::~ThreadTabHelper() {
	abort = true;
	updateTimer->stop();
	disconnect(connectionUpdate);
	if(gettingReply) {
		reply->abort();
		disconnect(reply);
		reply->deleteLater();
	}
}

void ThreadTabHelper::setAutoUpdate(bool update) {
	if(update) {
		connectionUpdate = connect(updateTimer, SIGNAL(timeout()),this,SLOT(getPosts()),UniqueDirect);
	}
}

void ThreadTabHelper::getPosts() {
	disconnect(connectionPost);
	if(reply) reply->abort();
	qDebug() << "getting posts for" << threadUrl;
	reply = nc.jsonManager->get(request);
	gettingReply = true;
	connectionPost = connect(reply,SIGNAL(finished()),this,SLOT(getPostsFinished()),UniqueDirect);
}

void ThreadTabHelper::writeJson(QString &board, QString &thread, QByteArray &rep) {
	QFile jsonFile(board+"/"+thread+"/"+thread+".json");
	jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	jsonFile.write(rep);
	jsonFile.close();
}

void ThreadTabHelper::getExtraFlags(){
	if(abort || !(QString("int|pol|sp|bant").contains(board))) return;
	qDebug().noquote().nospace() << "loading extra flags for " << board << '/' << thread;
	QStringList postNums;
	foreach(QString no, tfMap.keys()){
		if(abort) return;
		if(gottenFlags.contains(no)) continue;
		postNums.append(no);
	}
	QString data = "board=" % board % "&post_nrs=" % postNums.join("%2C");
	requestFlags.setUrl(QUrl("https://flagtism.drunkensailor.org/int/get_flags_api2.php"));
	requestFlags.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	replyFlags = nc.fileManager->post(requestFlags,data.toUtf8());
	connect(replyFlags,SIGNAL(finished()),this,SLOT(loadExtraFlags()),UniqueDirect);
}

void ThreadTabHelper::loadExtraFlags(){
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
		gottenFlags.insert(post_nr);
		QString region = ef.value("region").toString();
		if(!abort && tfMap.contains(post_nr)){
			ThreadForm *cur = tfMap.value(post_nr);
			cur->setRegion(region);
		}
	}
}

void ThreadTabHelper::getPostsFinished() {
	if(abort || !reply) return;
	gettingReply = false;
	if(reply->error()) {
		qDebug().noquote() << "loading post error:" << reply->errorString();
		if(reply->error() == QNetworkReply::ContentNotFoundError) {
			qDebug() << "Stopping timer for" << threadUrl;
			updateTimer->stop();
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
	QJsonArray posts = QJsonDocument::fromJson(postData).object().value("posts").toArray();
	int length = posts.size();
	qDebug().noquote() << QString("length of ").append(threadUrl).append(" is ").append(QString::number(length));
	//check OP if archived or closed
	QJsonObject p;
	if(length) {
		p = posts.at(0).toObject();
		Post OP(p,board);
		if(OP.archived) {
			qDebug().nospace() << "Stopping timer for " << threadUrl <<". Reason: Archived";
			if(updateTimer->isActive()) updateTimer->stop();
			emit threadStatus("archived",OP.archived_on);
		}
		else if(OP.closed) {
			qDebug().nospace() << "Stopping timer for " << threadUrl <<". Reason: Closed";
			if(updateTimer->isActive()) updateTimer->stop();
			emit threadStatus("closed");
		}
	}
	else return;
	//write to file
	if(writeIt) QtConcurrent::run(&ThreadTabHelper::writeJson, board, thread, postData);
	//load new posts
	int i = tfMap.size();
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	bool loadFile = settings.value("autoExpand",false).toBool() || this->expandAll;
	while(i<length) {
		p = posts.at(i).toObject();
		ThreadForm *tf;
		if(!abort) tf = new ThreadForm(api,board,thread,PostType::Reply,true,loadFile,parent);
		else break;
		tf->load(p);
		//TODO check post number without making the thread form?
		if(tfMap.contains(tf->post.no)){
			tf->deleteLater();
			i++;
			continue;
		}
		tfMap.insert(tf->post.no,tf);
		if(you.hasYou(board,tf->post.no)){
			tf->post.isYou = true;
		}
		//Use another thread?
		//if(filter.filterMatched(tf->matchThis()) || filter.filterMatched2(&(tf->post))){
		if(filterMe.filterMatched2(&(tf->post))){
			tf->hidden=true;
		}
		emit newTF(tf);
		//Update windowTitle with OP info
		if(i==0) {
			if(tf->post.sub.length()){
				emit windowTitle("/"+board+"/"+thread + " - " + tf->post.sub);
				emit tabTitle(tf->post.sub);
			}
			else if(tf->post.com.length()) {
				QString temp = tf->post.com;
				//TODO clean this
				temp = Filter::htmlParse(temp)
						.replace("\n"," ")
						.remove(QRegExp("<[^>]*>"));
				emit windowTitle("/"+board+"/"+thread + " - " + temp);
				if(!isFromSession) emit tabTitle(temp);
			}
		}
		QPointer<ThreadForm> replyTo;
		foreach (const QString &orig, tf->quotelinks) {
			QMap<QString,ThreadForm*>::iterator i = tfMap.find(orig);
			if(i != tfMap.end()) {
				replyTo = i.value();
				if(replyTo) {
					replyTo->replies.insert(tf->post.no.toDouble(),tf->post.no);
					replyTo->addReplyLink(tf->post.no,tf->post.isYou);
				}
			}
		}
		if(tf->post.hasYou){
			ThreadForm *cloned = tf->clone(0);
			nv->addNotification(cloned);
		}
		i++;
	}
	if(settings.value("extraFlags/enable",false).toBool()) getExtraFlags();
	//emit scrollIt();
}

void ThreadTabHelper::reloadFilters(){
	filterMe.filters2 = filter.filterMatchedPerTab(this->board,"thread");
	foreach(ThreadForm* tf,tfMap){
		if(filterMe.filterMatched2(&(tf->post))){
			tf->hidden=true;
			emit removeTF(tf);
		}
		else{
			tf->hidden=false;
			emit showTF(tf);
		}
	}
}

void ThreadTabHelper::loadAllImages() {
	expandAll = !expandAll;
	qDebug() << "settings expandAll for" << threadUrl << "to" << expandAll;
	if(expandAll) {
		QMapIterator<QString,ThreadForm*> mapI(tfMap);
		while (mapI.hasNext()) {
			mapI.next();
			mapI.value()->getFile();
		}
	}
}
