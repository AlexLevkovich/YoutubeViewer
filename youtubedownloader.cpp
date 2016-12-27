#include "youtubedownloader.h"
#include "byteshumanizer.h"
#include "default_values.h"
#include <QFileInfo>
#include <QFile>
#include "multidownloader.h"


YoutubeDownloader::YoutubeDownloader(const QUrl & url,const QString & out_file_name,int threads_count,QObject *parent) : DownloaderInterface(out_file_name,parent) {
    downloader = new MultiDownloader(url,threads_count,out_file_name);
    connect(downloader,SIGNAL(download_completed()),this,SLOT(download_completed()));
    connect(downloader,SIGNAL(progress(qint64,int,qint64)),SLOT(download_progress(qint64,int,qint64)));
    connect(downloader,SIGNAL(error_occured()),SLOT(download_error()));
}

YoutubeDownloader::~YoutubeDownloader() {
    downloader->quit();
    downloader->wait();
    delete downloader;
}

void YoutubeDownloader::download_progress(qint64 downloaded,int percents,qint64 speed) {
    emit progress(BytesHumanizer(downloaded).value(),BytesHumanizer(downloader->dataLength()).value(),percents,BytesHumanizer(speed).value());
}

void YoutubeDownloader::start() {
    QString savePath = downloader->outputName()+".save";
    if (QFileInfo(savePath).exists()) {
        if (!downloader->continueSaved(savePath)) emit finished(tr("Problem during starting downloader"));
    }
    else if (!downloader->start()) emit finished(tr("Problem during starting downloader"));
}

void YoutubeDownloader::download_error() {
    downloader->saveState(downloader->outputName()+".save");
    emit finished(downloader->errorString());
}

void YoutubeDownloader::download_completed() {
    emit finished("");
    QFile(downloader->outputName()+".save").remove();
}

void YoutubeDownloader::terminate() {
    downloader->terminate();
    downloader->saveState(downloader->outputName()+".save");
}

bool YoutubeDownloader::isDownloading() const {
    return downloader->isStarted();
}
