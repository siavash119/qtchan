#include "post.h"
#include "you.h"
#include "filter.h"
#include <QDateTime>

Post::Post() {
}

Post::Post(QJsonObject &p, QString &board)
{
	load(p,board);
}

//toBool doesn't work for QJsonValue(double,1)?
//Should I just use a qHash?
void Post::load(QJsonObject &p, QString &board)
{
	this->board = board;
	no = QString::number(p.value("no").toInt());
	resto = p.value("resto").toInt();
	if(resto == 0) {
		sticky = (p.value("sticky").toInt() == 1) ? true : false;
		closed = (p.value("closed").toInt() == 1) ? true : false;
		archived = (p.value("archived").toInt() == 1) ? true : false;
		archived_on = QString::number(p.value("archived_on").toDouble(),'d',0);
		bumplimit = (p.value("bumplimit").toInt() == 1) ? true : false;
		imagelimit = (p.value("imagelimit").toInt() == 1) ? true : false;
		semantic_url = p.value("semantic_url").toString();
	}
	now = p.value("now").toString();
	time = p.value("time").toInt();
	QDateTime timestamp;
	timestamp.setTime_t(time);
	realNow = timestamp.toString(Qt::SystemLocaleShortDate);
	name = p.value("name").toString();
	trip = p.value("trip").toString();
	id = p.value("id").toString();
	capcode = p.value("id").toString();
	country = p.value("country").toString();
	troll_country = p.value("troll_country").toString();
	country_name = p.value("country_name").toString();
	sub = p.value("sub").toString();
	//TODO regexp on all posts or do the whole json at once?
	//com = p.value("com").toString();
	com = p.value("com").toString();
	QRegularExpressionMatchIterator i = you.findYou(board,com);
	if(i.hasNext()) hasYou = true;
	com = Filter::replaceYouStrings(i,com);
	com = Filter::replaceQuoteStrings(com);
	double temp = p.value("tim").toDouble();
	if(temp != 0.0) {
		tim = QString::number(temp,'d',0);
		filename = p.value("filename").toString();
		ext = p.value("ext").toString();
		fsize = p.value("fsize").toDouble();
		md5 = p.value("md5").toString();
		w = p.value("w").toInt();
		h = p.value("h").toInt();
		size_img = QString(w + "x" + h);
		tn_w = p.value("tn_w").toInt();
		tn_h = p.value("tn_h").toInt();
		filedeleted = (p.value("filedeleted").toInt() == 1) ? true : false;
		spoiler = (p.value("spoiler").toInt() == 1) ? true : false;
		custom_spoiler = (p.value("custom_spoiler").toInt() == 1) ? true : false;
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
