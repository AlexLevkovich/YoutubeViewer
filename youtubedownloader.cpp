#include "youtubedownloader.h"
#include "byteshumanizer.h"
#include "default_values.h"
#include <QFileInfo>
#include <QDir>
#include <QRegExp>
#include <QBuffer>

extern QString TOOLS_BIN_PATH;

YoutubeDownloader::YoutubeDownloader(const QUrl & url,const QString & out_file_name,int threads_count,QObject *parent) : DownloaderInterface(out_file_name,parent) {
    m_threads_count = threads_count;
    m_url = url;
    is_working = false;

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
    m_file_name = QFileInfo(outputFileName()).fileName();
    m_dir_name = QFileInfo(outputFileName()).dir().path();
#ifndef WIN32
    downloader.start(QString("%1/stdbuf -i0 -o0 -e0 %1/aria2c --async-dns=false --enable-color=false --check-certificate=false --summary-interval=1 -c -j %2 -x %2 -s %2 -k 1M ").arg(TOOLS_BIN_PATH).arg(m_threads_count)+"\""+m_url.toString()+"\" -o \""+m_file_name+"\" --dir=\""+m_dir_name+"\"");
#else
    downloader.start(QString("%1/aria2c --async-dns=false --enable-color=false --check-certificate=false --summary-interval=1 -c -j %2 -x %2 -s %2 -k 1M ").arg(TOOLS_BIN_PATH).arg(m_threads_count)+"\""+m_url.toString()+"\" -o \""+m_file_name+"\" --dir=\""+m_dir_name+"\"");
#endif
}

void YoutubeDownloader::aria_error() {
    is_working = false;
    emit finished(tr("Cannot start %1/aria2c!!!").arg(TOOLS_BIN_PATH));
}

void YoutubeDownloader::aria_finished(int code,QProcess::ExitStatus /*status*/) {
    if (code == 0) aria_ready_read();
    else {
        error = error + QString::fromLocal8Bit(downloader.readAllStandardError());
        if (error.isEmpty() && (code != 0)) error = tr("Unknown aria's error!!!");
    }
    is_working = false;
    emit finished((code == 0)?"":error);
}

void YoutubeDownloader::aria_ready_read() {
    QByteArray output = output_remain + downloader.readAllStandardOutput();
    output_remain.clear();

    QString line;
    QByteArray line_arr;
    QBuffer output_buff(&output);
    output_buff.open(QIODevice::ReadOnly);
    while (!output_buff.atEnd()) {
        line_arr = output_buff.readLine();
        if (!line_arr.endsWith('\n') && output_buff.atEnd()) {
            output_remain += line_arr;
            break;
        }

        line = QString::fromLocal8Bit(!line_arr.endsWith('\n')?line_arr:line_arr.left(line_arr.length()-1));

        QRegExp date_reg("(\\d{1,2})/(\\d{2}) (\\d{1,2}):(\\d{2}):(\\d{2})");
        if (((date_reg.indexIn(line) >= 0) && line.contains(" [ERROR] ")) ||
            (line.contains("  -> ") && line.contains(" errorCode="))) {
            error += line + '\n';
            continue;
        }

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
