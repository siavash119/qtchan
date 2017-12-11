#ifndef CAPTCHA_H
#define CAPTCHA_H
#include "netcontroller.h"
#include <QObject>
#include <QNetworkReply>
#include <QThread>
#include <QPixmap>
#include <QTimer>

class Captcha : public QObject
{
	Q_OBJECT
	int timeout = 120;
	QString siteKey = "6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc";
	QString server = "https://www.google.com/recaptcha/api";
	QString lang = "en";
	QString urlChallenge = server+"/challenge?k="+siteKey+"&lang="+lang;
	QString urlImageBase = server+"/image?c=";
	QNetworkRequest requestChallenge = QNetworkRequest(QUrl(urlChallenge));
public:
	explicit Captcha();
	virtual ~Captcha();
	QString challenge;
	QString response;
	QTimer timer;
	bool loaded = false;
	bool loading = false;
	void getCaptcha();
	void getImage(QString challenge);
private:
	QNetworkReply *replyChallenge;
	QNetworkReply *replyImage;
signals:
	void challengeInfo(QString &challenge, QPixmap &challengeImage);
	void timeOut();
	void success();
	void fail();
public slots:
	void loadCaptcha();
	void loadImage();
};

#endif // CAPTCHA_H
