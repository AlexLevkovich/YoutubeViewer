#ifndef YOUTUBEDOWNLOADER_H
#define YOUTUBEDOWNLOADER_H

#include <QObject>
#include <QUrl>
#include <QProcess>

class YoutubeDownloader : public QObject {
    Q_OBJECT
public:
    explicit YoutubeDownloader(const QUrl & url,const QString & out_file_name,int threads_count = 8,QObject *parent = 0);
    void terminate();

signals:
    void finished(const QString & err);
    void progress(qreal bytes_downloaded,qreal length,int percents,qreal speed);

private slots:
    void aria_finished(int code,QProcess::ExitStatus status);
    void aria_ready_read();

private:
    QProcess downloader;
    static const QRegExp progress_exp;
};

#endif // YOUTUBEDOWNLOADER_H
