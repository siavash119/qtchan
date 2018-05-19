#ifndef FOURCHAN_H
#define FOURCHAN_H

#include "chan.h"
//TODO check if i use inline/use final/etc.

class FourChan : public Chan
{
public:
	inline FourChan(){
		myName = "4chan";
		myRegUrl.setPattern("^(4chan/|(?:(?:https?://)?boards\\.4chan\\.org/))?(?<url>.*)$");
		myRegUrl.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myRegToThread.setPattern("^/?(?<board>\\w+)(?:/thread)?/(?<thread>\\d+)(?:#p\\d+)?$");
		myRegToThread.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myRegToCatalog.setPattern("^/?(?<board>\\w+)/(?:catalog#s=)?(?<search>.+)?$");
		myRegToCatalog.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		myApiBase = "https://i.4cdn.org/";
		myCaptchaUrl = "https://www.google.com/recaptcha/api";
		captchaInfo = {
			"https://www.google.com/recaptcha/api",
			"6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc",
			"en",
			"https://www.google.com/recaptcha/api/fallback?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc",
			"https://www.google.com/recaptcha/api2/payload?c=",
			"https://boards.4chan.org/"
		};
	}
	inline QString name(){return myName;}
	inline QString thumbURL(QString &board,QString name, QString ext){(void)ext; return QString(board % '/' % name % "s.jpg");}
	inline QString imageURL(QString &board,QString name, QString ext){return QString(board % '/' % name % ext);}
	inline bool usesCaptcha(){return true;}
	//4chan doesn't need API name in front
	inline QRegularExpression regURL(){return myRegUrl;}
	inline QRegularExpression regToThread(){return myRegToThread;}
	inline QRegularExpression regToCatalog(){return myRegToCatalog;}
	inline QString apiBase(){return myApiBase;}
	inline QString boardURL(QString &board){return QString("https://a.4cdn.org/" % board % "/1.json");}
	inline QString catalogURL(QString &board){return QString("https://a.4cdn.org/" % board % "/catalog.json");}
	inline QString threadURL(QString &board, QString &thread){return QString("https://a.4cdn.org/" % board % "/thread/" % thread % ".json");}
	inline QString postURL(QString &board){return QString("https://sys.4chan.org/" % board % "/post");}
	//TODO lang from preferences
	inline QString captchaURL(){return myCaptchaUrl;}

	inline CaptchaLinks captchaLinks(){return captchaInfo;}
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
private:
	QString myName;
	QRegularExpression myRegUrl;
	QRegularExpression myRegToThread;
	QRegularExpression myRegToCatalog;
	QString myApiBase;
	QString myCaptchaUrl;
	CaptchaLinks captchaInfo;
};

#endif // FOURCHAN_H
