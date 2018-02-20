#ifndef EIGHTCHAN_H
#define EIGHTCHAN_H

#include "chan.h"
#include "netcontroller.h"
#include <QDebug>
#include <QString>

//TODO everything

class EightChan : public Chan
{
public:
	inline EightChan(){
		if(requiresCookies()) setCookies();
	}
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
	inline CaptchaLinks captchaLinks(){return CaptchaLinks{};}
	inline bool requiresCookies(){return true;}
	inline void setCookies(){
		qDebug() << "setting cookies for 8ch";
		nc.loadCookiesIntoAllManagers(".8ch.net","__cfduid","your __cfduid cookie value");
		nc.loadCookiesIntoAllManagers(".8ch.net","cf_clearance","your cf_clearance cookie value");
	}
	inline bool requiresUserAgent(){return true;}
};

#endif // EIGHTCHAN_H
