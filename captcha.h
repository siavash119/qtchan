#ifndef CAPTCHA_H
#define CAPTCHA_H
#include "netcontroller.h"
#include "chans.h"
#include <QObject>
#include <QNetworkReply>
#include <QThread>
#include <QPixmap>
#include <QTimer>

class Captcha : public QObject
{
	Q_OBJECT
	int timeout = 120;
public:
	explicit Captcha();
	virtual ~Captcha();
	void startUp(Chan *api);
	QString challenge;
	QString challengeQuestion;
	QString response;
	QTimer timer;
	bool loaded = false;
	bool loading = false;
	void getCaptcha();
	void getImage(QString challenge);
private:
	QString siteKey;
	QString server;
	QString lang;
	QString urlChallenge;
	QString urlImageBase;
	QNetworkRequest requestChallenge;
	QNetworkRequest requestAntiCaptcha;
	QNetworkRequest requestAntiSolution;
	QNetworkReply *replyChallenge;
	QNetworkReply *replyImage;

	QString antiKey;
	QString antiType;
	QString antiMakeUrl;
	QString antiGetUrl;
	QString antiCaptchaInfo;
	QString antiGetInfo;
	QString antiTaskID;
	void antiFinish();
	void antiMake();
private slots:
	void antiMade();
	void antiFinished();
signals:
	void challengeInfo(QString &challenge, QPixmap &challengeImage);
	void questionInfo(QString &challenge);
	void timeOut();
	void success();
	void fail();
	void captchaCode(QString &code);
public slots:
	void loadCaptcha();
	void loadImage();
};

#endif // CAPTCHA_H
