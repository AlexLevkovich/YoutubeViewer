#include "youtubedownloader.h"
#include "byteshumanizer.h"
#include "default_values.h"
#include <QFileInfo>
#include <QDir>

extern QString TOOLS_BIN_PATH;

YoutubeDownloader::YoutubeDownloader(const QUrl & url,const QString & out_file_name,int threads_count,QObject *parent) : QObject(parent) {
    QString file_name = QFileInfo(out_file_name).fileName();
    QString dir_name = QFileInfo(out_file_name).dir().path();
    downloader.setEnvironment(downloader.systemEnvironment() << "LANG=C");
    connect(&downloader,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(aria_finished(int,QProcess::ExitStatus)));
    connect(&downloader,SIGNAL(readyReadStandardOutput()),SLOT(aria_ready_read()));
#ifndef WIN32
    downloader.start(QString("%1/stdbuf -i0 -o0 -e0 %1/aria2c --check-certificate=false --summary-interval=1 -c -j %2 -x %2 -s %2 -k 1M ").arg(TOOLS_BIN_PATH).arg(threads_count)+"\""+url.toString()+"\" -o \""+file_name+"\" --dir=\""+dir_name+"\"");
#else
    downloader.start(QString("%1/aria2c --check-certificate=false --summary-interval=1 -c -j %2 -x %2 -s %2 -k 1M ").arg(TOOLS_BIN_PATH).arg(threads_count)+"\""+url.toString()+"\" -o \""+file_name+"\" --dir=\""+dir_name+"\"");
#endif
}

void YoutubeDownloader::aria_finished(int code,QProcess::ExitStatus /*status*/) {
    if (code == 0) aria_ready_read();
    emit finished((code == 0)?"":downloader.readAllStandardError());
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
    downloader.terminate();
}
