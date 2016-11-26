#ifndef YOUTUBEVIEW_H
#define YOUTUBEVIEW_H

#include <QListView>
#include <QProcess>
#include "youtubesearch.h"

class YoutubeView : public QListView {
    Q_OBJECT
public:
    explicit YoutubeView(QWidget *parent = 0);
    void setData(const QList<Media> & medias);
    QModelIndex selectNextIndexAfter(const QModelIndex & index);

protected:
    void resizeEvent(QResizeEvent * event);
    void setModel(QAbstractItemModel * model);
    void contextMenuEvent(QContextMenuEvent * e);

protected slots:
    void item_activated(const QModelIndex & index);

private slots:
    void execPlayer(const QObject *receiver,const char * execute_at_exit);
    void execPlayer(const QUrl & video_url = QUrl(),const QUrl & audio_url = QUrl());
    void _updateGeometry();
    void _updateGeometry2();
    void show_info();
    void download(const QUrl & url = QUrl());
    void view_uploader_channel();
    void show_channel_videos_popup();
    void on_selectionChanged(const QItemSelection & selected,const QItemSelection & deselected);

signals:
    void model_changed();
    void download_request(const QUrl & url,const QString & title);
    void download_subs_request(const Subtitle & subtitle,const QString & title);
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          const QString & playlist_id,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);
    void channel_videos_popup_requested(const QString & channel_id);
    void indexSelected(const QModelIndex & index);
    void mediaListIsEmpty();

private:
    QAbstractItemModel * prev_model;
    QItemSelectionModel * prev_sel_model;
};

#endif // YOUTUBEVIEW_H
