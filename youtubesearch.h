#ifndef YOUTUBESEARCH_H
#define YOUTUBESEARCH_H

#include <QObject>
#include <QtNetwork>
#include <QDateTime>
#include <QByteArray>
#include <QImage>
#include <QMap>
#include "json.h"

extern QDateTime MINIMUM_DATE;

#define MAX_QUERY_COUNT 50
#define YOUTUBE_VIDEOLIST_SEARCH "https://www.googleapis.com/youtube/v3/search?part=snippet&order=%1&type=video&key=%2&maxResults=%3"
#define YOUTUBE_CHANNELLIST_SEARCH "https://www.googleapis.com/youtube/v3/search?part=snippet&type=channel&key=%1&maxResults=1"
#define YOUTUBE_COMMENTS_SEARCH "https://www.googleapis.com/youtube/v3/commentThreads?part=snippet&key=%1&videoId=%2"
#define YOUTUBE_VIDEOINFO_SEARCH "https://www.googleapis.com/youtube/v3/videos?part=status,contentDetails,statistics,snippet&key=%1&id=%2"
#define YOUTUBE_VIDEO_CATEGORIES "https://www.googleapis.com/youtube/v3/videoCategories?part=snippet&key=%1&regionCode=US"
#define YOUTUBE_PLAYLIST_SEARCH "https://www.googleapis.com/youtube/v3/search?part=snippet&type=playlist&key=%1&channelId=%2&maxResults=%3"
#define YOUTUBE_PLAYLISTITEMS_SEARCH "https://www.googleapis.com/youtube/v3/playlistItems?part=snippet&key=%1&playlistId=%2&maxResults=%3"
#define YOUTUBE_VIDEO_URLS_PROCESS "%2/youtube-dl --skip-download --print-json %1"
#define CATEGORY_PART "&videoCategoryId=%1"
#define QUERY_PART "&q=%1"
#define AUTHOR_PART "&channelId=%1"
#define PAGE_TOKEN_PART "&pageToken=%1"
#define YOUTUBE_URL_FORMAT "https://youtu.be/%1"

enum YoutubeOrderBy {
    relevance = 0,
    date,
    rating,
    title,
    videoCount,
    viewCount
};

const QString orderby_values_en[6] = {"relevance",
                                      "date",
                                      "rating",
                                      "title",
                                      "videoCount",
                                      "viewCount"};

class FmtDesc {
private:
    QString m_resolution;
    QString m_description;
    QString m_acodec;
    QString m_vcodec;
    int m_bitrate;
    int m_id;

    inline FmtDesc() {}
    inline FmtDesc(QtJson::JsonObject format) {
        m_id = format["format_id"].toInt();
        if (format["width"].isNull() || format["height"].isNull()) m_resolution = QObject::tr("Audio only");
        else m_resolution = format["width"].toString() + "x" + format["height"].toString();
        m_vcodec = format["vcodec"].toString();
        if (m_vcodec == "none") m_vcodec = QString();
        m_acodec = format["acodec"].toString();
        if (m_acodec == "none") m_acodec = QString();
        m_bitrate = format["tbr"].toInt();
        m_description = format["format_note"].toString();
    }

public:
    QString resolution() const {
        return m_resolution;
    }

    QString description() const {
        return m_description;
    }

    QString acodec() const {
        return m_acodec;
    }

    QString vcodec() const {
        return m_vcodec;
    }

    int bitrate() const {
        return m_bitrate;
    }

    int id() const {
        return m_id;
    }

    inline QString toString() const {
        return QString("%1 (%2%3)").arg(m_resolution).arg((m_vcodec.isEmpty()?(m_acodec):m_vcodec)).arg(m_vcodec.isEmpty()?QString(", %1 Kbps").arg(m_bitrate):"");
    }

    inline QString toStringFull() const {
        return QString("%1 (%2%3)").arg(m_resolution).arg((m_vcodec.isEmpty()?(m_acodec):m_vcodec)).arg(m_vcodec.isEmpty()?QString(", %1 Kbps").arg(m_bitrate):(m_acodec.isEmpty()?" ,"+QObject::tr("No audio"):""));
    }

    friend class VideoInfo;
    friend class Media;
};

class VideoInfo {
private:
    QUrl m_url;
    QUrl m_audio_url;
    FmtDesc m_desc;
    QString m_filename;

    inline VideoInfo() {}

public:
    QUrl url() const {
        return m_url;
    }

    QUrl audio_url() const {
        return m_audio_url;
    }

    FmtDesc desc() const {
        return m_desc;
    }

    QString filename() const {
        return m_filename;
    }

    bool isAudioOnly() {
        return (m_desc.m_vcodec.isEmpty());
    }
    bool hasExternalAudio() {
        return (m_desc.m_acodec.isEmpty() && !m_desc.m_vcodec.isEmpty());
    }

    friend class Media;
};

enum YoutubeTimeId {
    before,
    after
};

class YoutubeTime {
public:

    YoutubeTime() {
        m_operation = after;
        m_date = MINIMUM_DATE;
    }
    YoutubeTime(YoutubeTimeId operation,const QDateTime & date) {
        m_operation = operation;
        m_date = date;
    }

    QString toString() const {
        YoutubeTime * p_this = (YoutubeTime *)this;
        switch (p_this->m_operation) {
            case before:
                return QString("&publishedBefore=%1").arg(p_this->m_date.toString(Qt::ISODate).left(19)+"Z");
            case after:
                return QString("&publishedAfter=%1").arg(p_this->m_date.toString(Qt::ISODate).left(19)+"Z");
        }
        return QString();
    }

