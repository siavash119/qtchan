#include "filter.h"
#include "post.h"
#include "you.h"
#include <QStandardPaths>
#include <QDebug>

Filter::Filter()
{
	loadFilterFile();
	loadFilterFile2();
}

/*void Filter::htmlParse(QString search) {

}*/

QSet<QString> Filter::findQuotes(QString post)
{
	QRegularExpression quotelink;
	quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
	quotelinkMatches = quotelink.globalMatch(post);
	QSet<QString> quotes;
	while (quotelinkMatches.hasNext()) {
		quotelinkMatch = quotelinkMatches.next();
		quotes.insert(QString(quotelinkMatch.captured(1)));
	}
	return quotes;
}

//TODO allow change filter file location setting
//TODO listen for file changes and reload filter
void Filter::loadFilterFile(){
	QString filterFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/" + "filters.conf";
	QFile inputFile(filterFile);
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			//replace \ with \\ for c++ regexp
			if(line.isEmpty() || line.at(0)=='#') continue;
			line = line.replace("\\\\","\\\\\\\\");
			filters.insert(QRegularExpression(line));
		}
		inputFile.close();
	}
}

void Filter::loadFilterFile2(){
	filters2.clear();
	QString filterFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/" + "filters2.conf";
	QFile inputFile(filterFile);
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QTextStream in(&inputFile);
		while (!in.atEnd())
		{
			QString line = in.readLine();
			if(!line.isEmpty() && line.at(0)=='!'){
				QString key = line.mid(1);
				QSet<QRegularExpression> set = filters2.value(key);
				QString exp = in.readLine();
				while(!exp.isEmpty() && exp.at(0) != '!' && !in.atEnd()){
					if(exp.isEmpty() || exp.at(0)=='#') continue;
					exp = exp.replace("\\\\","\\\\\\\\");
					set.insert(QRegularExpression(exp,QRegularExpression::CaseInsensitiveOption));
					exp = in.readLine();
				}
				filters2.insert(key,set);
			}
		}
		inputFile.close();
	}
	qDebug() << "########" << endl << "FILTERS" << endl << filters2 << endl << "#########";
}

void Filter::addFilter2(QString key, QString newFilter){
	QSet<QRegularExpression> set = filters2.value(key);
	set.insert(QRegularExpression(newFilter,QRegularExpression::CaseInsensitiveOption));
	filters2.insert(key,set);
	writeFilterFile2();
}

void Filter::writeFilterFile2(){
	QString filterFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/" + "filters2.conf";
	QFile file(filterFile);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
		QTextStream out(&file);
		foreach(QString key, filters2.keys()){
			out << '!' << key;
			foreach(QRegularExpression exp, filters2.value(key)){
				out << endl << exp.pattern();
			}
			out << endl << '!' << endl << endl;
		}
	}
}

void Filter::addFilter(QString &newFilter){
	filters.insert(QRegularExpression(newFilter));
	QString filterFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/" + "filters.conf";
	QFile file(filterFile);
	if(file.open(QIODevice::WriteOnly | QIODevice::Append)){
		newFilter.prepend("\n");
		file.write(newFilter.toUtf8());
		file.close();
	}
}

bool Filter::filterMatched2(Post *p){
	QHashIterator<QString, QSet<QRegularExpression>> i(filters2);
	while (i.hasNext()) {
		i.next();
		QString key = i.key();
		QSet<QRegularExpression> set = i.value();
		QString *temp = p->get(key);
		foreach(QRegularExpression exp, set){
			if(temp != Q_NULLPTR && !temp->isEmpty() && exp.match(*temp).hasMatch()){
				return true;
			}
		}
	}
	return false;
}

bool Filter::filterMatched(QString post){
	QSetIterator<QRegularExpression> i(filters);
	while (i.hasNext()){
		QRegularExpression temp = i.next();
		if(temp.match(post).hasMatch()){
			qDebug() << temp.pattern() << "matched with" << endl << post;
			return true;
		}
	}
	return false;
}

/*
QSet<QString> Filter::crossthread(QString search) {
	QRegularExpression quotelink;
	quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
	quotelinkMatches = quotelink.globalMatch(post);
	QSet<QString> quotes;
	while (quotelinkMatches.hasNext()) {
		quotelinkMatch = quotelinkMatches.next();
		quotes.insert(QString(quotelinkMatch.captured(1)));
	}
	return quotes;
}*/

QRegularExpression Filter::quoteRegExp("class=\"quote\"");
QRegularExpression Filter::quotelinkRegExp("class=\"quotelink\"");
QString Filter::colorString("class=\"quote\" style=\"color:#8ba446\"");
QString Filter::quoteString("class=\"quote\" style=\"color:#897399\"");
/*{
	QSettings settings;
	QString colorValue(settings.value("colorString"));
	return colorValue.isEmpty() ? "class=\"quote\" style=\"color:#8ba446\"" : colorValue;
}*/


QString Filter::replaceQuoteStrings(QString &string){
	//QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	//QColor color = settings.value("quote_color",);
	string.replace(quoteRegExp,colorString);
	string.replace(quotelinkRegExp,quoteString);
	return string;
}

QString Filter::replaceYouStrings(QRegularExpressionMatchIterator i, QString &string){
	QList<QRegularExpressionMatch> matches;
	while (i.hasNext()) {
		matches.append(i.next());
	}
	for(int i=matches.size()-1; i>=0; i--){
		QRegularExpressionMatch match = matches.at(i);
		if(match.capturedEnd()){
			string.insert(match.capturedEnd()," (You)");
		}
	}
	return string;
}

QString Filter::htmlParse(QString &html){
	return html.replace("<br>","\n").replace("&amp;","&")
		.replace("&gt;",">").replace("&lt;","<")
		.replace("&quot;","\"").replace("&#039;","'")
		.replace("<wb>","\n").replace("<wbr>","\n");
}

QString Filter::titleParse(QString &title){
	QRegularExpression htmlTag;
	htmlTag.setPattern("</?span.*?>");
	return title.replace(htmlTag,"").replace("<br>"," ").replace("&amp;","&")
		.replace("&gt;",">").replace("&lt;","<")
		.replace("&quot;","\"").replace("&#039;","'")
		.replace("<wb>","").replace("<wbr>","");
}

QString Filter::toStrippedHtml(QString &text){
	//QRegularExpression imgTag("<img.*?>");
	QRegularExpression htmlTag("<[^>]*>");
	return text.replace(QRegularExpression("<img.*?>")," ").replace(htmlTag,"").replace("&amp;","&")
		.replace("&gt;",">").replace("&lt;","<")
		.replace("&quot;","\"").replace("&#039;","'");
}

Filter filter;
