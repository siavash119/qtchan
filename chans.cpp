#include "chans.h"
#include <QRegularExpressionMatch>

Chan *fourChanAPI = new FourChan;

Chan* Chans::stringToType(QString url){
	QRegularExpression fourchan(fourChanAPI->regURL(),QRegularExpression::CaseInsensitiveOption);
	QRegularExpression eightchan("^(?:(?:https?:\\/\\/)?8ch.net)\\/?.*$",QRegularExpression::CaseInsensitiveOption);
	if(eightchan.match(url).hasMatch())return NULL;
	else return fourChanAPI;
}

void Chans::deleteAPIs(){
	delete fourChanAPI;
}
