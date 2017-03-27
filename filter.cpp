#include "filter.h"

Filter::Filter()
{
    quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
}

void Filter::htmlParse(QString search){

}

QSet<QString> Filter::findQuotes(QString post){
    QSet<QString> quotes;
    quotelinkMatches = quotelink.globalMatch(post);
    while (quotelinkMatches.hasNext()) {
        quotelinkMatch = quotelinkMatches.next();
        quotes.insert(QString(quotelinkMatch.captured(1)));
    }
    return quotes;
}
