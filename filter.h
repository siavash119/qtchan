#ifndef FILTER_H
#define FILTER_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QSet>
#include <QSettings>

#include <QDir>
#include <QFile>
#include <QTextStream>

class Filter
{
public:
	Filter();
	void regexp(QString search);
	//void htmlParse(QString search);
	static QSet<QString> findQuotes(QString post);
	bool filterMatched(QString post);
	QSet<QRegularExpression> filters;
	static QRegularExpression quoteRegExp;
	static QRegularExpression quotelinkRegExp;
	static QString colorString;
	static QString quoteString;
	static QString replaceQuoteStrings(QString &string);
private:
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
	QSet<QRegularExpression> loadFilterFile();
};

#endif // FILTER_H
