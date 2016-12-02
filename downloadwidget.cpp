#include "downloadwidget.h"
#include "ui_downloadwidget.h"
#include "youtubedownloader.h"
#include "subtitlesdownloader.h"
#include "byteshumanizer.h"
#include "errordialog.h"
#include <QMainWindow>
#include <QDebug>

extern QMainWindow * findMainWindow();

DownloadWidget::DownloadWidget(const QUrl & url,const QString & out_file_name,int threads_count,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadWidget) {
    ui->setupUi(this);

    init();

    this->url = url;
    this->out_file_name = out_file_name;
    ui->nameLabel->setText(out_file_name);
    this->threads_count = threads_count;

    init_new_downloader();
}

DownloadWidget::DownloadWidget(const Subtitle & subtitle,const QString & out_file_name,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadWidget) {
    ui->setupUi(this);

    init();
    ui->stopStartButton->setVisible(false);
    ui->terminateButton->setVisible(false);

    this->subtitle = subtitle;
    this->out_file_name = out_file_name;
    ui->nameLabel->setText(out_file_name);

    init_new_subs_downloader();
}

void DownloadWidget::init() {
    int height = font().pointSize();
    ui->terminateButton->setIconSize(QSize(height,height));
    ui->errorButton->setIconSize(QSize(height,height));
    ui->errorButton->setVisible(false);

    is_terminated = false;
    is_paused = false;
}

DownloadWidget::~DownloadWidget() {
    delete ui;
}

void DownloadWidget::init_new_downloader() {
    downloader = new YoutubeDownloader(url,out_file_name,threads_count,this);
    connect(downloader,SIGNAL(finished(const QString &)),this,SLOT(download_finished(const QString &)));
    connect(downloader,SIGNAL(progress(qreal,qreal,int,qreal)),this,SLOT(download_progress(qreal,qreal,int,qreal)));
    downloader->start();
}

void DownloadWidget::init_new_subs_downloader() {
    downloader = new SubtitlesDownloader(subtitle,out_file_name,this);
    connect(downloader,SIGNAL(finished(const QString &)),this,SLOT(download_finished(const QString &)));
    connect(downloader,SIGNAL(progress(qreal,qreal,int,qreal)),this,SLOT(download_progress(qreal,qreal,int,qreal)));
    downloader->start();
}

void DownloadWidget::download_finished(const QString & err) {
    if ((err.isEmpty() || is_terminated) && !is_paused) {
        if (property("parent").isValid()) {
            ((QObject *)property("parent").value<void *>())->deleteLater();
            if (property("separator").isValid()) {
                ((QObject *)property("separator").value<void *>())->deleteLater();
            }
            return;
        }
    }
    if (!is_paused) {
        ui->errorButton->setVisible(true);
        updateGeometry();
    }
    lastError = err;
    is_paused = false;
    ui->stopStartButton->setIcon(QIcon(":/images/res/media-playback-start.png"));
    downloader->deleteLater();
    downloader = NULL;
}

void DownloadWidget::download_progress(qreal bytes_downloaded,qreal length,int percents,qreal speed) {
    ui->percentsLabel->setText(QString("%1%").arg(percents));
    ui->progressBar->setValue(percents);
    if (length > 0) {
        ui->infoLabel->setText(BytesHumanizer(bytes_downloaded).toString()+"/"+BytesHumanizer(length).toString()+", "+BytesHumanizer(speed).toString()+tr("/s"));
    }
    else {
        ui->infoLabel->setText(BytesHumanizer(bytes_downloaded).toString());
    }
}

void DownloadWidget::on_errorButton_clicked() {
    ErrorDialog(tr("Error during downloading!"),lastError,findMainWindow(),"YoutubeViewer").exec();
}

void DownloadWidget::on_terminateButton_clicked() {
    is_terminated = true;
    if (downloader != NULL) downloader->terminate();
    else download_finished("");
}

void DownloadWidget::on_stopStartButton_clicked() {
    ui->stopStartButton->setIcon((downloader != NULL)?QIcon(":/images/res/media-playback-start.png"):QIcon(":/images/res/media-playback-stop.png"));
    if (downloader == NULL) {
        lastError.clear();
        ui->errorButton->setVisible(false);
        updateGeometry();
        init_new_downloader();
    }
    else {
        is_paused = true;
        downloader->terminate();
    }
}

bool DownloadWidget::isDownloading() const {
    return (downloader != NULL) && downloader->isDownloading();
}