    YoutubeTimeId operation() { return m_operation; }
    QDateTime date() const { return m_date; }
    bool isNull() { return m_date.isNull(); }
    static const QString timeParameterString(YoutubeTimeId id);

    bool operator==(const YoutubeTime & other) const {
        return m_operation == other.m_operation &&
               m_date == other.m_date;
    }

private:
    YoutubeTimeId m_operation;
    QDateTime m_date;
};

class YouTubeSearch;

class Media {
    friend class YouTubeSearch;
public:
    inline Media() { m_ignore = false; }

    inline QString id() const { return m_id; }
    inline QString title() const { return m_title; }
    inline QString description() const { return m_description; }
    inline QImage image() const { return m_image; }
    inline QUrl image_url() const { return m_image_url; }
    inline QUrl url() const { return m_url; }
    inline QDateTime date() const { return m_date; }
    inline QString duration() const { return m_duration; }
    inline QString author() const { return m_author; }
    inline QString channel_id() const { return m_channel_id; }
    inline double rating() const { return m_rating; }
    inline QString category() const { return m_category; }
    inline QUrl comments_url() const { return m_comments_url; }
    inline QList<VideoInfo> video_infos() const { return m_video_infos; }
    inline bool doIgnore() const { return m_ignore; }

    VideoInfo best_video() const;
    void download_video_infos();
private:
    void sort_video_urls_by_quality();
    static void download_video_categories();
    static bool sort_by_quality(const QUrl & url1, const QUrl & url2);
    static int compare_id_by_quality(const void *m1, const void *m2);
    static const QString findUrlParam(const QUrl & youtube_link,const QString & key);

    QString m_id;
    QString m_title;
    QString m_description;
    QUrl m_image_url;
    QImage m_image;
    QUrl m_url;
    QDateTime m_date;
    QString m_duration;
    QString m_channel_id;
    QString m_author;
    double m_rating;
    QString m_category;
    QUrl m_comments_url;
    QList<VideoInfo> m_video_infos;
    bool m_ignore;
};

class YPlayList {
public:
    inline QString title() const {
        return m_title;
    }
    inline QString id() const {
        return m_id;
    }
    inline QUrl image_url() const {
        return m_img_url;
    }
    QImage image() const;
private:
    YPlayList() {}

    QString m_id;
    QString m_title;
    QUrl m_img_url;
    QImage m_img;

    friend class YouTubeSearch;
};

class YouTubeSearch : public QObject {
    Q_OBJECT

public:
    explicit YouTubeSearch(QObject *parent = 0);
    ~YouTubeSearch();
    static const QStringList categories();
    static const QString orderByParameterString(YoutubeOrderBy id);
    static QString downloadChannelId(const QString & channel_name,const QString & userKey,QString & error);
    static QList<YPlayList> downloadChannelPlaylists(const QString & channel_id,const QString & userKey,QString & error);
    inline bool hasNextPage() { return !m_nextPageToken.isEmpty(); }
    inline bool hasPrevPage() { return !m_prevPageToken.isEmpty(); }
    inline QString prevPageToken() const { return m_prevPageToken; }
    inline QString nextPageToken() const { return m_nextPageToken; }
    bool is_in_processing();
    void terminate_processing(QIODevice * ignore_device = NULL);

public slots:
    bool search(const QString & userKey,
                const QString & query,
                const QString & category = QString(),
                const QString & channel = QString(),
                const QString & playlist_id = QString(),
                YoutubeOrderBy orderby = relevance,
                const YoutubeTime & time = YoutubeTime(),
                const QString & pageToken = QString());
    bool search_again(const QString & pageToken);

signals:
    void error(int code,const QString & error);
    void completed(const QList<Media> & medias);

private slots:
    void was_error(QNetworkReply::NetworkError error);
    void get_channel_id_was_error(QNetworkReply::NetworkError error);
    void get_categories_was_error(QNetworkReply::NetworkError error);
    void finished();
    void image_finished();
    void desc_finished();
    void on_categories_finished();
    void on_channel_id_finished();

private:
    static QByteArray downloadResource(const QUrl & url,QString & error);
    static QList<YPlayList> downloadChannelPlaylists(const QString & channel_id,const QString & userKey,QString & error,const QString & pageToken);
    void download_channel_id(const QString & channel_name);
    void download_categories(const QString & userKey,bool do_loop = false);
    QNetworkReply * getImageDownloadReply(const QUrl & image_url);
    QNetworkReply * getDescriptionDownloadReply(const QUrl & url);
    void start_download_previews();
    void start_download_full_descriptions();

    QString m_userKey;
    QString m_query;
    QString m_channel_id;
    QList<Media> m_medias;
    int m_categoryId;
    QString m_category;
    QString m_pageToken;
    QString m_channel;
    QString m_playlist_id;
    YoutubeOrderBy m_orderby;
    YoutubeTime m_time;
    int m_thread_count;
    QMap<QNetworkReply *,int> resources_map;
    bool is_processing;
    QString m_nextPageToken;
    QString m_prevPageToken;
    static QStringList m_categories;
    static QNetworkAccessManager * manager;

    friend class YPlayList;
};

#endif // YOUTUBESEARCH_H
