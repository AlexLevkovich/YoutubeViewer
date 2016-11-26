#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>
#include <QUrl>
#include "youtubesearch.h"

namespace Ui {
class DownloadWidget;
}

class DownloaderInterface;

class DownloadWidget : public QWidget {
    Q_OBJECT

public:
    DownloadWidget(const QUrl & url,const QString & out_file_name,int threads_count,QWidget *parent = 0);
    DownloadWidget(const Subtitle & subtitle,const QString & out_file_name,QWidget *parent = 0);
    ~DownloadWidget();
    bool isDownloading() const;

private slots:
    void download_finished(const QString & err);
    void download_progress(qreal bytes_downloaded,qreal length,int percents,qreal speed);
    void on_errorButton_clicked();
    void on_terminateButton_clicked();
    void on_stopStartButton_clicked();

private:
    void init_new_downloader();
    void init_new_subs_downloader();
    void init();

    Ui::DownloadWidget *ui;
    DownloaderInterface * downloader;
    QString lastError;
    QUrl url;
    QString out_file_name;
    Subtitle subtitle;
    int threads_count;
    bool is_terminated;
    bool is_paused;
};

#endif // DOWNLOADWIDGET_H
