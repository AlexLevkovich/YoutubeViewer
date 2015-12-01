#ifndef YOUTUBECOMMENTSREADER_H
#define YOUTUBECOMMENTSREADER_H

#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>

#define MAX_QUERY_COUNT 50

class CommentsInfo {
public:
    inline CommentsInfo() {}

    inline QString & comment() { return m_comment; }
    inline QDateTime & date() { return m_date; }
    inline QString & author() { return m_author; }
    inline QString & title() { return m_title; }
    QString toString() const;

private:
    QString m_comment;
    QString m_title;
    QString m_author;
    QDateTime m_date;
};

class YoutubeCommentsReader : public QObject {
    Q_OBJECT
public:
    explicit YoutubeCommentsReader(QObject *parent = 0);
    explicit YoutubeCommentsReader(const QUrl & url,QObject *parent = 0);
    bool start_search(const QString & pageToken);
    void setUrl(const QUrl & url);
    QUrl url() const;
    QString nextPageToken() const;
protected:
    QNetworkReply * getNetworkReply(const QString & pageToken);

signals:
    void error(const QString & error);
    void completed(const QString & html);

private slots:
    void was_error(QNetworkReply::NetworkError error);
    void finished();

private:
    QUrl m_url;
    QNetworkAccessManager manager;
    QString m_nextPageToken;
};

#endif // YOUTUBECOMMENTSREADER_H
