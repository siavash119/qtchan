#ifndef EIGHTCHAN_H
#define EIGHTCHAN_H

#include "chan.h"
#include "netcontroller.h"
#include <QDebug>

//TODO everything

class EightChan : public Chan
{
public:
	inline EightChan(){
		nc.loadCookiesIntoAllManagers(".8ch.net","__cfduid","your __cfduid cookie value");
		nc.loadCookiesIntoAllManagers(".8ch.net","cf_clearance","your cf_clearance cookie value");
	}
	inline QString name(){return "8ch";}
	inline QString thumbURL(){return "";}
	inline QString imageURL(){return "";}
	inline QString regURL(){return QString("^(?:https?:\\/\\/)?8ch.net\\/?.*$");}
	inline QString regToThread(){return QString("^(?:(?:https?:\\/\\/)?8ch.net)\\/?(\\w+)(?:\\/res)?\\/(\\d+)(?:#p\\d+)?$");}
	inline QString regToCatalog(){return QString("^(?:(?:https?:\\/\\/)?8ch.net)\\/?(\\w+)\\/(?:catalog#s=)?(.+)?$");}
	inline QString apiBase(){return QString("https://media.8ch.net/file_store/");}
	inline QString boardURL(QString &board){return QString("https://" % board % "/0.json");}
	inline QString catalogURL(QString &board){return QString("https://" % board % "/catalog.json");}
	inline QString threadURL(QString &board, QString &thread){return QString("https://" % board % "/res/" % thread % ".json");}
	inline QString postURL(QString &board){return QString("https://sys.8ch.net/" % board % "/post");}
	inline bool usesCaptcha(){return false;}
	inline QString captchaURL(){return "";}
	inline bool requiresCookies(){return true;}
	inline void setCookies(){
		qDebug() << "setting cookies for 8ch";
		nc.loadCookiesIntoAllManagers(".8ch.net","__cfduid","your __cfduid cookie value");
		nc.loadCookiesIntoAllManagers(".8ch.net","cf_clearance","your cf_clearance cookie value");
	}
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
};

#endif // EIGHTCHAN_H
