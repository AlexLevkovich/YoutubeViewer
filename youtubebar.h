#ifndef YOUTUBEBAR_H
#define YOUTUBEBAR_H

#include <QToolBar>
#include <QUrl>
#include "youtubesearch.h"

class QAction;
class SearchWidget;
class YoutubeSettingsWidget;

class YoutubeBar : public QToolBar {
    Q_OBJECT
public:
    explicit YoutubeBar(QWidget *parent = 0);
    void init();
    void addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count);

signals:
    void search_completed(const QList<Media> & medias);
    void search_started();


private slots:
    void show_search_videos_popup(const QString & channel_id = QString());
    void search_was_completed(const QList<Media> & medias);
    void search_was_started();
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);
    void search_error(int code,const QString & err_str);
    void previous_page();
    void next_page();
    void first_page();
    void refresh_page();
    void stop_processing();

private:
    YouTubeSearch youtube_search;
    QAction * m_previous_page;
    QAction * m_next_page;
    QAction * m_first_page;
    QAction * m_page_num;
    QAction * m_refresh_page;
    QAction * m_stop_processing;
    bool was_error;
    SearchWidget * search_widget;
    YoutubeSettingsWidget * settings_widget;
    int current_page;
    QString current_page_token;
};

#endif // YOUTUBEBAR_H
