#ifndef YOUTUBESEARCH_H
#define YOUTUBESEARCH_H

#include <QObject>
#include <QtNetwork>
#include <QDateTime>
#include <QByteArray>
#include <QImage>
#include <QMap>
#include <QList>
#include "json.h"

extern QDateTime MINIMUM_DATE;

#define MAX_QUERY_COUNT 50
#define YOUTUBE_VIDEOLIST_SEARCH "https://www.googleapis.com/youtube/v3/search?part=snippet&type=video&key=%1&maxResults=%2"
#define YOUTUBE_CHANNELLIST_SEARCH "https://www.googleapis.com/youtube/v3/search?part=snippet&type=channel&key=%1&maxResults=1"
#define YOUTUBE_COMMENTS_SEARCH "https://www.googleapis.com/youtube/v3/commentThreads?part=snippet&key=%1&videoId=%2"
#define YOUTUBE_VIDEOINFO_SEARCH "https://www.googleapis.com/youtube/v3/videos?part=status,contentDetails,statistics,snippet&key=%1&id=%2"
#define YOUTUBE_VIDEO_CATEGORIES "https://www.googleapis.com/youtube/v3/videoCategories?part=snippet&key=%1&regionCode=US"
#define YOUTUBE_PLAYLIST_SEARCH "https://www.googleapis.com/youtube/v3/search?part=snippet&type=playlist&key=%1&channelId=%2&maxResults=%3"
#define YOUTUBE_PLAYLISTITEMS_SEARCH "https://www.googleapis.com/youtube/v3/playlistItems?part=snippet&key=%1&playlistId=%2&maxResults=%3"
#define YOUTUBE_SUBLISTITEMS_SEARCH "https://www.youtube.com/api/timedtext?key=%1&v=%2&asrs=1&type=list&tlangs=1"
#define YOUTUBE_SUBITEM_URL "https://www.youtube.com/api/timedtext?key=%1&v=%2&type=track&lang=%3"
#define YOUTUBE_VIDEO_URLS_PROCESS "%2 --no-check-certificate --skip-download --print-json --no-warnings %1"
#define CATEGORY_PART "&videoCategoryId=%1"
#define ORDER_PART "&order=%1"
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
    int m_width;
    int m_height;

    inline FmtDesc() {
        m_width = 0;
        m_height = 0;
        m_bitrate = 0;
        m_id = -1;
    }

    inline FmtDesc(QtJson::JsonObject format) {
        m_id = format["format_id"].toInt();
        if (format["width"].isNull() || format["height"].isNull()) {
            m_width = 0;
            m_height = 0;
            m_resolution = QObject::tr("Audio only");
        }
        else {
            m_width = format["width"].toInt();
            m_height = format["height"].toInt();
            m_resolution = QString("%1x%2").arg(m_width).arg(m_height);
        }
        m_vcodec = format["vcodec"].toString();
        if (m_vcodec == "none") m_vcodec = QString();
        m_acodec = format["acodec"].toString();
        if (m_acodec == "none") m_acodec = QString();
        m_bitrate = format["tbr"].toInt();
        m_description = format["format_note"].toString();
    }

public:
    inline bool isValid() const {
        return (!m_vcodec.isEmpty() || !m_acodec.isEmpty()) && (m_id >= 0);
    }

    inline QString resolution() const {
        return m_resolution;
    }

    inline QString description() const {
        return m_description;
    }

    inline QString acodec() const {
        return m_acodec;
    }

    inline QString vcodec() const {
        return m_vcodec;
    }

    inline int width() const {
        return m_width;
    }

    inline int height() const {
        return m_height;
    }

    inline int bitrate() const {
        return m_bitrate;
    }

    inline int id() const {
        return m_id;
    }

    inline QString toString() const {
        return QString("%1 (%2%3)").arg(m_resolution).arg((m_vcodec.isEmpty()?(m_acodec):m_vcodec)).arg(m_vcodec.isEmpty()?QString(", %1 Kbps").arg(m_bitrate):"");
    }

    inline QString toStringFull() const {
        return QString("%1 (%2%3)").arg(m_resolution).arg((m_vcodec.isEmpty()?(m_acodec):m_vcodec)).arg(m_vcodec.isEmpty()?QString(", %1 Kbps").arg(m_bitrate):(m_acodec.isEmpty()?" ,"+QObject::tr("No audio"):""));
    }

    inline bool isAudioOnly() {
        return (m_vcodec.isEmpty());
    }

    inline bool hasExternalAudio() {
        return (m_acodec.isEmpty() && !m_vcodec.isEmpty());
    }

    friend class VideoInfo;
    friend class Media;
};

