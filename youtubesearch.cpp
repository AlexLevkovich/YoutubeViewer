#include <stdio.h>
#include <QLocale>
#include <QApplication>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include "youtubesearch.h"
#include "default_values.h"
#include <search.h>
#include <QDebug>


extern QSettings *theSettings;
extern QString TOOLS_BIN_PATH;

const QString YouTubeSearch::orderByParameterString(YoutubeOrderBy id) {
    switch (id) {
        case relevance:
        return QObject::tr("Relevance");
        case date:
        return QObject::tr("Date");
        case rating:
        return QObject::tr("Rating");
        case title:
        return QObject::tr("Title");
        case videoCount:
        return QObject::tr("Video count");
        case viewCount:
        return QObject::tr("View count");
    }
    return "";
}

const QString YoutubeTime::timeParameterString(YoutubeTimeId id) {
    switch (id) {
        case before:
        return QObject::tr("Before");
        case after:
        return QObject::tr("After");
    }
    return "";
}

VideoInfo Media::best_video() const {
    Media * p_this = (Media *)this;
    QList<VideoInfo> infos = p_this->video_infos();
    if (infos.count() <= 0) return VideoInfo();
    return infos.at(infos.count()-1);
}

void Media::download_video_infos() {
    if (!video_infos().isEmpty()) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QProcess links_process;
    links_process.setProcessChannelMode(QProcess::MergedChannels);
    links_process.start(QString(YOUTUBE_VIDEO_URLS_PROCESS).arg(url().toString()).arg(TOOLS_BIN_PATH));
    links_process.waitForFinished(-1);

    if (links_process.exitCode() == 0) {
        bool ok;
        QtJson::JsonObject result = QtJson::parse(QString::fromUtf8(links_process.readAll()),ok).toMap();
        if (!ok) return;

        foreach(QVariant v_item, result["formats"].toList()) {
            QtJson::JsonObject item = v_item.value<QtJson::JsonObject>();
            VideoInfo info;
            info.filename = result["_filename"].toString();
            info.desc = FmtDesc(item);
            info.url = QUrl(item["url"].toString());
            if (!info.desc.description.isEmpty()) video_infos().append(info);
        }
    }

    QUrl audio_url;
    for (int i=0;i<video_infos().count();i++) {
        VideoInfo info = video_infos().at(i);
        if (info.isAudioOnly()) audio_url = info.url;
    }

    for (int i=0;i<video_infos().count();i++) {
        VideoInfo & info = video_infos()[i];
        if (info.hasExternalAudio()) info.audio_url = audio_url;
    }
    QApplication::restoreOverrideCursor();
}

static const QString convertYoutubeDuration(const QString & str) {
    QByteArray arr = str.toLocal8Bit();
    int days = 0;
    int hours = 0;
    int mins = 0;
    int secs = 0;
    if (::sscanf(arr.constData(),"P%dDT%dH%dM%dS",&days,&hours,&mins,&secs) != 4) {
        days = 0;
        hours = 0;
        mins = 0;
        secs = 0;
        if (::sscanf(arr.constData(),"PT%dH%dM%dS",&hours,&mins,&secs) != 3) {
            hours = 0;
            mins = 0;
            secs = 0;
            if (::sscanf(arr.constData(),"PT%dM%dS",&mins,&secs) != 2) {
                mins = 0;
                secs = 0;
                if (::sscanf(arr.constData(),"PT%dS",&secs) != 1) return str;
            }
        }
    }

    QString ret;
    QString temp;
    if (days > 0) ret += QString("%1 %2 ").arg(days).arg(QObject::tr("days"));
    if (hours > 0) ret += temp.sprintf("%.2d:",hours);
    else ret += "00:";
    if (mins > 0) ret += temp.sprintf("%.2d:",mins);
    else ret += "00:";
    if (secs > 0) ret += temp.sprintf("%.2d",secs);
    else ret += "00";

    return ret;
}

QStringList YouTubeSearch::m_categories = QStringList();

