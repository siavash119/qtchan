#include "threadtabhelper.h"
#include "netcontroller.h"

ThreadTabHelper::ThreadTabHelper(QString board, QString thread, QWidget* parent){
    this->board = board;
    this->thread = thread;
    this->parent = parent;
    this->threadUrl = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
    QSettings settings;
    this->expandAll = settings.value("autoExpand",false).toBool();
}

void ThreadTabHelper::startUp(){
    QDir().mkpath(board+"/"+thread+"/thumbs");
    qDebug() << threadUrl;
    request = QNetworkRequest(QUrl(threadUrl));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    getPosts();
    updateTimer = new QTimer();
    updateTimer->setInterval(30000);
    updateTimer->start();
    QSettings settings;
    if(settings.value("autoUpdate").toBool()){
        connectionUpdate = connect(updateTimer, &QTimer::timeout,[=](){
            getPosts();
        });
    }
}

ThreadTabHelper::~ThreadTabHelper(){
    //disconnect(connectionUpdate);
    //disconnect(connectionPost);
    qDebug () << "destroying ThreadTabHelper";
    updateTimer->stop();
    delete updateTimer;
    if(gettingReply) reply->abort();
    //delete updateTimer;
}

void ThreadTabHelper::setAutoUpdate(bool update){
    disconnect(connectionUpdate);
    if(update){
        connectionUpdate = connect(updateTimer, &QTimer::timeout,[=](){
            getPosts();
        });
    }
}

void ThreadTabHelper::getPosts(){
    qDebug() << "getting posts for" << threadUrl;
    reply = nc.jsonManager->get(request);
    gettingReply = true;
    connectionPost = connect(reply, &QNetworkReply::finished,[=](){
        loadPosts();
    });
}

//todo put this in background thread
void ThreadTabHelper::writeJson(QString &board, QString &thread, QByteArray &rep){
    QFile jsonFile(board+"/"+thread+"/"+thread+".json");
    jsonFile.open(QIODevice::WriteOnly);
    QDataStream out(&jsonFile);
    out << rep;
    jsonFile.close();
}

void ThreadTabHelper::loadPosts(){
    gettingReply = false;
    if(reply->error()){
        qDebug().noquote() << "loading post error:" << reply->errorString();
        if(reply->error() == QNetworkReply::ContentNotFoundError){
            qDebug() << "Stopping timer for" << threadUrl;
            updateTimer->stop();
            emit thread404();
        }
        reply->deleteLater();
        return;
    }
    //write to file and make json array
    QByteArray rep = reply->readAll();
    disconnect(connectionPost);
    QtConcurrent::run(&ThreadTabHelper::writeJson,board, thread, rep);
    posts = QJsonDocument::fromJson(rep).object().value("posts").toArray();
    int length = posts.size();
    qDebug().noquote() << QString("length of ").append(threadUrl).append(" is ").append(QString::number(length));
    int i = tfMap.size();
    QSettings settings;
    bool loadFile = settings.value("autoExpand",false).toBool() || this->expandAll;
    while(!abort && i<length){
        p = posts.at(i).toObject();
        ThreadForm *tf = new ThreadForm(board,thread,PostType::Reply,true,loadFile,parent);
        tf->load(p);
        emit newTF(tf);
        tfMap.insert(tf->post.no,tf);
        if(i==0){
            if(tf->post.sub.length())emit windowTitle("/"+board+"/"+thread + " - " + tf->post.sub);
            else if(tf->post.com.length()){
                QString temp = tf->post.com;
                emit windowTitle("/"+board+"/"+thread + " - " +
                     ThreadForm::htmlParse(temp
                        .replace(QRegExp("</?span( class=\"quote\" style=\"color:#[\\d|\\w]{6}\")?>"),""))
                        .replace("\n"," "));
            }
        }
        QSet<QString> quotes = tf->quotelinks;
        QPointer<ThreadForm> replyTo;
        foreach (const QString &orig, quotes)
        {
            replyTo = tfMap.find(orig).value();
            if(replyTo != tfMap.end().value()){
                replyTo->replies.insert(tf->post.no.toDouble(),tf->post.no);
                //replyTo->setReplies();
            }
        }
        if(replyTo) replyTo->setReplies();
        i++;
        QCoreApplication::processEvents();
    }
    if(!abort) emit addStretch();
    //emit scrollIt();
    reply->deleteLater();
}

void ThreadTabHelper::loadAllImages(){
    expandAll = !expandAll;
    qDebug() << "settings expandAll for" << threadUrl << "to" << expandAll;
    if(expandAll){
        QMapIterator<QString,ThreadForm*> mapI(tfMap);
        while (mapI.hasNext()) {
            mapI.next();
            mapI.value()->loadOrig();
        }
    }
}
