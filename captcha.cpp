#include "captcha.h"
#include <QDebug>

Captcha::Captcha()
{

}

Captcha::~Captcha(){

}

void Captcha::getCaptcha(){
	qDebug() << "getting captcha from " + urlChallenge;
	replyChallenge = nc.jsonManager->get(requestChallenge);
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
	qDebug() << replyString;
	int start = replyString.indexOf("{\n    challenge : ");
	if(start == -1) return;
	int end = replyString.indexOf("'",start+19);
	if(end == -1) return;
	qDebug() << start << " " << end;
	challenge = replyString.mid(start+19,end-start-19);
	qDebug() << challenge;
	getImage(challenge);
}

void Captcha::getImage(QString challenge){
	qDebug() << "getting image";
	if(challenge.isEmpty()) return;
	replyImage = nc.jsonManager->get(QNetworkRequest(QUrl(urlImageBase+challenge)));
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
