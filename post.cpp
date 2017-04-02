#include "post.h"
#include <QDebug>
#include <QSettings>

Post::Post(QJsonObject &p, QString &board)
{
    this->board = board;
    no = QString::number(p.value("no").toInt());
    resto = p.value("resto").toInt();
    if(!resto){
        sticky = p.value("sticky").toBool();
        closed = p.value("closed").toBool();
        archived = p.value("archived").toBool();
        archived_on = p.value("archived_on").toInt();
        bumplimit = p.value("bumplimit").toBool();
        imagelimit = p.value("imagelimit").toBool();
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
    com = htmlParse(p.value("com").toString());
    double temp = p.value("tim").toDouble();
    if(temp){
        tim = QString::number(temp,'d',0);
        filename = p.value("filename").toString();
        ext = p.value("ext").toString();
        fsize = p.value("fsize").toDouble();
        md5 = p.value("md5").toString();
        w = p.value("w").toInt();
        h = p.value("h").toInt();
        tn_w = p.value("tn_w").toInt();
        tn_h = p.value("tn_h").toInt();
        filedeleted = p.value("filedeleted").toBool();
        spoiler = p.value("spoiler").toBool();
        custom_spoiler = p.value("custom_spoiler").toInt();
    }
}

Post::~Post(){
    delete this;
}

QString Post::htmlParse(QString string){
    //QSettings settings;
    //QColor color = settings.value("quote_color",);
    QString colorString = "class=\"quote\" style=\"color:#8ba446\"";
    QString quoteString = "class=\"quote\" style=\"color:#897399\"";
    QRegExp quotes("class=\"quote\"");
    QRegExp quotelinks("class=\"quotelink\"");
    string.replace(quotes,colorString);
    string.replace(quotelinks,quoteString);
    return string;
}
