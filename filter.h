#ifndef FILTER_H
#define FILTER_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QSet>


class Filter
{
public:
	Filter();
	void regexp(QString search);
	//void htmlParse(QString search);
	static QSet<QString> findQuotes(QString post);

private:
	QRegularExpression quotelink;
	QRegularExpressionMatch quotelinkMatch;
	QRegularExpressionMatchIterator quotelinkMatches;
};

#endif // FILTER_H