YouTubeSearch::YouTubeSearch(QObject *parent) : QObject(parent) {
    is_processing = false;
}

YouTubeSearch::~YouTubeSearch() {}

bool YouTubeSearch::search(const QString & userKey,
                           const QString & query,
                           const QString & category,
                           const QString & channel_id,
                           YoutubeOrderBy orderby,
                           const YoutubeTime & time,
                           const QString & pageToken) {

    m_userKey = userKey;
    m_query = query;
    m_categoryId = m_categories.indexOf(category);
    if (m_categoryId < 0) m_categoryId = 0;
    else m_categoryId++;
    m_channel_id = channel_id;
    m_orderby = orderby;
    m_time = time;
    m_nextPageToken.clear();
    m_prevPageToken.clear();
    return search_again(pageToken);
}

const QStringList YouTubeSearch::categories() {
    return m_categories;
}

bool YouTubeSearch::search_again(const QString & pageToken) {
    if (m_userKey.isEmpty()) return false;
    if (is_in_processing()) return false;
    if (m_query.isEmpty() && m_channel_id.isEmpty()) return false;

    is_processing = true;
    m_thread_count = theSettings->value("threads_count",THREADS_COUNT).toInt();
    m_medias.clear();
    resources_map.clear();

    QString url = QString(YOUTUBE_API).arg(orderby_values_en[m_orderby]).arg(m_userKey).arg(MAX_QUERY_COUNT);
    if (m_categoryId > 0) url += QString(CATEGORY_PART).arg(m_categoryId);
    if (!m_query.isEmpty()) url += QString(QUERY_PART).arg(m_query);
    if (!m_channel_id.isEmpty()) url += QString(AUTHOR_PART).arg(m_channel_id);
    if (!m_time.isNull()) url += m_time.toString();
    if (!pageToken.isEmpty()) url += QString(PAGE_TOKEN_PART).arg(pageToken);

    QNetworkReply * m_reply = manager.get(QNetworkRequest(url));
    m_reply->setProperty("type","main");
    m_reply->ignoreSslErrors();
    resources_map[m_reply] = 0;
    connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    connect(m_reply,SIGNAL(finished()),this,SLOT(finished()));

    return true;
}

void YouTubeSearch::terminate_processing(QIODevice * ignore_device) {
    QMap<QNetworkReply *,int> resources_map_temp = resources_map;
    resources_map.clear();
    QMapIterator<QNetworkReply *,int> i(resources_map_temp);
    while (i.hasNext()) {
        i.next();
        QNetworkReply * device = i.key();
        if (device != ignore_device) {
            device->disconnect(SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
            device->disconnect(SIGNAL(finished()),0,0);
            device->abort();
            delete device;
        }
    }
    is_processing = false;
    m_medias.clear();
    if (resources_map.count() > 0) terminate_processing(ignore_device);
    else emit completed(m_medias);
}

void YouTubeSearch::was_error(QNetworkReply::NetworkError err) {
    QNetworkReply * m_device = (QNetworkReply *)QObject::sender();
    if ((m_device->property("type").toString() == "image") && (err == QNetworkReply::ContentNotFoundError)) return;

    QString err_str = m_device->errorString();
    m_device->disconnect(SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    m_device->disconnect(SIGNAL(finished()),0,0);
    m_device->abort();
    terminate_processing(m_device);
    emit error(m_device->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),err_str);
    m_device->deleteLater();
}

void YouTubeSearch::finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();
    if (m_reply == NULL) return;

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        bool ok;
        QtJson::JsonObject result = QtJson::parse(QString::fromUtf8(m_reply->readAll()),ok).toMap();
        if (!ok) {
            emit error(100,tr("JSON parsing error!"));
        }
        else {
            m_nextPageToken = result["nextPageToken"].toString();
            m_prevPageToken = result["prevPageToken"].toString();

            foreach(QVariant v_item, result["items"].toList()) {
                QtJson::JsonObject item = v_item.value<QtJson::JsonObject>();
                Media media;
                QtJson::JsonObject id = item["id"].toMap();
                media.id() = id["videoId"].toString();
                QtJson::JsonObject snippet = item["snippet"].toMap();
                media.date() = QDateTime::fromString(snippet["publishedAt"].toString().left(19)+"+00:00",Qt::ISODate);
                media.title() = snippet["title"].toString();
                media.channel_id() = snippet["channelId"].toString();
                QtJson::JsonObject thumbnails = snippet["thumbnails"].toMap();
                media.image_url() = QUrl(thumbnails["high"].toMap()["url"].toString());
                media.author() = snippet["channelTitle"].toString();
                media.url() = QUrl(QString(YOUTUBE_URL_FORMAT).arg(media.id()));
                media.comments_url() = QUrl(QString(YOUTUBE_COMMENT_API).arg(m_userKey).arg(media.id()));
                m_medias << media;
            }
        }

        if (ok) {
            foreach(QVariant v_error, result["error"].toMap()) {
                QtJson::JsonObject error_tag = v_error.value<QtJson::JsonObject>();
                emit error(error_tag["code"].toInt(),error_tag["message"].toString());
            }
        }
    }

    m_reply->deleteLater();
    if (m_medias.count() > 0) {
        resources_map.clear();
        start_download_previews();
    }
    else {
        is_processing = false;
        emit completed(m_medias);
    }
}

