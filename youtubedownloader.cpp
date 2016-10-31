#include "youtubedownloader.h"
#include "byteshumanizer.h"
#include "default_values.h"
#include <QFileInfo>
#include <QDir>

extern QString TOOLS_BIN_PATH;

YoutubeDownloader::YoutubeDownloader(const QUrl & url,const QString & out_file_name,int threads_count,QObject *parent) : QObject(parent) {
    m_threads_count = threads_count;
    m_url = url;
    is_working = false;

    m_file_name = QFileInfo(out_file_name).fileName();
    m_dir_name = QFileInfo(out_file_name).dir().path();
    downloader.setEnvironment(downloader.systemEnvironment() << "LANG=C");
    connect(&downloader,SIGNAL(started()),this,SLOT(aria_started()));
    connect(&downloader,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(aria_finished(int,QProcess::ExitStatus)));
    connect(&downloader,SIGNAL(readyReadStandardOutput()),SLOT(aria_ready_read()));
#if QT_VERSION >= 0x050000
    connect(&downloader,SIGNAL(errorOccurred(QProcess::ProcessError)),SLOT(aria_error()));
#else
    connect(&downloader,SIGNAL(error(QProcess::ProcessError)),SLOT(aria_error()));
#endif
}

void YoutubeDownloader::aria_started() {
    is_working = true;
}

void YoutubeDownloader::start() {
#ifndef WIN32
    downloader.start(QString("%1/stdbuf -i0 -o0 -e0 %1/aria2c --check-certificate=false --summary-interval=1 -c -j %2 -x %2 -s %2 -k 1M ").arg(TOOLS_BIN_PATH).arg(m_threads_count)+"\""+m_url.toString()+"\" -o \""+m_file_name+"\" --dir=\""+m_dir_name+"\"");
#else
    downloader.start(QString("%1/aria2c --check-certificate=false --summary-interval=1 -c -j %2 -x %2 -s %2 -k 1M ").arg(TOOLS_BIN_PATH).arg(m_threads_count)+"\""+m_url.toString()+"\" -o \""+m_file_name+"\" --dir=\""+m_dir_name+"\"");
#endif
}

void YoutubeDownloader::aria_error() {
    is_working = false;
    emit finished(tr("Cannot start %1/aria2c!!!").arg(TOOLS_BIN_PATH));
}

void YoutubeDownloader::aria_finished(int code,QProcess::ExitStatus /*status*/) {
    if (code == 0) aria_ready_read();
    QString error = downloader.readAllStandardError();
    if (error.isEmpty() && (code != 0)) error = tr("Unknown aria's error!!!");
    is_working = false;
    emit finished((code == 0)?"":error);
}

void YoutubeDownloader::aria_ready_read() {
    const QStringList output = QString (downloader.readAllStandardOutput()).split('\n');

    foreach (QString line,output) {
        if (line.startsWith("[") && line.endsWith("]")) {
            QStringList parms = line.split(" ");
            if (parms.count() < 5) continue;
            QStringList _progress = parms[1].split("/");
            if (_progress.count() < 2) continue;
            QString downloaded = _progress[0];
            _progress = _progress[1].split("(");
            if (_progress.count() < 2) continue;
            QString length = _progress[0];
            QString percents = _progress[1].left(_progress[1].length()-2);
            QStringList speeds = parms[3].split(":");
            if (speeds.count() < 2) continue;
            QString speed = speeds[1];
            emit progress(BytesHumanizer(downloaded).value(),BytesHumanizer(length).value(),percents.toInt(),BytesHumanizer(speed).value());
        }
    }
}

void YoutubeDownloader::terminate() {
    downloader.kill();
}

bool YoutubeDownloader::isDownloading() const {
    return is_working;
}
