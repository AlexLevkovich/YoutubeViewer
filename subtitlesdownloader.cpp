#include "subtitlesdownloader.h"
#include "xmltosrtconverter.h"

SubtitlesDownloader::SubtitlesDownloader(const Subtitle & subtitle,const QString & out_file_name,QObject *parent) : DownloaderInterface(out_file_name,parent) {
    m_subtitle = subtitle;
    is_downloading = false;
}

void SubtitlesDownloader::terminate() {}

void SubtitlesDownloader::emit_progress(qreal bytes_downloaded,int percents) {
    emit progress(bytes_downloaded,0,percents,0);
}

void SubtitlesDownloader::emit_finished(const QString & err) {
    is_downloading = false;
    emit finished(err);
}

void SubtitlesDownloader::start() {
    is_downloading = true;
    QMetaObject::invokeMethod(this,"emit_progress",Qt::QueuedConnection,Q_ARG(qreal,0),Q_ARG(int,0));
    if (!XmlToSrtConverter::writeToSrtFile(m_subtitle.data(),outputFileName())) {
        QMetaObject::invokeMethod(this,"emit_finished",Qt::QueuedConnection,Q_ARG(QString,tr("Netwotk failure!!!")));
        return;
    }
    QMetaObject::invokeMethod(this,"emit_progress",Qt::QueuedConnection,Q_ARG(qreal,m_subtitle.data().count()),Q_ARG(int,100));
    QMetaObject::invokeMethod(this,"emit_finished",Qt::QueuedConnection,Q_ARG(QString,""));
}

bool SubtitlesDownloader::isDownloading() const {
    return is_downloading;
}
