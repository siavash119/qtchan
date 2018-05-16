#ifndef TWOCHHK_H
#define TWOCHHK_H

#include "chan.h"

//TODO check if i use inline/use final/etc.

class TwoChHk : public Chan
{
public:
	inline QString thumbURL(){return "";}
	inline QString imageURL(){return "";}
	inline bool usesCaptcha(){return true;}
	inline QString regURL(){return QString("^(?:(?:https?:\\/\\/)?2ch.hk)\\/?.*$");}
	inline QString regToThread(){return QString("^2ch.hk\\/(\\w+)\\/(\\d+)$");}
	inline QString regToCatalog(){return QString("^(?:(?:https?:\\/\\/)?2ch.hk)?\\/?(\\w+)\\/(?:catalog#s=)?(.+)?$");}
	inline QString apiBase(){return QString("https://2ch.hk/");}
	inline QString boardURL(QString &board){return QString("https://2ch.hk/" % board % "/1.json");}
	inline QString catalogURL(QString &board){return QString("https://2ch.hk/" % board % "/catalog.json");}
	inline QString threadURL(QString &board, QString &thread){return QString("https://2ch.hk/" % board % "/res/" % thread % ".json");}
	//TODO
	inline QString postURL(QString &board){(void)board; return QString();}
	inline QString captchaURL(){return QString();}
	inline CaptchaLinks captchaLinks(){return CaptchaLinks{0};}
	/*inline QString postURL(QString &board){return QString("https://sys.4chan.org/" % board % "/post");}
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
	inline CaptchaLinks captchaLinks(){return captchaInfo;}*/
	inline QJsonArray postsArray(QByteArray &data, QString type){
		(void)type;
		QJsonArray temp = QJsonDocument::fromJson(data).object().value("threads")
				.toArray().at(0).toObject().value("posts").toArray();
		return temp;
	}

	inline PostKeys postKeys(){
		PostKeys keys;
		keys.com = "comment";
		keys.no = "num";
		keys.sub = "subject";
		keys.time = "timestamp";
		return keys;
	}
};

#endif // TWOCHHK_H
