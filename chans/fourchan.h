#ifndef FOURCHAN_H
#define FOURCHAN_H

#include "chan.h"
#include <QString>

//TODO check if i use inline/use final/etc.

class FourChan : public Chan
{
public:
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
		"6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc",
		"https://www.google.com/recaptcha/api",
		"en",
		"https://www.google.com/recaptcha/api/challenge?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc&lang=en",
		"https://www.google.com/recaptcha/api/image?c="
	};
	inline CaptchaLinks captchaLinks(){return captchaInfo;}
};

#endif // FOURCHAN_H
