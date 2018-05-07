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

class Post;

class Filter
{
public:
	Filter();
	void regexp(QString search);
	//void htmlParse(QString search);
	static QSet<QString> findQuotes(QString post);
	bool filterMatched(QString post);
	QSet<QRegularExpression> filters;
	QHash<QString,QSet<QRegularExpression>> filters2;
	static QRegularExpression quoteRegExp;
	static QRegularExpression quotelinkRegExp;
	static QString colorString;
	static QString quoteString;
	static QString replaceQuoteStrings(QString &string);
	static QString replaceYouStrings(QRegularExpressionMatchIterator i, QString &string);
	static QString htmlParse(QString &html);
	static QString titleParse(QString &title);
	static QString toStrippedHtml(QString &text);
	void addFilter(QString &newFilter);
	void addFilter2(QString key, QString newFilter);
	bool filterMatched2(Post *p);
	void loadFilterFile2();

private:
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
	void loadFilterFile();
	void writeFilterFile2();
};

extern Filter filter;

#endif // FILTER_H