class Subtitle {
private:
    QUrl m_url;
    QString m_lang_code;
    QString m_lang_original;
    QByteArray m_data;

    void download_data() const;
    inline QUrl url() const {
        return m_url;
    }
public:
    Subtitle() {}
    inline bool isValid() const {
        return (m_url.isValid() && !m_lang_code.isEmpty() && !m_lang_original.isEmpty());
    }
    inline QString langCode() const {
        return m_lang_code;
    }
    inline QString langOriginal() const {
        return m_lang_original;
    }
    inline QByteArray data() const {
        if (m_data.isEmpty()) download_data();
        return m_data;
    }
    inline bool downloaded() const { return !m_data.isEmpty(); }
    inline QString toString() const {
        return m_lang_original;
    }

    friend class Media;
};

Q_DECLARE_METATYPE(Subtitle)


class VideoInfo {
private:
    QUrl m_url;
    QUrl m_audio_url;
    FmtDesc m_desc;
    QString m_filename;

    inline VideoInfo() {}

public:
    inline bool isValid() const {
        return (m_url.isValid() || m_audio_url.isValid()) && m_desc.isValid();
    }

    inline QUrl url() const {
        return m_url;
    }

    inline QUrl audio_url() const {
        return m_audio_url;
    }

    inline FmtDesc desc() const {
        return m_desc;
    }

    inline QString filename() const {
        return m_filename;
    }

    inline bool isAudioOnly() {
        return m_desc.isAudioOnly();
    }
    inline bool hasExternalAudio() {
        return m_desc.hasExternalAudio();
    }

    friend class Media;
};

enum YoutubeTimeId {
    before,
    after
};

class YoutubeTime {
public:

    inline YoutubeTime() {
        m_operation = after;
        m_date = MINIMUM_DATE;
    }
    inline YoutubeTime(YoutubeTimeId operation,const QDateTime & date) {
        m_operation = operation;
        m_date = date;
    }

    inline QString toString() const {
        YoutubeTime * p_this = (YoutubeTime *)this;
        switch (p_this->m_operation) {
            case before:
                return QString("&publishedBefore=%1").arg(p_this->m_date.toString(Qt::ISODate).left(19)+"Z");
            case after:
                return QString("&publishedAfter=%1").arg(p_this->m_date.toString(Qt::ISODate).left(19)+"Z");
        }
        return QString();
    }

    inline YoutubeTimeId operation() { return m_operation; }
    inline QDateTime date() const { return m_date; }
    inline bool isNull() { return m_date.isNull(); }
    static const QString timeParameterString(YoutubeTimeId id);

    inline bool operator==(const YoutubeTime & other) const {
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
    inline QList<Subtitle> subtitles() const { return m_subtitles; }
    QList<QByteArray> subtitlesData() const;
    inline QList<VideoInfo> video_infos() const {
        ((Media *)this)->download_video_infos();
        return m_video_infos;
    }
    VideoInfo best_video() const;
    VideoInfo video(int height,const QString & vcodec) const;
private:
    inline bool doIgnore() const { return m_ignore; }
    void download_video_infos();
    void download_subtitles_list();
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
    QList<Subtitle> m_subtitles;
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
    inline YPlayList() {}

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
    static QList<YPlayList> downloadChannelPlaylists(const QString & channel_id,const QString & userKey,YoutubeOrderBy orderby,QString & error);
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
    void get_categories_was_error(QNetworkReply::NetworkError error);
    void finished();
    void image_finished();
    void desc_finished();
    void on_categories_finished();
    void on_channel_id_finished();

private:
    static QString parseChannelId(const QByteArray & data,const QString & channel_name,QString & error);
    static QByteArray downloadResource(const QUrl & url,QString & error);
    static QList<YPlayList> downloadChannelPlaylists(const QString & channel_id,const QString & userKey,QString & error,const QString & pageToken,YoutubeOrderBy orderby = relevance);
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
    friend class Media;
    friend class Subtitle;
};

#endif // YOUTUBESEARCH_H
