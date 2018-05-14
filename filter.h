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
	QHash< QString,QHash<QRegularExpression,QString> > filters2;
	static QRegularExpression quoteRegExp;
	static QRegularExpression quotelinkRegExp;
	static QString colorString;
	static QString quoteString;
	static QString replaceQuoteStrings(QString &string);
	static QString replaceYouStrings(QRegularExpressionMatchIterator i, QString &string);
	static QString htmlParse(QString &html);
	static QString titleParse(QString &title);
	static QString toStrippedHtml(QString &text);
	static QString filterEscape(QString &string);

	void addFilter(QString &newFilter);
	void addFilter2(QString key, QString newFilter, QString options = QString());
	bool filterMatched2(Post *p);
	void loadFilterFile2();
	QHash< QString,QHash<QRegularExpression,QString> > filterMatchedPerTab(QString board, QString tabType);
	bool useFilterPerTab(QString &options, QString board, QString tabType);

private:
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
	void loadFilterFile();
	void writeFilterFile2();
	bool useFilter(QString &options, Post *p);
};

extern Filter filter;

#endif // FILTER_H
