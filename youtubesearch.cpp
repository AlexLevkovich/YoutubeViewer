#include <stdio.h>
#include <QLocale>
#include <QApplication>
#include <QEventLoop>
#include <QMainWindow>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include "youtubesearch.h"
#include "default_values.h"
#include <search.h>
#include <QDebug>
#include "createnewyoutubekeydialog.h"


extern QSettings *theSettings;
extern QString TOOLS_BIN_PATH;
extern QMainWindow * findMainWindow();

QImage YPlayList::image() const {
    if (m_img.isNull() && m_img_url.isValid()) {
        QString error;
        return QImage::fromData(YouTubeSearch::downloadResource(m_img_url,error));
    }
    return m_img;
}


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
            info.m_desc = FmtDesc(item);
            info.m_filename = result["_filename"].toString();
            if (info.isAudioOnly()) info.m_filename += "a";
            info.m_url = QUrl(item["url"].toString());
            m_video_infos.append(info);
        }
    }

    QUrl audio_url;
    for (int i=0;i<video_infos().count();i++) {
        VideoInfo info = video_infos().at(i);
        if (info.isAudioOnly()) audio_url = info.url();
    }

    for (int i=0;i<video_infos().count();i++) {
        VideoInfo info = video_infos()[i];
        if (info.hasExternalAudio()) {
            info.m_audio_url = audio_url;
            m_video_infos[i] = info;
        }
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
QNetworkAccessManager * YouTubeSearch::manager = NULL;

YouTubeSearch::YouTubeSearch(QObject *parent) : QObject(parent) {
    is_processing = false;
    if (qApp == NULL) {
        qDebug() << "QApplication should be initilized first!!!";
        ::exit(1);
        return;
    }
    if (manager == NULL) manager = new QNetworkAccessManager(qApp);
}

YouTubeSearch::~YouTubeSearch() {}

bool YouTubeSearch::search(const QString & userKey,
                           const QString & query,
                           const QString & category,
                           const QString & channel,
                           const QString & playlist_id,
                           YoutubeOrderBy orderby,
                           const YoutubeTime & time,
                           const QString & pageToken) {

    m_userKey = userKey;
    m_query = query;
    m_orderby = orderby;
    m_time = time;
    m_nextPageToken.clear();
    m_prevPageToken.clear();
    m_category = category;
    m_playlist_id = playlist_id;
    m_pageToken = pageToken;
    m_channel_id.clear();
    m_channel = channel;

    if (m_categories.isEmpty()) download_categories(theSettings->value("youtube_user_key","").toString());
    else {
        m_categoryId = m_categories.indexOf(category);
        if (m_categoryId < 0) m_categoryId = 0;
        else m_categoryId++;

        if (!channel.isEmpty()) download_channel_id(channel);
        else return search_again(pageToken);
    }

    return true;
}

QList<YPlayList> YouTubeSearch::downloadChannelPlaylists(const QString & channel_id,const QString & userKey,QString & error) {
    return downloadChannelPlaylists(channel_id,userKey,error,"");
}

QList<YPlayList> YouTubeSearch::downloadChannelPlaylists(const QString & channel_id,const QString & userKey,QString & error,const QString & pageToken) {
    QString url = QString(YOUTUBE_PLAYLIST_SEARCH).arg(userKey).arg(channel_id).arg(MAX_QUERY_COUNT);
    if (!pageToken.isEmpty()) url += QString(PAGE_TOKEN_PART).arg(pageToken);

    YPlayList playlist;
    QList<YPlayList> ret_list;
    QEventLoop loop;
    QNetworkReply * m_reply = manager->get(QNetworkRequest(url));
    m_reply->ignoreSslErrors();
    connect(m_reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    error = m_reply->errorString();
    m_reply->abort();
    m_reply->deleteLater();

    if (statusCode >= 200 && statusCode < 300) {
        bool ok;
        QtJson::JsonObject result = QtJson::parse(QString::fromUtf8(m_reply->readAll()),ok).toMap();
        if (!ok) {
            error = tr("JSON parsing error!");
        }
        else {
            QString nextPageToken = result["nextPageToken"].toString();
            foreach(QVariant v_item, result["items"].toList()) {
                QtJson::JsonObject item = v_item.value<QtJson::JsonObject>();
                QtJson::JsonObject snippet = item["snippet"].toMap();
                QtJson::JsonObject id = item["id"].toMap();
                playlist.m_id = id["playlistId"].toString();
                playlist.m_title = snippet["title"].toString();
                QtJson::JsonObject thumbnails = snippet["thumbnails"].toMap();
                QtJson::JsonObject high = thumbnails["high"].toMap();
                if (!high.isEmpty()) playlist.m_img_url = QUrl(high["url"].toString());
                else {
                    QtJson::JsonObject medium = thumbnails["medium"].toMap();
                    if (!medium.isEmpty()) playlist.m_img_url = QUrl(medium["url"].toString());
                    else {
                        QtJson::JsonObject def = thumbnails["default"].toMap();
                        if (!def.isEmpty()) playlist.m_img_url = QUrl(def["url"].toString());
                    }
                }
                ret_list.append(playlist);
            }
            error.clear();
            if (!nextPageToken.isEmpty()) ret_list += downloadChannelPlaylists(channel_id,userKey,error,nextPageToken);
        }

        if (ok) {
            foreach(QVariant v_error, result["error"].toMap()) {
                QtJson::JsonObject error_tag = v_error.value<QtJson::JsonObject>();
                error = error_tag["message"].toString();
            }
        }
    }

    return ret_list;
}

QString YouTubeSearch::downloadChannelId(const QString & channel_name,const QString & userKey,QString & error) {
    if (channel_name.isEmpty()) {
        error = "Input channel name is empty!!!";
        return QString();
    }

    QString channel_id;

    QString url = QString(YOUTUBE_CHANNELLIST_SEARCH).arg(userKey);
    url += QString(QUERY_PART).arg(channel_name);

    QEventLoop loop;
    QNetworkReply * m_reply = manager->get(QNetworkRequest(url));
    m_reply->ignoreSslErrors();
    connect(m_reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) {
        bool ok;
        QtJson::JsonObject result = QtJson::parse(QString::fromUtf8(m_reply->readAll()),ok).toMap();
        if (!ok) {
            error = tr("JSON parsing error!");
        }
        else {
            foreach(QVariant v_item, result["items"].toList()) {
                QtJson::JsonObject item = v_item.value<QtJson::JsonObject>();
                QtJson::JsonObject snippet = item["snippet"].toMap();
                if (channel_name == snippet["title"].toString()) {
                    channel_id = snippet["channelId"].toString();
                }
            }
        }

        if (ok) {
            foreach(QVariant v_error, result["error"].toMap()) {
                QtJson::JsonObject error_tag = v_error.value<QtJson::JsonObject>();
                error = error_tag["message"].toString();
            }
        }
    }
    else error = m_reply->errorString();

    m_reply->abort();
    m_reply->deleteLater();

    return channel_id;
}

void YouTubeSearch::download_channel_id(const QString & channel_name) {
    if (channel_name.isEmpty()) return;

    QString url = QString(YOUTUBE_CHANNELLIST_SEARCH).arg(m_userKey);
    url += QString(QUERY_PART).arg(channel_name);

    QNetworkReply * m_reply = manager->get(QNetworkRequest(url));
    m_reply->setProperty("channel_name",channel_name);
    m_reply->ignoreSslErrors();
    connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(get_channel_id_was_error(QNetworkReply::NetworkError)));
    connect(m_reply,SIGNAL(finished()),this,SLOT(on_channel_id_finished()));
}

void YouTubeSearch::on_channel_id_finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

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
                QtJson::JsonObject snippet = item["snippet"].toMap();
                if (m_reply->property("channel_name").toString() == snippet["title"].toString()) {
                    m_channel_id = snippet["channelId"].toString();
                    search_again(m_pageToken);
                }
            }
        }

        if (ok) {
            foreach(QVariant v_error, result["error"].toMap()) {
                QtJson::JsonObject error_tag = v_error.value<QtJson::JsonObject>();
                emit error(error_tag["code"].toInt(),error_tag["message"].toString());
            }
        }
    }

    m_reply->abort();
    m_reply->deleteLater();
}

