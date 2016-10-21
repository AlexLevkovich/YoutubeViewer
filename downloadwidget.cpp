#include "downloadwidget.h"
#include "ui_downloadwidget.h"
#include "youtubedownloader.h"
#include "byteshumanizer.h"
#include <QMessageBox>
#include <QMainWindow>
#include <QDebug>

extern QMainWindow * findMainWindow();

DownloadWidget::DownloadWidget(const QUrl & url,const QString & out_file_name,int threads_count,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DownloadWidget) {
    ui->setupUi(this);
    int height = font().pointSize();
    ui->terminateButton->setIconSize(QSize(height,height));
    ui->errorButton->setIconSize(QSize(height,height));
    ui->errorButton->setVisible(false);
    ui->nameLabel->setText(out_file_name);

    is_terminated = false;
    is_paused = false;
    this->url = url;
    this->out_file_name = out_file_name;
    this->threads_count = threads_count;

    init_new_downloader();
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
    delete downloader;
    downloader = NULL;
}

void DownloadWidget::download_progress(qreal bytes_downloaded,qreal length,int percents,qreal speed) {
    ui->percentsLabel->setText(QString("%1%").arg(percents));
    ui->progressBar->setValue(percents);
    ui->infoLabel->setText(BytesHumanizer(bytes_downloaded).toString()+"/"+BytesHumanizer(length).toString()+", "+BytesHumanizer(speed).toString()+tr("/s"));
}

void DownloadWidget::on_errorButton_clicked() {
    QMessageBox::critical(findMainWindow(),tr("Error during downloading!"),lastError);
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
