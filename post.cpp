#include "post.h"

Post::Post() {
}

Post::Post(QJsonObject &p, QString &board)
{
	load(p,board);
}

//toBool doesn't work for QJsonValue(double,1)?
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
	name = p.value("name").toString();
	id = p.value("id").toString();
	capcode = p.value("id").toString();
	country = p.value("country").toString();
	country_name = p.value("country_name").toString();
	sub = p.value("sub").toString();
	com = quoteColor(p.value("com").toString());
	double temp = p.value("tim").toDouble();
	if(temp != 0.0) {
		tim = QString::number(temp,'d',0);
		filename = p.value("filename").toString();
		ext = p.value("ext").toString();
		fsize = p.value("fsize").toDouble();
		md5 = p.value("md5").toString();
		w = p.value("w").toInt();
		h = p.value("h").toInt();
		tn_w = p.value("tn_w").toInt();
		tn_h = p.value("tn_h").toInt();
		filedeleted = (p.value("filedeleted").toInt() == 1) ? true : false;
		spoiler = (p.value("spoiler").toInt() == 1) ? true : false;
		custom_spoiler = (p.value("custom_spoiler").toInt() == 1) ? true : false;
	}
}

Post::~Post() {
}

QString Post::quoteColor(QString string)
{
	//QSettings settings;
	//QColor color = settings.value("quote_color",);
	QRegExp quotes("class=\"quote\"");
	QRegExp quotelinks("class=\"quotelink\"");
	string.replace(quotes,colorString);
	string.replace(quotelinks,quoteString);
	return string;
}
