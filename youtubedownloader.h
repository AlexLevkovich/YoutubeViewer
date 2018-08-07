#ifndef YOUTUBEDOWNLOADER_H
#define YOUTUBEDOWNLOADER_H

#include <QObject>
#include <QUrl>
#include "downloaderinterface.h"

class MultiDownloader;

class YoutubeDownloader : public DownloaderInterface {
    Q_OBJECT
public:
    explicit YoutubeDownloader(const QUrl & url,const QString & out_file_name,int threads_count = 8,QObject *parent = 0);
    ~YoutubeDownloader();
    void terminate();
    void start();
    bool isDownloading() const;

signals:
    void finished(const QString & err);
    void progress(qreal bytes_downloaded,qreal length,int percents,qreal speed);

private slots:
    void location_changed(const QUrl & url);
    void download_completed();
    void download_error();
    void download_progress(qint64 downloaded,int percents,qint64 speed);

private:
    MultiDownloader * downloader;
};

#endif // YOUTUBEDOWNLOADER_H
