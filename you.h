#ifndef YOU_H
#define YOU_H

#include <QSet>
#include <QString>
#include <QRegularExpression>

class You
{
public:
	You();
	void addYou(const QString &threadNum);
	bool hasYou(const QString &text);
	QRegularExpressionMatchIterator findYou(const QString &text);
	void updateRegExp();
	QRegularExpression matchYou;

	QString youFileName = "you";
	void loadYou(QString fileName = "you");
	void saveYou(QString fileName = "you");

private:
	QSet<QString> yourPosts;
};

extern You you;

#endif // YOU_H
