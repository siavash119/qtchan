#include "threadtabhelper.h"
#include "netcontroller.h"

ThreadTabHelper::ThreadTabHelper(QString board, QString thread, QWidget* parent){
    this->board = board;
    this->thread = thread;
    this->parent = parent;
    this->threadUrl = "https://a.4cdn.org/"+board+"/thread/"+thread+".json";
}

void ThreadTabHelper::startUp(){
    QDir().mkpath(board+"/"+thread+"/thumbs");
    qDebug() << threadUrl;
    request = QNetworkRequest(QUrl(threadUrl));
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    getPosts();
}

void ThreadTabHelper::getPosts(){
    reply = nc.jsonManager->get(request);
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
    if(reply->error()){
        qDebug().noquote() << "loading post error:" << reply->errorString();
        reply->deleteLater();
        return;
    }
    //write to file and make json array
    QByteArray rep = reply->readAll();
    QtConcurrent::run(&ThreadTabHelper::writeJson,board, thread, rep);
    posts = QJsonDocument::fromJson(rep).object().value("posts").toArray();
    int length = posts.size();
    qDebug().noquote() << QString("length of ").append(threadUrl).append(" is ").append(QString::number(length));
    int i = tfMap.size();
    while(!abort && i<length){
        p = posts.at(i).toObject();
        ThreadForm *tf = new ThreadForm(board,thread,PostType::Reply,true,parent);
        tf->load(p);
        emit newTF(tf);
        tfMap.insert(tf->post->no,tf);
        if(i==0){
            if(tf->post->sub.length())emit windowTitle("/"+board+"/"+thread + " - " + tf->post->sub);
            else if(tf->post->com.length()) emit windowTitle("/"+board+"/"+thread + " - " + ThreadForm::htmlParse(tf->post->com).replace("\n"," "));
        }
        QSet<QString> quotes = tf->quotelinks;
        //QCoreApplication::processEvents();
        QPointer<ThreadForm> replyTo;
        foreach (const QString &orig, quotes)
        {
            replyTo = tfMap.find(orig).value();
            if(replyTo != tfMap.end().value()){
                replyTo->replies.insert(tf->post->no.toDouble(),tf->post->no);
                //replyTo->setReplies();
            }
        }
        if(replyTo) replyTo->setReplies();
        i++;
        QCoreApplication::processEvents();
    }
    emit addStretch();
    reply->deleteLater();
    disconnect(connectionPost);
}
