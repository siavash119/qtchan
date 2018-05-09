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
				QHash<QRegularExpression,QString> hash = filters2.value(key);
				while(!in.atEnd()){
					line = in.readLine();
					if(line.isEmpty() || line.at(0)=='#'){
						continue;
					}
					else if(line.at(0) == '!') break;
					line = line.replace("\\\\","\\\\\\\\");
					int ind = line.lastIndexOf('$');
					QRegularExpression exp;
					QString options;
					if(ind != -1){
						exp.setPattern(line.mid(0,ind));
						if(line.length() > ind+1)
							options = line.mid(ind+1);
					}
					else{
						exp.setPattern(line);
					}
					exp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
					hash.insert(exp,options);
				}
				filters2.insert(key,hash);
			}
		}
		inputFile.close();
	}
	qDebug() << "########" << endl << "FILTERS" << endl << filters2 << endl << "#########";
}

void Filter::addFilter2(QString key, QString newFilter){
	QHash<QRegularExpression,QString> hash = filters2.value(key);
	hash.insert(QRegularExpression(newFilter,QRegularExpression::CaseInsensitiveOption),QString());
	filters2.insert(key,hash);
	writeFilterFile2();
}

void Filter::writeFilterFile2(){
	QString filterFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/" + "filters2.conf";
	QFile file(filterFile);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
		QTextStream out(&file);
		QHashIterator<QString, QHash<QRegularExpression,QString>> i(filters2);
		while(i.hasNext()){
			i.next();
			out << '!' << i.key();
			QHashIterator<QRegularExpression,QString> j(i.value());
			while(j.hasNext()){
				j.next();
				out << endl << j.key().pattern() << j.value();
			}
			out << endl << '!' << endl << endl;
		}
	}
}

bool Filter::filterMatched2(Post *p){
	QHashIterator<QString, QHash<QRegularExpression,QString>> i(filters2);
	while (i.hasNext()) {
		i.next();
		QString key = i.key();
		QString *temp = p->get(key);
		QHashIterator<QRegularExpression,QString> j(i.value());
		while(j.hasNext()){
			j.next();
			QRegularExpression exp = j.key();
			//QString options = j.value();
			//&& useFilter(options,p)
			if(temp != Q_NULLPTR && temp->contains(exp))
				return true;
		}
	}
	return false;
}

bool Filter::useFilter(QString &options, Post *p){
	if(options.isEmpty()) return true;
	QStringList optionsList = options.split(';');
	QVector<bool> useIt(optionsList.size(),false);
	int i = 0;
	foreach(QString option,optionsList){
		if(option.isEmpty()) return true;
		QStringList kvPairs = option.split(':');
		if(kvPairs.size() != 2){
			useIt.data()[i++] = true;
			continue;
		}
		QString key = kvPairs.at(0);
		QString values = kvPairs.at(1);
		QStringList valuesList = values.split(',');
		foreach(QString value, valuesList){
			if(key == "boards"){
				if(value == p->board){
					useIt.data()[i] = true;
					break;
				}
			}
			else if(key == "op" && p->resto == 0){
				if(value == "only" || value == "yes"){
					useIt.data()[i] = true;
					break;
				}
				else{
					return false;
				}
			}
			else if(key == "exclude"){
				if(value == p->board) return false;
			}
		}
		i++;
	}
	foreach(bool temp, useIt){
		if(!temp) return false;
	}
	return true;
}

QHash<QString,QHash<QRegularExpression,QString>> Filter::filterMatchedPerTab(QString board, QString tabType){
	QHashIterator<QString, QHash<QRegularExpression,QString>> i(filters2);
	while (i.hasNext()) {
		i.next();
		QHash<QRegularExpression,QString> temp = i.value();
		QHashIterator<QRegularExpression,QString> j(temp);
		while(j.hasNext()){
			j.next();
			QString options = j.value();
			//TODO build useFilters per tab instead of checking per post
			if(!useFilterPerTab(options,board,tabType)) {
				temp.remove(j.key());
			}
		}
		filters2.insert(i.key(),temp);
	}
	return filters2;
}

bool Filter::useFilterPerTab(QString &options, QString board, QString tabType){
	if(options.isEmpty()) return true;
	QStringList optionsList = options.split(';');
	QVector<bool> useIt(optionsList.size(),false);
	int i = 0;
	foreach(QString option,optionsList){
		if(option.isEmpty()) return true;
		QStringList kvPairs = option.split(':');
		if(kvPairs.size() != 2){
			useIt.data()[i++] = true;
			continue;
		}
		QString key = kvPairs.at(0);
		QString values = kvPairs.at(1);
		QStringList valuesList = values.split(',');
		foreach(QString value, valuesList){
			if(key == "boards"){
				if(value == board){
					useIt.data()[i] = true;
					break;
				}
			}
			else if(key == "op"){
				if(tabType == "board" && (value == "only" || value == "yes")){
					useIt.data()[i] = true;
					break;
				}
				else if(tabType == "thread" && value == "no"){
					useIt.data()[i] = true;
					break;
				}
				else return false;
			}
			else if(key == "exclude"){
				if(value == board) return false;
			}
		}
		i++;
	}
	foreach(bool temp, useIt){
		if(!temp) return false;
	}
	return true;
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
