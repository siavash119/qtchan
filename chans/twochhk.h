#ifndef TWOCHHK_H
#define TWOCHHK_H

#include "chan.h"

//TODO check if i use inline/use final/etc.

class TwoChHk : public Chan
{
public:
	inline TwoChHk(){
		regurl.setPattern("^(?:(?:https?:\\/\\/)?2ch.hk)\\/?.*$");
		regurl.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		regtothread.setPattern("^2ch.hk\\/(\\w+)\\/(\\d+)$");
		regtothread.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
		regtocatalog.setPattern("^(?:(?:https?:\\/\\/)?2ch.hk)?\\/?(\\w+)\\/(?:catalog#s=)?(.+)?$");
		regtocatalog.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	}
	inline QString name(){return "8ch.hk";}
	inline QString thumbURL(QString &board,QString name, QString ext){(void)ext; return QString(board % '/' % name % "s.jpg");}
	inline QString imageURL(QString &board,QString name, QString ext){return QString(board % '/' % name % ext);}
	inline bool usesCaptcha(){return true;}
	inline QRegularExpression regURL(){return regurl;}
	inline QRegularExpression regToThread(){return regtothread;}
	inline QRegularExpression regToCatalog(){return regtocatalog;}
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
private:
	QRegularExpression regurl;
	QRegularExpression regtothread;
	QRegularExpression regtocatalog;
};

#endif // TWOCHHK_H
