#ifndef YOUTUBEBAR_H
#define YOUTUBEBAR_H

#include <QToolBar>
#include <QUrl>
#include "youtubesearch.h"

class QAction;
class SearchWidget;
class YoutubeSettingsWidget;
class QToolButton;

enum ButtonPlayMode {
    PlayDown = 0,
    PlayUp,
    Stop,
    Disabled
};


class YoutubeBar : public QToolBar {
    Q_OBJECT
public:
    explicit YoutubeBar(QWidget *parent = 0);
    void init();
    void addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count);
    void addNewDownload(const Subtitle & subtitle,const QString & out_file_name);
    bool areDownloadsInProgress();
    inline ButtonPlayMode playMode() const { return m_play_mode; }
    void setPlayMode(ButtonPlayMode play_mode);
    inline bool isPlaying() const { return m_play_mode == Stop; }

signals:
    void search_completed(const QList<Media> & medias);
    void search_started();
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          const QString & playlist_id,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);
    void play_stop_requested(ButtonPlayMode mode);

private slots:
    void start_down_action_clicked();
    void start_up_action_clicked();
    void start_stop_button_clicked();
    void show_search_videos_popup(const QString & channel = QString());
    void search_was_completed(const QList<Media> & medias);
    void search_was_started();
    void on_search_requested(const QString & query,
                             const QString & category,
                             const QString & author,
                             const QString & playlist_id,
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
    QAction * m_play;
    QToolButton * start_stop_button;
    bool was_error;
    SearchWidget * search_widget;
    YoutubeSettingsWidget * settings_widget;
    int current_page;
    QString current_page_token;
    ButtonPlayMode m_play_mode;
};

#endif // YOUTUBEBAR_H
