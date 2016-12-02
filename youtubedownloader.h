#ifndef YOUTUBEDOWNLOADER_H
#define YOUTUBEDOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QProcess>
#include "downloaderinterface.h"

class YoutubeDownloader : public DownloaderInterface {
    Q_OBJECT
public:
    explicit YoutubeDownloader(const QUrl & url,const QString & out_file_name,int threads_count = 8,QObject *parent = 0);
    void terminate();
    void start();
    bool isDownloading() const;

signals:
    void finished(const QString & err);
    void progress(qreal bytes_downloaded,qreal length,int percents,qreal speed);

private slots:
    void aria_finished(int code,QProcess::ExitStatus status);
    void aria_ready_read();
    void aria_error();
    void aria_started();

private:
    QProcess downloader;
    static const QRegExp progress_exp;
    int m_threads_count;
    QString m_file_name;
    QString m_dir_name;
    QUrl m_url;
    bool is_working;
    QString error;
    QByteArray output_remain;
};

#endif // YOUTUBEDOWNLOADER_H