void YouTubeSearch::get_channel_id_was_error(QNetworkReply::NetworkError err) {
    QNetworkReply * m_device = (QNetworkReply *)QObject::sender();
    if (err == QNetworkReply::ContentNotFoundError) return;

    emit error(m_device->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),m_device->errorString());
}

const QStringList YouTubeSearch::categories() {
    if (m_categories.isEmpty()) {
        YouTubeSearch().download_categories(theSettings->value("youtube_user_key","").toString(),true);
    }
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

    QString url;
    if (m_playlist_id.isEmpty()) {
        url = QString(YOUTUBE_VIDEOLIST_SEARCH).arg(orderby_values_en[m_orderby]).arg(m_userKey).arg(MAX_QUERY_COUNT);
        if (m_categoryId > 0) url += QString(CATEGORY_PART).arg(m_categoryId);
        if (!m_query.isEmpty()) url += QString(QUERY_PART).arg(m_query);
        if (!m_channel_id.isEmpty()) url += QString(AUTHOR_PART).arg(m_channel_id);
        if (!m_time.isNull()) url += m_time.toString();
    }
    else url = QString(YOUTUBE_PLAYLISTITEMS_SEARCH).arg(m_userKey).arg(m_playlist_id).arg(MAX_QUERY_COUNT);
    if (!pageToken.isEmpty()) url += QString(PAGE_TOKEN_PART).arg(pageToken);


    QNetworkReply * m_reply = manager->get(QNetworkRequest(url));
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

    QtJson::JsonObject id;
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
                QtJson::JsonObject snippet = item["snippet"].toMap();
                if (m_playlist_id.isEmpty()) id = item["id"].toMap();
                else id = snippet["resourceId"].toMap();
                media.m_id = id["videoId"].toString();
                media.m_date = QDateTime::fromString(snippet["publishedAt"].toString().left(19)+"+00:00",Qt::ISODate);
                media.m_title = snippet["title"].toString();
                media.m_channel_id = snippet["channelId"].toString();
                QtJson::JsonObject thumbnails = snippet["thumbnails"].toMap();
                QtJson::JsonObject high = thumbnails["high"].toMap();
                if (!high.isEmpty()) media.m_image_url = QUrl(high["url"].toString());
                else {
                    QtJson::JsonObject medium = thumbnails["medium"].toMap();
                    if (!medium.isEmpty()) media.m_image_url = QUrl(medium["url"].toString());
                    else {
                        QtJson::JsonObject def = thumbnails["default"].toMap();
                        if (!def.isEmpty()) media.m_image_url = QUrl(def["url"].toString());
                    }
                }
                media.m_author = snippet["channelTitle"].toString();
                media.m_url = QUrl(QString(YOUTUBE_URL_FORMAT).arg(media.id()));
                media.m_comments_url = QUrl(QString(YOUTUBE_COMMENTS_SEARCH).arg(m_userKey).arg(media.id()));
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

void YouTubeSearch::download_categories(const QString & userKey,bool do_loop) {
    QString url = QString(YOUTUBE_VIDEO_CATEGORIES).arg(userKey);

    QEventLoop loop;
    QNetworkReply * m_reply = manager->get(QNetworkRequest(url));
    m_reply->ignoreSslErrors();
    connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(get_categories_was_error(QNetworkReply::NetworkError)));
    connect(m_reply,SIGNAL(finished()),this,SLOT(on_categories_finished()));
    if (do_loop) {
        connect(m_reply,SIGNAL(finished()),&loop,SLOT(quit()));
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }

}

