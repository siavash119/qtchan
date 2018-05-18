#include "you.h"
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
You::You()
{
	loadYou(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/you");
}

void You::loadYou(QString fileName){
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly)) {
		qDebug() << "error opening your posts file:" << file.errorString();
	}

	QTextStream in(&file);
	QString api = "UNK";
	QString board = "UNK";
	while(!in.atEnd()) {
		QString line = in.readLine();
		if(line.at(0) == ':'){
			api = line.mid(1);
			continue;
		}
		if(line.at(0) == ';'){
			board = line.mid(1);
			continue;
		}
		if(line.at(0) == ',') line = line.mid(1);
		QStringList fields = line.split(",");
		fields.toSet();
		QSet<QString> posts;
		foreach(QString temp, fields){
			if(!temp.isEmpty()) posts.insert(temp);
		}
		QHash< QString, QSet< QString> > boards = yourPosts.value(api);
		boards.insert(board,posts);
		yourPosts.insert(api,boards);
		updateRegExp(api,board);
	}
	file.close();
	qDebug() << "######" << endl << "YOU" << endl << yourPosts << endl << "########";
}

void You::saveYou(const QString fileName){
	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly))
	{
		QTextStream stream(&file);
		foreach(QString api, yourPosts.keys()){
			stream << ':' << api << endl;
			foreach(QString board, yourPosts.value(api).keys()){
				stream << ';' << board << endl;
				foreach(QString post, yourPosts.value(api).value(board)){
					stream << "," << post;
				}
			stream << endl;
			}
		}
		file.close();
	}
}

void You::addYou(const QString &api, const QString &board, const QString &post){
	qDebug().noquote().nospace() << "adding you " << api << '/' << board << '/' << post;
	QHash< QString, QSet< QString> > boards = yourPosts.value(api);
	QSet<QString> posts = boards.value(board);
	posts.insert(post);
	boards.insert(board,posts);
	yourPosts.insert(api,boards);
	//yourPosts.value(boardName).insert(postNum);
	updateRegExp(api,board);
}

void You::updateRegExp(const QString &api, const QString &board){
	QString pattern = "&gt;&gt;(";
	foreach (QString post, yourPosts.value(api).value(board)) {
		pattern += post % "|";
	}
	pattern.chop(1);
	pattern += ")";
	QHash<QString,QRegularExpression> patts = matchYou.value(api);
	QRegularExpression patt = patts.value(board);
	patt.setPattern(pattern);
	patt.optimize();
	patts.insert(board,patt);
	matchYou.insert(api,patts);
}

bool You::hasYou(const QString &api, const QString &board, const QString &post){
	QSet<QString> temp = yourPosts.value(api).value(board);
	if(!temp.isEmpty() && temp.contains(post)) return true;
	return false;
}

QRegularExpressionMatchIterator You::findYou(const QString &api, const QString &board, const QString &text){
	QRegularExpression temp = matchYou.value(api).value(board);
	if(!temp.pattern().isEmpty()) return temp.globalMatch(text);
	else return QRegularExpressionMatchIterator();
}

You you;
