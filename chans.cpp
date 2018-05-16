#include "chans.h"
#include <QRegularExpressionMatch>

Chan* Chans::stringToType(QString url){
	//QRegularExpression fourchan(fourChanAPI->regURL(),QRegularExpression::CaseInsensitiveOption);
	QRegularExpression eightchan(eightChanAPI->regURL(),QRegularExpression::CaseInsensitiveOption);
	QRegularExpression twoChHk(twoChHkAPI->regURL(),QRegularExpression::CaseInsensitiveOption);
	if(eightchan.match(url).hasMatch())return eightChanAPI;
	else if(twoChHk.match(url).hasMatch())return twoChHkAPI;
	else return fourChanAPI;
}

void Chans::deleteAPIs(){
	delete fourChanAPI;
	delete eightChanAPI;
	delete twoChHkAPI;
}
