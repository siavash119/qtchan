#ifndef FOURCHAN_H
#define FOURCHAN_H

#include "chan.h"

//TODO check if i use inline/use final/etc.

class FourChan : public Chan
{
public:
	inline QString name(){return "4chan";}
	inline QString thumbURL(){return "";}
	inline QString imageURL(){return "";}
	inline bool usesCaptcha(){return true;}
	inline QString regURL(){return QString("^(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?.*$");}
	inline QString regToThread(){return QString("^(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?(\\w+)(?:\\/thread)?\\/(\\d+)(?:#p\\d+)?$");}
	inline QString regToCatalog(){return QString("^(?:(?:https?:\\/\\/)?boards.4chan.org)?\\/?(\\w+)\\/(?:catalog#s=)?(.+)?$");}
	inline QString apiBase(){return QString("https://i.4cdn.org/");}
	inline QString boardURL(QString &board){return QString("https://a.4cdn.org/" % board % "/1.json");}
	inline QString catalogURL(QString &board){return QString("https://a.4cdn.org/" % board % "/catalog.json");}
	inline QString threadURL(QString &board, QString &thread){return QString("https://a.4cdn.org/" % board % "/thread/" % thread % ".json");}
	inline QString postURL(QString &board){return QString("https://sys.4chan.org/" % board % "/post");}
	//TODO lang from preferences
	inline QString captchaURL(){return QString("https://www.google.com/recaptcha/api");}
	CaptchaLinks captchaInfo = {
		"https://www.google.com/recaptcha/api",
		"6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc",
		"en",
		"https://www.google.com/recaptcha/api/fallback?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc",
		"https://www.google.com/recaptcha/api2/payload?c=",
		"https://boards.4chan.org/"
	};
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
};

#endif // FOURCHAN_H
