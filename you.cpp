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
	QString boardName = "UNK";
	QSet<QString> posts = yourPosts.value(boardName);
	while(!in.atEnd()) {
		QString line = in.readLine();
		if(line.at(0) == ':'){
			boardName = line.mid(1);
			continue;
		}
		if(line.at(0) == ',') line = line.mid(1);
		QStringList fields = line.split(",");
		posts = yourPosts.value(boardName);
		foreach(QString temp, fields){
			if(!temp.isEmpty())posts.insert(temp);
		}
		yourPosts.insert(boardName,posts);
		updateRegExp(boardName);
	}
	file.close();
	qDebug() << yourPosts;
}

void You::saveYou(const QString fileName){
	QFile file(fileName);
	if (file.open(QIODevice::ReadWrite))
	{
		QTextStream stream(&file);
		foreach(QString boardName, yourPosts.keys()){
			stream << ":" << boardName << endl;
			foreach(QString postNum, yourPosts.value(boardName)){
				stream << "," << postNum;
			}
			stream << endl;
		}
	}
	file.close();
}

void You::addYou(const QString &boardName, const QString &postNum){
	QSet<QString> posts = yourPosts.value(boardName);
	posts.insert(postNum);
	yourPosts.insert(boardName,posts);
	//yourPosts.value(boardName).insert(postNum);
	updateRegExp(boardName);
}

void You::updateRegExp(const QString &boardName){
	QString pattern = "&gt;&gt;(";
	foreach (QString postNum, yourPosts.value(boardName)) {
		pattern += postNum % "|";
	}
	pattern.chop(1);
	pattern += ")";
	QRegularExpression patt = matchYou.value(boardName);
	patt.setPattern(pattern);
	patt.optimize();
	matchYou.insert(boardName,patt);
}

bool You::hasYou(const QString &boardName, const QString &postNum){
	QSet<QString> temp = yourPosts.value(boardName);
	if(!temp.isEmpty() && temp.contains(postNum)) return true;
	return false;
}

QRegularExpressionMatchIterator You::findYou(const QString &boardName, const QString &text){
	QRegularExpression temp = matchYou.value(boardName);
	if(!temp.pattern().isEmpty()) return temp.globalMatch(text);
	else return QRegularExpressionMatchIterator();
}

You you;
