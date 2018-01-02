#ifndef CHAN_H
#define CHAN_H

#include <QString>

//TODO: does everything need to be a virtual function?

struct CaptchaLinks{
	QString server;
	QString siteKey;
	QString lang;
	QString challengeURL;
	QString imageBaseURL;
};

class Chan
{
public:
	virtual inline ~Chan(){}
	virtual QString boardURL(QString &board) = 0;
	virtual QString catalogURL(QString &board) = 0;
	virtual QString threadURL(QString &board, QString &thread) = 0;
	virtual QString postURL(QString &board) = 0;
	virtual QString thumbURL() = 0;
	virtual QString imageURL() = 0;
	virtual bool usesCaptcha() = 0;
	virtual QString captchaURL() = 0;
	virtual QString regURL() = 0;
	virtual QString regToThread() = 0;
	virtual QString regToCatalog() = 0;
	virtual QString apiBase() = 0;
	virtual CaptchaLinks captchaLinks() = 0;
};

#endif // CHANS_H
