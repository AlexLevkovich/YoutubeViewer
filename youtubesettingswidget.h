#ifndef YOUTUBESETTINGSWIDGET_H
#define YOUTUBESETTINGSWIDGET_H

#include <QWidget>
#include <QUrl>
#include "youtubesearch.h"

namespace Ui {
class YoutubeSettingsWidget;
}

class YoutubeSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit YoutubeSettingsWidget(QWidget *parent = 0);
    ~YoutubeSettingsWidget();
    void addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count);
    void addNewDownload(const Subtitle & subtitle,const QString & out_file_name);
    bool areDownloadsInProgress();

private:
    Ui::YoutubeSettingsWidget *ui;
};

#endif // YOUTUBESETTINGSWIDGET_H
