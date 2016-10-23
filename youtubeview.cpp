#include "youtubeview.h"
#include "youtubelistmodel.h"
#include "youtubelistitemdelegate.h"
#include "default_values.h"
#include <QKeyEvent>
#include <QSettings>
#include <QMenu>
#include "externalplayer.h"
#include "youtubecommentsdialog.h"
#include "providersdialog.h"


extern QSettings *theSettings;

YoutubeView::YoutubeView(QWidget *parent) : QListView(parent) {
    prev_model = model();
    prev_sel_model = selectionModel();
    setResizeMode(QListView::Adjust);
    setItemDelegate(new YoutubeListItemDelegate(this));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(this,SIGNAL(activated(const QModelIndex &)),this,SLOT(item_activated(const QModelIndex &)));
}

void YoutubeView::setModel(QAbstractItemModel * model) {
    QListView::setModel(model);
    emit model_changed();
}

void YoutubeView::setData(const QList<Media> & medias) {
    setIconSize(theSettings->value("preview_size",QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT)).toSize());
    if (prev_model != NULL) prev_model->removeRows(0,model()->rowCount());
    reset();
    setModel(new YoutubeListModel(medias,this));
    if (prev_model != NULL) prev_model->deleteLater();
    if (prev_sel_model != NULL) prev_sel_model->deleteLater();
    prev_model = model();
    prev_sel_model = selectionModel();
    QMetaObject::invokeMethod(this,"_updateGeometry",Qt::QueuedConnection);
}

void YoutubeView::_updateGeometry() {
    resize(QSize(size().width(),size().height()-1));
    QMetaObject::invokeMethod(this,"_updateGeometry2",Qt::QueuedConnection);
}

void YoutubeView::_updateGeometry2() {
    resize(QSize(size().width(),size().height()+1));
}

void YoutubeView::item_activated(const QModelIndex & /*index*/) {
    execPlayer();
}

void YoutubeView::resizeEvent(QResizeEvent * event) {
    QListView::resizeEvent(event);
    emit model_changed();
}

void YoutubeView::execPlayer(const QUrl & video_url,const QUrl & audio_url) {
    QAction * source_object = (QAction *)QObject::sender();
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    QUrl m_video_url = video_url;
    QUrl m_audio_url = audio_url;
    Media * media = (Media *)sel_list.at(0).data(Qt::UserRole).value<void *>();
    if (!video_url.isValid()) {
        m_video_url = media->best_video().url;
        m_audio_url = media->best_video().audio_url;
    }
    if (!video_url.isValid() && (source_object != NULL) && source_object->property("video_url").isValid()) {
        m_video_url = source_object->property("video_url").toUrl();
        m_audio_url = source_object->property("audio_url").toUrl();
    }
    if (!m_video_url.isValid()) {
        m_video_url = media->url();
        m_audio_url = QUrl();
    }

    ProvidersDialog providers(m_video_url,m_audio_url,this);
    if (providers.exec() == QDialog::Rejected) return;
    new ExternalPlayer(providers.command());
}

void YoutubeView::download(const QUrl & url) {
    QAction * source_object = (QAction *)QObject::sender();
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    QUrl m_url = url;
    Media * media = (Media *)sel_list.at(0).data(Qt::UserRole).value<void *>();
    VideoInfo info = media->best_video();
    if (!url.isValid()) m_url = info.url;
    if (!url.isValid() && (source_object != NULL) && source_object->property("video_url").isValid()) m_url = source_object->property("video_url").toUrl();
    if (!m_url.isValid()) m_url = media->url();

    emit download_request(m_url,info.filename);
}

void YoutubeView::contextMenuEvent(QContextMenuEvent * e) {
    QListView::contextMenuEvent(e);
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    QMenu menu(this);
    QMenu download_menu;
    download_menu.setIcon(QIcon(":/images/res/download.png"));
    download_menu.setTitle(tr("Download..."));
    QMenu playerMenu;
    playerMenu.setIcon(QIcon(":/images/res/media-playback-start.png"));
    playerMenu.setTitle(tr("Play with external player..."));
    Media * media = (Media *)sel_list.at(0).data(Qt::UserRole).value<void *>();
    media->download_video_infos();
    QList<VideoInfo> real_links = media->video_infos();
    if (real_links.count() <= 0) {
        menu.addAction(QIcon(":/images/res/media-playback-start.png"),tr("Play with external player..."),this,SLOT(execPlayer()));
        menu.addAction(QIcon(":/images/res/download.png"),tr("Download..."),this,SLOT(download()));
    }
    else {
        for (int i=0;i<real_links.count();i++) {
            if (!real_links[i].isAudioOnly()) {
                QAction * action = playerMenu.addAction(QIcon(":/images/res/tool-animator.png"),real_links[i].desc.toString(),this,SLOT(execPlayer()));
                action->setProperty("video_url",real_links[i].url);
                action->setProperty("audio_url",real_links[i].audio_url);
            }
            QAction *  action = download_menu.addAction(QIcon(":/images/res/tool-animator.png"),real_links[i].desc.toStringFull(),this,SLOT(download()));
            action ->setProperty("video_url",real_links[i].url);
            action ->setProperty("audio_url",real_links[i].audio_url);
        }
        menu.addMenu(&playerMenu);
        menu.addMenu(&download_menu);
    }

    menu.addAction(QIcon(":/images/res/find-user.png"),tr("View channel")+" \""+media->author()+"\"",this,SLOT(view_uploader_channel()));
    menu.addAction(QIcon(":/images/res/find-user-video.png"),tr("Search videos on channel")+" \""+media->author()+"\"...",this,SLOT(show_channel_videos_popup()));
    menu.addAction(QIcon(":/images/res/help-about.png"),tr("Information..."),this,SLOT(show_info()));
    menu.exec(e->globalPos());
}

void YoutubeView::show_channel_videos_popup() {
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    Media * media = (Media *)sel_list.at(0).data(Qt::UserRole).value<void *>();
    emit channel_videos_popup_requested(media->channel_id());
}

void YoutubeView::view_uploader_channel() {
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    Media * media = (Media *)sel_list.at(0).data(Qt::UserRole).value<void *>();
    emit search_requested("","",media->channel_id(),relevance,YoutubeTime());
}

void YoutubeView::show_info() {
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    YoutubeCommentsDialog((Media *)sel_list.at(0).data(Qt::UserRole).value<void *>(),this).exec();
}
