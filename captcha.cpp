#include "captcha.h"
#include <QDebug>

Captcha::Captcha()
{
}

//TODO use the CaptchaLinks object from api instead of locally defined variables
void Captcha::startUp(Chan *api){
	CaptchaLinks links = api->captchaLinks();
	siteKey = links.siteKey;
	server = links.server;
	lang = QString(links.lang);
	urlChallenge = links.challengeURL;
	urlImageBase = links.imageBaseURL;
	requestChallenge = QNetworkRequest(QUrl(urlChallenge));
	requestChallenge.setRawHeader(QByteArray("Referer"), QByteArray(links.refererURL.toUtf8()));
}

Captcha::~Captcha(){

}

void Captcha::getCaptcha(){
	loading = true;
	replyChallenge = NULL;
	qDebug() << "getting captcha from " + urlChallenge;
	replyChallenge = nc.captchaManager->get(requestChallenge);
	QObject::connect(replyChallenge,&QNetworkReply::finished,this,&Captcha::loadCaptcha);
}

void Captcha::loadCaptcha(){
	qDebug() << "got captcha";
	if(!replyChallenge){
		emit fail();
		return;
	}
	else if(replyChallenge->error()) {
		qDebug().noquote() << "loading post error:" << replyChallenge->errorString();
		replyChallenge->deleteLater();
		return;
	}
	QByteArray rep = replyChallenge->readAll();
	replyChallenge->deleteLater();
	if(rep.isEmpty()) return;
	QString replyString(rep);
	QString matchStart = "src=\"/recaptcha/api2/payload?c=";
	int start = replyString.indexOf(matchStart);
	if(start == -1) return;
	int end = replyString.indexOf("&",start+matchStart.length());
	if(end == -1) return;
	challenge = replyString.mid(start+matchStart.length(),end-start-matchStart.length());
	start = replyString.indexOf("<strong>");
	end = replyString.indexOf("</strong>",start+8);
	challengeQuestion = replyString.mid(start+8,end-start-8);
	emit questionInfo(challengeQuestion);
	getImage(challenge);
}

void Captcha::getImage(QString challenge){
	qDebug() << "getting image";
	if(challenge.isEmpty()) return;
	QNetworkRequest imageChallenge(QUrl(urlImageBase+challenge+"&k="+siteKey));
	replyImage = nc.captchaManager->get(imageChallenge);
	connect(replyImage,&QNetworkReply::finished,this,&Captcha::loadImage);
}

void Captcha::loadImage(){
	qDebug() << "got image";
	if(!replyImage){
		emit fail();
		return;
	}
	else if(replyImage->error()) {
		qDebug().noquote() << "loading post error:" << replyImage->errorString();
		replyImage->deleteLater();
		return;
	}
	QByteArray rep = replyImage->readAll();
	replyImage->deleteLater();
	if(rep.isEmpty()) return;
	QPixmap challengeImage;
	challengeImage.loadFromData(rep);
	loaded = true;
	QTimer::singleShot(180000, [=](){loaded = false;});
	emit challengeInfo(challenge,challengeImage);
}
