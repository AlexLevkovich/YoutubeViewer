#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>
#include <QUrl>

namespace Ui {
class DownloadWidget;
}

class YoutubeDownloader;

class DownloadWidget : public QWidget {
    Q_OBJECT

public:
    explicit DownloadWidget(const QUrl & url,const QString & out_file_name,int threads_count,QWidget *parent = 0);
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

    Ui::DownloadWidget *ui;
    YoutubeDownloader * downloader;
    QString lastError;
    QUrl url;
    QString out_file_name;
    int threads_count;
    bool is_terminated;
    bool is_paused;
};

#endif // DOWNLOADWIDGET_H
