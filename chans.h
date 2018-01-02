#ifndef CHANS_H
#define CHANS_H

#include "chans/fourchan.h"
#include "chans/eightchan.h"

class Chans
{
public:
	static Chan* stringToType(QString url);
	static void deleteAPIs();
};

extern Chan *fourChanAPI;
extern Chan *eightChanAPI;
#endif // CHANS_H
