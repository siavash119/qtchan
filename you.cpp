#include "you.h"
#include <QFile>
#include <QDebug>

You::You()
{
	loadYou();
}

void You::loadYou(QString fileName){
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly)) {
		qDebug() << "error opening your posts file:" << file.errorString();
	}

	QTextStream in(&file);

	while(!in.atEnd()) {
		QString line = in.readLine();
		QStringList fields = line.split(",");
		foreach(QString temp, fields){
			yourPosts.insert(temp);
		}
	}
	file.close();
	updateRegExp();
}

void You::saveYou(QString fileName){
	QFile file(fileName);
	if (file.open(QIODevice::ReadWrite))
	{
		QTextStream stream(&file);
		foreach(QString postNum, yourPosts){
			stream << postNum << endl;
		}
	}
	file.close();
}

void You::addYou(const QString &threadNum){
	yourPosts.insert(threadNum);
	updateRegExp();
}

void You::updateRegExp(){
	QString pattern = "&gt;&gt;(";
	foreach (QString postNum, yourPosts) {
		pattern += postNum % "|";
	}
	pattern.chop(1);
	pattern += ")";
	matchYou.setPattern(pattern);
	matchYou.optimize();
}

bool You::hasYou(const QString &text){
	if(yourPosts.contains(text)) return true;
	return false;
}

QRegularExpressionMatchIterator You::findYou(const QString &text){
	return matchYou.globalMatch(text);
}

You you;
