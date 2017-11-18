#include "filter.h"
#include <QStandardPaths>
#include <QDebug>

Filter::Filter()
{
    filters = loadFilterFile();
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
QSet<QRegularExpression> Filter::loadFilterFile(){
    QSet<QRegularExpression> set;
    QString filterFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/" + "filters.conf";
    QFile inputFile(filterFile);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          //replace \ with \\ for c++ regexp
          if(line.at(0)=='#') continue;
          line = line.replace("\\\\","\\\\\\\\");
          set.insert(QRegularExpression(line));
       }
    }
    return set;
}

bool Filter::filterMatched(QString post){
    QSetIterator<QRegularExpression> i(filters);
    while (i.hasNext()){
        QRegularExpression temp = i.next();
        if(temp.match(post).hasMatch()){
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
