#include "post.h"
#include "you.h"
#include "filter.h"
#include <QDateTime>

Post::Post() {
}

Post::Post(QJsonObject p, PostKeys &keys, QString &board)
{
	load(p,keys,board);
}

//toBool doesn't work for QJsonValue(double,1)?
//Should I just use a qHash?
void Post::load(QJsonObject &p, PostKeys &keys, QString &board)
{
	this->board = board;
	no = QString::number(p.value(keys.no).toInt());
	resto = p.value(keys.resto).toInt();
	if(resto == 0) {
		sticky = (p.value(keys.sticky).toInt() == 1) ? true : false;
		closed = (p.value(keys.closed).toInt() == 1) ? true : false;
		archived = (p.value(keys.archived).toInt() == 1) ? true : false;
		archived_on = QString::number(p.value(keys.archived_on).toDouble(),'d',0);
		bumplimit = (p.value(keys.bumplimit).toInt() == 1) ? true : false;
		imagelimit = (p.value(keys.imagelimit).toInt() == 1) ? true : false;
		semantic_url = p.value(keys.semantic_url).toString();
	}
	now = p.value(keys.now).toString();
	time = p.value(keys.time).toInt();
	QDateTime timestamp;
	timestamp.setTime_t(time);
	realNow = timestamp.toString(Qt::SystemLocaleShortDate);
	name = p.value(keys.name).toString();
	trip = p.value(keys.trip).toString();
	id = p.value(keys.id).toString();
	capcode = p.value(keys.id).toString();
	country = p.value(keys.country).toString();
	troll_country = p.value(keys.troll_country).toString();
	country_name = p.value(keys.country_name).toString();
	sub = p.value(keys.sub).toString();
	//TODO regexp on all posts or do the whole json at once?
	//com = p.value("com").toString();
	com = p.value(keys.com).toString();
	QRegularExpressionMatchIterator i = you.findYou(keys.api,board,com);
	if(i.hasNext()) hasYou = true;
	if(you.hasYou(keys.api,board,no)){
		isYou = true;
	}
	com = Filter::replaceYouStrings(i,com);
	com = Filter::replaceQuoteStrings(com);
	quotelinks = Filter::findQuotes(com);
	double temp = p.value(keys.tim).toDouble();
	if(temp != 0.0) {
		tim = QString::number(temp,'d',0);
		filename = p.value(keys.filename).toString();
		ext = p.value(keys.ext).toString();
		fsize = p.value(keys.fsize).toDouble();
		md5 = p.value(keys.md5).toString();
		w = p.value(keys.w).toInt();
		h = p.value(keys.h).toInt();
		size_img = QString(w + "x" + h);
		tn_w = p.value(keys.tn_w).toInt();
		tn_h = p.value(keys.tn_h).toInt();
		filedeleted = (p.value(keys.filedeleted).toInt() == 1) ? true : false;
		spoiler = (p.value(keys.spoiler).toInt() == 1) ? true : false;
		custom_spoiler = (p.value(keys.custom_spoiler).toInt() == 1) ? true : false;
	}
}

QString* Post::get(QString key){
	if(key == "no") return &no;
	else if(key == "name") return &name;
	else if(key == "sub") return &sub;
	else if(key == "com") return &com;
	else if(key == "trip") return &trip;
	else if(key == "md5") return &md5;
	else if(key == "size") return &size_img;
	else if(key == "filename") return &filename;
	else if(key == "country_name") return &country_name;
	else return Q_NULLPTR;
}

Post::~Post() {
}
/*
QString Post::quoteColor(QString string)
{
	//QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	//QColor color = settings.value("quote_color",);
	string.replace(quotesRegExp,colorString);
	string.replace(quotelinksRegExp,quoteString);
	QRegularExpressionMatchIterator i = you.findYou(string);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		if(match.capturedEnd()){
			string.insert(match.capturedEnd()," (You)");
		}
	}
	return string;
}*/
