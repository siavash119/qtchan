#include "chans.h"
#include <QRegularExpressionMatch>

Chan *fourChanAPI = new FourChan();
Chan *eightChanAPI = new EightChan();

Chan* Chans::stringToType(QString url){
	//QRegularExpression fourchan(fourChanAPI->regURL(),QRegularExpression::CaseInsensitiveOption);
	QRegularExpression eightchan(eightChanAPI->regURL(),QRegularExpression::CaseInsensitiveOption);
	if(eightchan.match(url).hasMatch())return eightChanAPI;
	else return fourChanAPI;
}

void Chans::deleteAPIs(){
	delete fourChanAPI;
	delete eightChanAPI;
}