bool YouTubeSearch::download_categories(const QString & userKey) {
    if (!m_categories.isEmpty()) return false;
    if (is_in_processing()) return false;

    QString url = QString(YOUTUBE_VIDEO_CATEGORIES).arg(userKey);
    QNetworkReply * m_reply = manager.get(QNetworkRequest(url));
    m_reply->setProperty("type","main");
    m_reply->ignoreSslErrors();
    resources_map[m_reply] = 0;
    is_processing = true;
    connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    connect(m_reply,SIGNAL(finished()),this,SLOT(categories_finished()));

    return true;
}

void YouTubeSearch::categories_finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();
    if (m_reply == NULL) return;

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        bool ok;
        QtJson::JsonObject result = QtJson::parse(QString::fromUtf8(m_reply->readAll()),ok).toMap();
        if (!ok) {
            emit error(100,tr("JSON parsing error!"));
        }
        else {
            foreach(QVariant v_item, result["items"].toList()) {
                QtJson::JsonObject item = v_item.value<QtJson::JsonObject>();
                m_categories << item["snippet"].toMap()["title"].toString();
            }
        }

        if (ok) {
            foreach(QVariant v_error, result["error"].toMap()) {
                QtJson::JsonObject error_tag = v_error.value<QtJson::JsonObject>();
                emit error(error_tag["code"].toInt(),error_tag["message"].toString());
            }
        }
    }

    m_reply->deleteLater();
    is_processing = false;
    emit categories_completed();
}

void YouTubeSearch::start_download_previews() {
    bool replyStarted = false;
    for (int i=0;i<m_medias.count();i++) {
        if (resources_map.count() >= m_thread_count) break;
        if (m_medias[i].image().isNull()) {
            if (resources_map.key(i,NULL) == NULL) {
                QNetworkReply * reply = getImageDownloadReply(m_medias[i].image_url());
                resources_map[reply] = i;
                replyStarted = true;
            }
        }
    }

    if (!replyStarted && resources_map.isEmpty()) {
        start_download_full_descriptions();
    }
}

QNetworkReply * YouTubeSearch::getImageDownloadReply(const QUrl & image_url) {
    QNetworkReply * image_reply = manager.get(QNetworkRequest(image_url));
    image_reply->setProperty("type","image");
    image_reply->ignoreSslErrors();
    connect(image_reply,SIGNAL(finished()),this,SLOT(image_finished()));
    connect(image_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    return image_reply;
}

void YouTubeSearch::image_finished() {
    QNetworkReply * image_reply = (QNetworkReply *)QObject::sender();
    if (image_reply == NULL) return;

    int statusCode = image_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        QByteArray data = image_reply->readAll();
        m_medias[resources_map[image_reply]].image() = (data.isEmpty()?QImage(":/images/res/default.png"):QImage::fromData(data)).scaled(theSettings->value("preview_size",QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT)).toSize(),Qt::KeepAspectRatio);
    }
    else {
        if (statusCode == 404) {
            m_medias[resources_map[image_reply]].image() = QImage(":/images/res/default.png").scaled(theSettings->value("preview_size",QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT)).toSize());
        }
    }
    resources_map.remove(image_reply);
    image_reply->deleteLater();
    start_download_previews();
}

