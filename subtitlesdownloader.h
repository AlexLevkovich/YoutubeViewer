#ifndef SUBTITLESDOWNLOADER_H
#define SUBTITLESDOWNLOADER_H

#include "downloaderinterface.h"
#include "youtubesearch.h"

class SubtitlesDownloader : public DownloaderInterface {
    Q_OBJECT
public:
    SubtitlesDownloader(const Subtitle & subtitle,const QString & out_file_name,QObject *parent = 0);
    void terminate();
    void start();
    bool isDownloading() const;

private slots:
    void emit_progress(qreal bytes_downloaded,int percents);
    void emit_finished(const QString & err);

signals:
    void finished(const QString & err);
    void progress(qreal bytes_downloaded,qreal length,int percents,qreal speed);

private:
    Subtitle m_subtitle;
    bool is_downloading;
};

#endif // SUBTITLESDOWNLOADER_H
