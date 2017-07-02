#include <QJsonArray>
#include "threadtabhelper.h"
#include "netcontroller.h"

ThreadTabHelper::ThreadTabHelper() {

}

void ThreadTabHelper::startUp(QString &board, QString &thread, QWidget *parent)
{
	this->parent = parent;
	this->thread = thread;
	this->board = board;
	this->threadUrl = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
	QSettings settings;
	this->expandAll = settings.value("autoExpand",false).toBool();
	QDir().mkpath(board+"/"+thread+"/thumbs");
	qDebug() << threadUrl;
	request = QNetworkRequest(QUrl(threadUrl));
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
	getPosts();
	updateTimer = new QTimer();
	updateTimer->setInterval(60000);
	updateTimer->start();
	if(settings.value("autoUpdate").toBool()) {
		connectionUpdate = connect(updateTimer, &QTimer::timeout,
								   this,&ThreadTabHelper::getPosts,UniqueDirect);
	}
}

ThreadTabHelper::~ThreadTabHelper() {
	disconnect(parent);
	disconnect(this);
	abort = true;
	updateTimer->stop();
	disconnect(connectionUpdate);
	delete updateTimer;
	if(gettingReply) {
		reply->abort();
		disconnect(reply);
		reply->deleteLater();
	}
	qDeleteAll(tfMap);
}

void ThreadTabHelper::setAutoUpdate(bool update) {
	if(update) {
		connectionUpdate = connect(updateTimer, &QTimer::timeout,
								   this,&ThreadTabHelper::getPosts,UniqueDirect);
	}
}

void ThreadTabHelper::getPosts() {
	qDebug() << "getting posts for" << threadUrl;
	reply = nc.jsonManager->get(request);
	gettingReply = true;
	connectionPost = connect(reply, &QNetworkReply::finished,
							 this,&ThreadTabHelper::loadPosts, Qt::DirectConnection);
}

void ThreadTabHelper::writeJson(QString &board, QString &thread, QByteArray &rep) {
	QFile jsonFile(board+"/"+thread+"/"+thread+".json");
	jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	QDataStream out(&jsonFile);
	out << rep;
	jsonFile.close();
}

void ThreadTabHelper::loadPosts() {
	gettingReply = false;
	if(reply->error()) {
		qDebug().noquote() << "loading post error:" << reply->errorString();
		if(reply->error() == QNetworkReply::ContentNotFoundError) {
			qDebug() << "Stopping timer for" << threadUrl;
			updateTimer->stop();
			emit threadStatus("404");
		}
		reply->deleteLater();
		return;
	}
	//write to file and make json array
	QByteArray rep = reply->readAll();
	reply->deleteLater();
	QtConcurrent::run(&ThreadTabHelper::writeJson,board, thread, rep);
	QJsonArray posts = QJsonDocument::fromJson(rep).object().value("posts").toArray();
	int length = posts.size();
	qDebug().noquote() << QString("length of ").append(threadUrl).append(" is ").append(QString::number(length));
	//check OP if archived or closed
	QJsonObject p;
	if(length) {
		p = posts.at(0).toObject();
		Post OP(p,board);
		if(OP.archived) {
			qDebug().nospace() << "Stopping timer for " << threadUrl <<". Reason: Archived";
			updateTimer->stop();
			emit threadStatus("archived",OP.archived_on);
		}
		else if(OP.closed) {
			qDebug().nospace() << "Stopping timer for " << threadUrl <<". Reason: Closed";
			updateTimer->stop();
			emit threadStatus("closed");
		}
	}
	else return;
	//load new posts
	int i = tfMap.size();
	QSettings settings;
	bool loadFile = settings.value("autoExpand",false).toBool() || this->expandAll;
	while(!abort && i<length) {
		p = posts.at(i).toObject();
		ThreadForm *tf = new ThreadForm(board,thread,PostType::Reply,true,loadFile,parent);
		tf->load(p);
		tfMap.insert(tf->post.no,tf);
		emit newTF(tf);
		if(i==0) {
			if(tf->post.sub.length())emit windowTitle("/"+board+"/"+thread + " - " + tf->post.sub);
			else if(tf->post.com.length()) {
				QString temp = tf->post.com;
				emit windowTitle("/"+board+"/"+thread + " - " +
								 ThreadForm::htmlParse(temp
													   .replace(QRegExp("</?span( class=\"quote\" style=\"color:#[\\d|\\w]{6}\")?>"),""))
								 .replace("\n"," "));
			}
		}
		QPointer<ThreadForm> replyTo;
		foreach (const QString &orig, tf->quotelinks)
		{
			QMap<QString,ThreadForm*>::iterator i = tfMap.find(orig);
			if(i != tfMap.end()) {
				replyTo = i.value();
				if(replyTo) {
					replyTo->replies.insert(tf->post.no.toDouble(),tf->post.no);
					replyTo->setReplies();
				}
			}
		}
		i++;
		QCoreApplication::processEvents();
	}
	if(!abort) emit addStretch();
	//emit scrollIt();
}

void ThreadTabHelper::loadAllImages() {
	expandAll = !expandAll;
	qDebug() << "settings expandAll for" << threadUrl << "to" << expandAll;
	if(expandAll) {
		QMapIterator<QString,ThreadForm*> mapI(tfMap);
		while (mapI.hasNext()) {
			mapI.next();
			mapI.value()->loadOrig();
		}
	}
}
