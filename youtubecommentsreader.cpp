#include "youtubecommentsreader.h"
#include <QNetworkRequest>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include "json.h"

QString CommentsInfo::toString() const {
    CommentsInfo * p_this = (CommentsInfo *)this;
    return QString("<span style=\"font-weight: bold;\">%1</span>"
                   "<hr><span style=\"color: rgb(102, 102, 102);\">"
                   "%2 | %3</span><br>"
                   "%4<br><br>").arg(p_this->title()).arg(p_this->date().toString()).arg(p_this->author()).arg(p_this->comment());
}

YoutubeCommentsReader::YoutubeCommentsReader(QObject *parent) : QObject(parent) {
}

YoutubeCommentsReader::YoutubeCommentsReader(const QUrl & url,QObject *parent) : QObject(parent) {
    m_url = url;
}

void YoutubeCommentsReader::setUrl(const QUrl & url) {
    m_url = url;
}

QUrl YoutubeCommentsReader::url() const {
    return m_url;
}

QNetworkReply * YoutubeCommentsReader::getNetworkReply(const QString & pageToken) {
    QString url = m_url.toString();
    if (!pageToken.isEmpty()) url += QString("&pageToken=%1").arg(pageToken);
    QNetworkReply * reply = manager.get(QNetworkRequest(url));
    reply->ignoreSslErrors();
    connect(reply,SIGNAL(finished()),this,SLOT(finished()));
    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    return reply;
}

QString YoutubeCommentsReader::nextPageToken() const {
    return m_nextPageToken;
}

bool YoutubeCommentsReader::start_search(const QString & pageToken) {
    if (!m_url.isValid()) return false;
    return (getNetworkReply(pageToken) != NULL);
}

void YoutubeCommentsReader::was_error(QNetworkReply::NetworkError /*error*/) {
    emit error(((QNetworkReply *)QObject::sender())->errorString());
}

void YoutubeCommentsReader::finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();
    if (m_reply == NULL) return;

    QString html("<html><head><meta content=\"text/html; charset=UTF-8\" "
                 "http-equiv=\"content-type\"><title></title></head><body>");

    QXmlStreamReader xml;

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        bool ok;
        QtJson::JsonObject result = QtJson::parse(QString::fromUtf8(m_reply->readAll()),ok).toMap();
        if (!ok) {
            emit error(tr("JSON parsing error!"));
        }
        else {
            m_nextPageToken = result["nextPageToken"].toString();

            foreach(QVariant v_item, result["items"].toList()) {
                QtJson::JsonObject item = v_item.value<QtJson::JsonObject>();
                QtJson::JsonObject snippet = item["snippet"].toMap();
                snippet = (snippet["topLevelComment"].toMap())["snippet"].toMap();
                CommentsInfo comment_info;
                QString commentString = snippet["textDisplay"].toString();
                comment_info.title() = commentString.left(20)+"...";
                comment_info.date() = QDateTime::fromString(snippet["updatedAt"].toString().left(19)+"+00:00",Qt::ISODate);
                comment_info.author() = snippet["authorDisplayName"].toString();
                comment_info.comment() = commentString;
                html += "\r\n" + comment_info.toString();
            }
        }

        if (ok) {
            foreach(QVariant v_error, result["error"].toMap()) {
                QtJson::JsonObject error_tag = v_error.value<QtJson::JsonObject>();
                emit error(error_tag["message"].toString());
            }
        }
    }

    m_reply->deleteLater();
    html += "\r\n</body></html>";
    emit completed(html);
}
