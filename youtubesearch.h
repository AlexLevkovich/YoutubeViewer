#ifndef YOUTUBESEARCH_H
#define YOUTUBESEARCH_H

#include <QObject>
#include <QtNetwork>
#include <QDateTime>
#include <QImage>
#include <QMap>

#define MAX_QUERY_COUNT 50
#define YOUTUBE_API "https://www.googleapis.com/youtube/v3/search?part=snippet&order=%1&type=video&key=%2&q=%3&maxResults=%4"
#define YOUTUBE_COMMENT_API "https://www.googleapis.com/youtube/v3/commentThreads?part=snippet&key=%1&videoId=%2"
#define YOUTUBE_VIDEO "https://www.googleapis.com/youtube/v3/videos?part=status,contentDetails,statistics,snippet&key=%1&id=%2"
#define YOUTUBE_VIDEO_CATEGORIES "https://www.googleapis.com/youtube/v3/videoCategories?part=snippet&key=%1&regionCode=US"
#define YOUTUBE_VIDEO_URLS_PROCESS "%2/youtube-dl --skip-download --get-format -g --all-formats https://www.youtube.com/watch?v=%1"
#define CATEGORY_PART "&videoCategoryId=%1"
#define AUTHOR_PART "&forContentOwner=true&onBehalfOfContentOwner=%1"
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

struct FmtQuality {
    QString quality;
    int id;

    inline FmtQuality() {}

    inline FmtQuality(QString quality, int id) {
        this->quality = quality;
        this->id = id;
    }

    inline FmtQuality(QString str) {
        QStringList parts = str.split(" - ");
        if (parts.count() == 2) {
            quality = parts[1];
            id = parts[0].toInt();
        }
    }

    inline QString toString() const {
        return QString("%1 (%2)").arg(quality).arg(id);
    }
};

struct VideoInfo {
    QUrl url;
    FmtQuality quality;

    inline VideoInfo() {}
};

enum YoutubeTimeId {
    before,
    after
};

class YoutubeTime {
public:

    YoutubeTime() {}
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

private:
    YoutubeTimeId m_operation;
    QDateTime m_date;
};

class YouTubeSearch;

class Media {
    friend class YouTubeSearch;
public:
    inline Media() { m_ignore = false; }

    inline QString & id() { return m_id; }
    inline QString & title() { return m_title; }
    inline QString & description() { return m_description; }
    inline QImage & image() { return m_image; }
    inline QUrl & image_url() { return m_image_url; }
    inline QUrl & url() { return m_url; }
    inline QDateTime & date() { return m_date; }
    inline QString & duration() { return m_duration; }
    inline QString & author() { return m_author; }
    inline double & rating() { return m_rating; }
    inline QString & category() { return m_category; }
    inline QUrl & comments_url() { return m_comments_url; }
    inline QList<VideoInfo> & video_infos() { return m_video_infos; }
    inline bool & doIgnore() { return m_ignore; }

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
    QString m_author;
    double m_rating;
    QString m_category;
    QUrl m_comments_url;
    QList<VideoInfo> m_video_infos;
    bool m_ignore;
};

class YouTubeSearch : public QObject {
    Q_OBJECT

public:
    explicit YouTubeSearch(QObject *parent = 0);
    ~YouTubeSearch();

public slots:
    bool download_categories(const QString & userKey);
    static const QStringList categories();
    bool search(const QString & userKey,
                const QString & query,
                const QString & category = QString(),
                const QString & author = QString(),
                YoutubeOrderBy orderby = relevance,
                const YoutubeTime & time = YoutubeTime(),
                const QString & pageToken = QString());
    bool search_again(const QString & pageToken);
    bool is_in_processing();
    void terminate_processing(QIODevice * ignore_device = NULL);
    inline QString prevPageToken() const { return m_prevPageToken; }
    inline QString nextPageToken() const { return m_nextPageToken; }
    inline bool hasNextPage() { return !m_nextPageToken.isEmpty(); }
    inline bool hasPrevPage() { return !m_prevPageToken.isEmpty(); }
    static const QString orderByParameterString(YoutubeOrderBy id);

signals:
    void error(int code,const QString & error);
    void completed(const QList<Media> & medias);
    void categories_completed();

private slots:
    void was_error(QNetworkReply::NetworkError error);
    void finished();
    void image_finished();
    void desc_finished();
    void categories_finished();

private:
    QNetworkReply * getImageDownloadReply(const QUrl & image_url);
    QNetworkReply * getDescriptionDownloadReply(const QUrl & url);
    void start_download_previews();
    void start_download_full_descriptions();

    QNetworkAccessManager manager;
    QString m_userKey;
    QString m_query;
    QList<Media> m_medias;
    int m_categoryId;
    QString m_author;
    YoutubeOrderBy m_orderby;
    YoutubeTime m_time;
    int m_thread_count;
    QMap<QNetworkReply *,int> resources_map;
    bool is_processing;
    QString m_nextPageToken;
    QString m_prevPageToken;
    static QStringList m_categories;
};

#endif // YOUTUBESEARCH_H
