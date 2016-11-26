#include "youtubesettingswidget.h"
#include "ui_youtubesettingswidget.h"

YoutubeSettingsWidget::YoutubeSettingsWidget(QWidget *parent) : QWidget(parent), ui(new Ui::YoutubeSettingsWidget) {
    ui->setupUi(this);
}

YoutubeSettingsWidget::~YoutubeSettingsWidget() {
    delete ui;
}

void YoutubeSettingsWidget::addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count) {
    ui->downloadButton->addNewDownload(url,out_file_name,threads_count);
}

void YoutubeSettingsWidget::addNewDownload(const Subtitle & subtitle,const QString & out_file_name) {
    ui->downloadButton->addNewDownload(subtitle,out_file_name);
}

bool YoutubeSettingsWidget::areDownloadsInProgress() {
    return ui->downloadButton->areDownloadsInProgress();
}