void YouTubeSearch::get_categories_was_error(QNetworkReply::NetworkError err) {
    QNetworkReply * m_device = (QNetworkReply *)QObject::sender();
    if (err == QNetworkReply::ContentNotFoundError) return;
    int statusCode = m_device->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if ((statusCode == 400) || (statusCode == 403)) return;

    emit error(statusCode,m_device->errorString());
}

void YouTubeSearch::on_categories_finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

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
    m_reply->abort();
    m_reply->deleteLater();

    if (m_categories.isEmpty() && ((statusCode == 400) || (statusCode == 403))) {
        CreateNewYoutubeKeyDialog dlg(findMainWindow());
        if (dlg.exec() == QDialog::Rejected) {
            qApp->quit();
            return;
        }
        m_userKey=dlg.key();
        theSettings->setValue("youtube_user_key",m_userKey);
        download_categories(m_userKey);
        return;
    }
    else if (m_categories.isEmpty()) {
        qApp->quit();
        return;
    }

    m_categoryId = m_categories.indexOf(m_category);
    if (m_categoryId < 0) m_categoryId = 0;
    else m_categoryId++;

    if (!m_channel.isEmpty()) download_channel_id(m_channel);
    else search_again(m_pageToken);
}

QByteArray YouTubeSearch::downloadResource(const QUrl & url,QString & error) {
    QByteArray ret_data;
    QEventLoop loop;
    QNetworkReply * image_reply = manager->get(QNetworkRequest(url));
    image_reply->ignoreSslErrors();
    connect(image_reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    int statusCode = image_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode >= 200 && statusCode < 300) ret_data = image_reply->readAll();
    else error = image_reply->errorString();

    image_reply->abort();
    image_reply->deleteLater();

    return ret_data;
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
    QNetworkReply * image_reply = manager->get(QNetworkRequest(image_url));
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
        m_medias[resources_map[image_reply]].m_image = (data.isEmpty()?QImage(":/images/res/default.png"):QImage::fromData(data)).scaled(theSettings->value("preview_size",QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT)).toSize(),Qt::KeepAspectRatio);
    }
    else {
        if (statusCode == 404) {
            m_medias[resources_map[image_reply]].m_image = QImage(":/images/res/default.png").scaled(theSettings->value("preview_size",QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT)).toSize());
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
                QNetworkReply * reply = getDescriptionDownloadReply(QString(YOUTUBE_VIDEOINFO_SEARCH).arg(m_userKey).arg(m_medias[i].id()));
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
    QNetworkReply * desc_reply = manager->get(QNetworkRequest(url));
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
                m_medias[resources_map[desc_reply]].m_description = desc;
                m_medias[resources_map[desc_reply]].m_duration = convertYoutubeDuration(contentDetails["duration"].toString());
                m_medias[resources_map[desc_reply]].m_rating = ((likeCount-dislikeCount)*5.0)/(likeCount+dislikeCount);
                int categoryId = snippet["categoryId"].toInt()-1;
                if ((categoryId >= 0) && (m_categories.count() > categoryId)) m_medias[resources_map[desc_reply]].m_category = m_categories[categoryId];
                is_ok = true;
            }
            if (!is_ok) {
                m_medias[resources_map[desc_reply]].m_ignore = true;
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

