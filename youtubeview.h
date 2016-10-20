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

protected:
    void resizeEvent(QResizeEvent * event);
    void setModel(QAbstractItemModel * model);
    void contextMenuEvent(QContextMenuEvent * e);

protected slots:
    void item_activated(const QModelIndex & index);

private slots:
    void _updateGeometry();
    void _updateGeometry2();
    void execPlayer(const QUrl & video_url = QUrl(),const QUrl & audio_url = QUrl());
    void show_info();
    void download(const QUrl & url = QUrl());

signals:
    void model_changed();
    void download_request(const QUrl & url,const QString & title);

private:
    QAbstractItemModel * prev_model;
    QItemSelectionModel * prev_sel_model;
};

#endif // YOUTUBEVIEW_H
