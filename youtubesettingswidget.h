#ifndef YOUTUBESETTINGSWIDGET_H
#define YOUTUBESETTINGSWIDGET_H

#include <QWidget>
#include <QUrl>

namespace Ui {
class YoutubeSettingsWidget;
}

class YoutubeSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit YoutubeSettingsWidget(QWidget *parent = 0);
    ~YoutubeSettingsWidget();
    void addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count);

private:
    Ui::YoutubeSettingsWidget *ui;
};

#endif // YOUTUBESETTINGSWIDGET_H
