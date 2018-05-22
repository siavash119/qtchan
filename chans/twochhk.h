#ifndef TWOCHHK_H
#define TWOCHHK_H

#include "chan.h"
#include "you.h"

//TODO check if i use inline/use final/etc.

class TwoChHkPost : public Post
{
public:
	inline TwoChHkPost(QJsonObject &p, QString &board, QString &thread){
		this->board = board;
		this->thread = thread;
		no = QString::number(p.value("num").toInt());
		resto = p.value("resto").toInt();
		if(resto == 0) {
			sticky = (p.value("sticky").toInt() == 1) ? true : false;
			closed = (p.value("closed").toInt() == 1) ? true : false;
			archived = (p.value("archived").toInt() == 1) ? true : false;
			archived_on = QString::number(p.value("archived_on").toDouble(),'d',0);
			bumplimit = (p.value("bumplimit").toInt() == 1) ? true : false;
			imagelimit = (p.value("imagelimit").toInt() == 1) ? true : false;
			semantic_url = p.value("semantic_url").toString();
		}
		now = p.value("now").toString();
		time = p.value("timestamp").toInt();
		QDateTime timestamp;
		timestamp.setTime_t(time);
		realNow = timestamp.toString(Qt::SystemLocaleShortDate);
		name = p.value("name").toString();
		trip = p.value("trip").toString();
		id = p.value("id").toString();
		capcode = p.value("id").toString();
		country = p.value("country").toString();
		troll_country = p.value("troll_country").toString();
		country_name = p.value("country_name").toString();
		sub = p.value("subject").toString();
		//TODO regexp on all posts or do the whole json at once?
		//com = p.value("com").toString();
		com = p.value("comment").toString();
		QRegularExpressionMatchIterator i = you.findYou("4chan",board,com);
		if(i.hasNext()) hasYou = true;
		if(you.hasYou("4chan",board,no)){
			isYou = true;
		}
		com = Filter::replaceYouStrings(i,com);
		com = Filter::replaceQuoteStrings(com);
		quotelinks = Filter::findQuotes(com);
		QJsonArray allFiles = p.value("files").toArray();
		for(int i=0; i<allFiles.size(); i++){
			QJsonObject f = allFiles.at(i).toObject();
			PostFile file;
			file.tim = f.value("name").toString();
			file.filename = f.value("name").toString();
			file.ext = f.value("ext").toString();
			file.fsize = f.value("fsize").toDouble();
			file.md5 = f.value("md5").toString();
			file.w = f.value("w").toInt();
			file.h = f.value("h").toInt();
			file.size_img = QString(file.w + "x" + file.h);
			file.tn_w = f.value("tn_w").toInt();
			file.tn_h = f.value("tn_h").toInt();
			file.filedeleted = (f.value("filedeleted").toInt() == 1) ? true : false;
			file.spoiler = (f.value("spoiler").toInt() == 1) ? true : false;
			file.custom_spoiler = (f.value("custom_spoiler").toInt() == 1) ? true : false;
			files.append(file);
		}
	}
};

class TwoChHk : public Chan
{
public:
	inline TwoChHk(){
		myName = "2ch.hk";
		myRegUrl.setPattern("^(2ch.hk/|(?:(?:https?://)?2ch.hk/))(?<url>.*)$");
		myRegUrl.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myRegToThread.setPattern("^/?(?<board>\\w+)(?:/res)?/(?<thread>\\d+)(?:#p\\d+)?(\\.html)?$");
		myRegToThread.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myRegToCatalog.setPattern("^/?(?<board>\\w+)/(?:catalog#s=)?(?<search>.+)?$");
		myRegToCatalog.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	}
	inline QString name(){return myName;}
	inline QString thumbURL(QString &board,QString &thread, QString name, QString ext){
		(void)ext;
		return QString(board % '/' % thread % '/' % name % "s.jpg");
	}
	inline QString imageURL(QString &board,QString &thread, QString name, QString ext){
		return QString(board % '/' % thread % '/' % name % ext);
	}
	inline bool usesCaptcha(){return true;}
	inline QRegularExpression regURL(){return myRegUrl;}
	inline QRegularExpression regToThread(){return myRegToThread;}
	inline QRegularExpression regToCatalog(){return myRegToCatalog;}
	inline QString apiBase(){return QString("https://2ch.hk/");}
	inline QString boardURL(QString &board){return QString("https://2ch.hk/" % board % "/1.json");}
	inline QString catalogURL(QString &board){return QString("https://2ch.hk/" % board % "/catalog.json");}
	inline QString threadURL(QString &board, QString &thread){return QString("https://2ch.hk/" % board % "/res/" % thread % ".json");}
	//TODO
	inline QString postURL(QString &board){(void)board; return QString();}
	inline PostKeys postKeys(){
		PostKeys keys;
		keys.api = "2ch.hk";
		keys.com = "comment";
		keys.no = "num";
		keys.sub = "subject";
		keys.time = "timestamp";
		return keys;
	}
	inline QJsonArray postsArray(QByteArray &data, QString type){
		(void)type;
		return QJsonDocument::fromJson(data).object().value("threads")
				.toArray().at(0).toObject().value("posts").toArray();
	}
	//TODO below functions
	inline QJsonArray threadsArray(QByteArray &data){
		return QJsonDocument::fromJson(data).object().value("threads").toArray();
	}
	inline QJsonArray catalogArray(QByteArray &data){
		return QJsonDocument::fromJson(data).array();
	}
	inline QJsonArray catalogPageArray(QJsonArray &allThreads, int index){
		return allThreads.at(index).toObject().value("threads").toArray();
	}
	inline Post post(QJsonObject p, QString &board, QString &thread){
		return TwoChHkPost(p,board,thread);
	}
private:
	QString myName;
	QRegularExpression myRegUrl;
	QRegularExpression myRegToThread;
	QRegularExpression myRegToCatalog;
	QString myApiBase;
	QString myCaptchaUrl;
	CaptchaLinks captchaInfo;
};

#endif // TWOCHHK_H