void YouTubeSearch::start_download_full_descriptions() {
    bool replyStarted = false;
    for (int i=0;i<m_medias.count();i++) {
        if (resources_map.count() >= m_thread_count) break;
        if (m_medias[i].description().isNull() && !m_medias[i].doIgnore()) {
            if (resources_map.key(i,NULL) == NULL) {
                QNetworkReply * reply = getDescriptionDownloadReply(QString(YOUTUBE_VIDEO).arg(m_userKey).arg(m_medias[i].id()));
                resources_map[reply] = i;
                replyStarted = true;
            }
        }
    }

    if (!replyStarted && resources_map.isEmpty()) {
        is_processing = false;
        for (int i=(m_medias.count()-1);i>=0;i--) {
            if (m_medias[i].doIgnore()) m_medias.removeAt(i);
        }
        emit completed(m_medias);
    }
}

QNetworkReply * YouTubeSearch::getDescriptionDownloadReply(const QUrl & url) {
    QNetworkReply * desc_reply = manager.get(QNetworkRequest(url));
    desc_reply->setProperty("type","desc");
    desc_reply->ignoreSslErrors();
    connect(desc_reply,SIGNAL(finished()),this,SLOT(desc_finished()));
    connect(desc_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    return desc_reply;
}

void YouTubeSearch::desc_finished() {
    QNetworkReply * desc_reply = (QNetworkReply *)QObject::sender();
    if (desc_reply == NULL) return;

    int statusCode = desc_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        bool ok;
        QtJson::JsonObject result = QtJson::parse(QString::fromUtf8(desc_reply->readAll()),ok).toMap();
        if (!ok) {
            emit error(100,tr("JSON parsing error!"));
        }
        else {
            bool is_ok = false;
            foreach(QVariant v_item, result["items"].toList()) {
                QtJson::JsonObject item = v_item.value<QtJson::JsonObject>();
                QtJson::JsonObject snippet = item["snippet"].toMap();
                QtJson::JsonObject contentDetails = item["contentDetails"].toMap();
                QtJson::JsonObject statistics = item["statistics"].toMap();
                double likeCount = statistics["likeCount"].toDouble();
                double dislikeCount = statistics["dislikeCount"].toDouble();
                QString desc = snippet["description"].toString();
                if (desc.isNull()) desc="";
                m_medias[resources_map[desc_reply]].description() = desc;
                m_medias[resources_map[desc_reply]].duration() = convertYoutubeDuration(contentDetails["duration"].toString());
                m_medias[resources_map[desc_reply]].rating() = ((likeCount-dislikeCount)*5.0)/(likeCount+dislikeCount);
                int categoryId = snippet["categoryId"].toInt()-1;
                if ((categoryId >= 0) && (m_categories.count() > categoryId)) m_medias[resources_map[desc_reply]].category() = m_categories[categoryId];
                is_ok = true;
            }
            if (!is_ok) {
                m_medias[resources_map[desc_reply]].doIgnore() = true;
            }
        }

        if (ok) {
            foreach(QVariant v_error, result["error"].toMap()) {
                QtJson::JsonObject error_tag = v_error.value<QtJson::JsonObject>();
                emit error(error_tag["code"].toInt(),error_tag["message"].toString());
            }
        }
    }
    resources_map.remove(desc_reply);
    desc_reply->deleteLater();
    start_download_full_descriptions();
}

bool YouTubeSearch::is_in_processing() {
    return is_processing;
}

