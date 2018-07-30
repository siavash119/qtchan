#ifndef EIGHTCHAN_H
#define EIGHTCHAN_H

#include "chan.h"
#include "filter.h"
#include "you.h"
#include "netcontroller.h"
#include <QDebug>

//TODO everything

class EightChanPost : public Post
{
public:
	inline EightChanPost(QJsonObject &p, QString &board, QString &thread){
		this->board = board;
		this->thread = thread;
		no = QString::number(p.value("no").toInt());
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
		time = p.value("time").toInt();
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
		sub = p.value("sub").toString();
		//TODO regexp on all posts or do the whole json at once?
		//com = p.value("com").toString();
		com = p.value("com").toString();
		QRegularExpressionMatchIterator i = you.findYou("8ch",board,com);
		if(i.hasNext()) hasYou = true;
		if(you.hasYou("8ch",board,no)){
			isYou = true;
		}
		com = Filter::replaceYouStrings(i,com);
		com = Filter::replaceQuoteStrings(com);
		quotelinks = Filter::findQuotes(com);
		QJsonArray filesArray;
		if(!p.value("extra_files").isNull()){
			filesArray = p.value("extra_files").toArray();
		}
		int numFiles = filesArray.size();
		for(int i = -1; i<numFiles; i++){
			QJsonObject fileObject;
			if(i == -1) fileObject = p;
			else fileObject = filesArray.at(i).toObject();
			PostFile file;
			file.tim = fileObject.value("tim").toString();
			if(file.tim.isEmpty()) continue;
			file.filename = fileObject.value("filename").toString();
			file.ext = fileObject.value("ext").toString();
			file.fsize = fileObject.value("fsize").toDouble();
			file.md5 = fileObject.value("md5").toString();
			file.w = fileObject.value("w").toInt();
			file.h = fileObject.value("h").toInt();
			file.size_img =QString::number(file.w) % "x" % QString::number(file.h);
			file.tn_w = fileObject.value("tn_w").toInt();
			file.tn_h = fileObject.value("tn_h").toInt();
			file.filedeleted = (fileObject.value("filedeleted").toInt() == 1) ? true : false;
			file.spoiler = (fileObject.value("spoiler").toInt() == 1) ? true : false;
			file.custom_spoiler = (fileObject.value("custom_spoiler").toInt() == 1) ? true : false;
			file.tnUrlPath = "/thumb/" % file.tim % file.ext;
			file.fileUrlPath = '/' % file.tim % file.ext;
			QString indString = numFiles > 1 ? QString::number(i).append("-") : "";
			file.tnPath = "thumbs/" % no % "-" % indString % file.filename % ".jpg";
			file.filePath = no % "-" % indString % file.filename % file.ext;
			file.infoString = file.filename % file.ext
					% " (" % QString("%1").arg(file.w)
					% "x" % QString("%1").arg(file.h)
					% ", " % QString("%1").arg(file.fsize/1024,0,'f',0)
					% " KB)";
			files.append(file);
		}
	}
};


class EightChan : public Chan
{
public:
	inline EightChan(){
		qDebug() << "loading eightchan api";
		nc.loadCookiesIntoAllManagers(".8ch.net","__cfduid","your __cfduid value");
		nc.loadCookiesIntoAllManagers(".8ch.net","cf_clearance","your cf_clearance value");
		myRegUrl.setPattern("^(8ch.net/|(?:(?:https?://)?8ch.net/))(?<url>[^.]*)(\\.html)?$");
		myRegUrl.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myRegToThread.setPattern("^/?(?<board>\\w+)(?:/res)?/(?<thread>\\d+)(?:#p\\d+)?$");
		myRegToThread.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myRegToCatalog.setPattern("^/?(?<board>\\w+)/(?:catalog#s=)?(?<search>.+)?$");
		myRegToCatalog.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myApiBase = "https://media.8ch.net/file_store";
		myCaptchaUrl = "https://www.google.com/recaptcha/api";
		captchaInfo = {
			"https://www.google.com/recaptcha/api",
			"6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc",
			"en",
			"https://www.google.com/recaptcha/api/fallback?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc",
			"https://www.google.com/recaptcha/api2/payload?c=",
			"https://boards.4chan.org/"
		};
		quoteExp.setPattern("class=\\\\\"body-line ltr quote\\\\\"");
		quoteLinkExp.setPattern("<a onclick=\\\\\"highlightReply\\('\\d+', event\\);\\\\\" href=\\\\\"[^#]+#");
	}
	inline QString name(){return "8ch";}
	inline QString thumbURL(QString &board,QString &thread, QString name, QString ext){
		(void)thread; (void)ext;
		return QString(board % '/' % name % "s.jpg");
	}
	inline QString imageURL(QString &board,QString &thread, QString name, QString ext){
		(void)thread;
		return QString(board % '/' % name % ext);
	}
	inline QRegularExpression regURL(){return myRegUrl;}
	inline QRegularExpression regToThread(){return myRegToThread;}
	inline QRegularExpression regToCatalog(){return myRegToCatalog;}
	inline QString apiBase(){return myApiBase;}
	inline QString boardURL(QString &board){return QString("https://8ch.net/" % board % "/0.json");}
	inline QString catalogURL(QString &board){return QString("https://8ch.net/" % board % "/catalog.json");}
	inline QString threadURL(QString &board, QString &thread){return QString("https://8ch.net/" % board % "/res/" % thread % ".json");}
	inline QString postURL(QString &board){return QString("https://sys.8ch.net/" % board % "/post");}
	inline bool usesCaptcha(){return false;}
	inline QString captchaURL(){return "";}
	inline bool requiresCookies(){return true;}
	inline bool requiresUserAgent(){return true;}
	inline PostKeys postKeys(){
		PostKeys keys;
		keys.api = "8ch";
		return keys;
	}
	inline QJsonArray postsArray(QByteArray &data, QString type){
		(void)type;
		return QJsonDocument::fromJson(data).object().value("posts").toArray();
	}
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
		return EightChanPost(p,board,thread);
	}
	inline void replacements(QByteArray &data){
		QString dataString(data);
		dataString.replace(quoteExp,"class=\\\"quote\\\" style=\\\"color:#8ba446\\\"");
		dataString.replace(quoteLinkExp,"<a class=\\\"quote\\\" style=\\\"color:#897399\\\" href=\\\"#p");
		data = dataString.toUtf8();
	}

private:
	QString myName;
	QRegularExpression myRegUrl;
	QRegularExpression myRegToThread;
	QRegularExpression myRegToCatalog;
	QString myApiBase;
	QString myCaptchaUrl;
	CaptchaLinks captchaInfo;
	QRegularExpression quoteExp;
	QRegularExpression quoteLinkExp;
};

#endif // EIGHTCHAN_H
