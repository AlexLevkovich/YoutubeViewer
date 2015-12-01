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

void YoutubeView::execPlayer(const QUrl & url) {
    QAction * source_object = (QAction *)QObject::sender();
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    QUrl m_url = url;
    Media * media = (Media *)sel_list.at(0).data(Qt::UserRole).value<void *>();
    if (!url.isValid()) m_url = media->best_video().url;
    if (!url.isValid() && (source_object != NULL) && source_object->property("url").isValid()) m_url = source_object->property("url").toUrl();
    if (!m_url.isValid()) m_url = media->url();

    ProvidersDialog providers(m_url,this);
    if (providers.exec() == QDialog::Rejected) return;
    new ExternalPlayer(providers.command());
}

void YoutubeView::download(const QUrl & url) {
    QAction * source_object = (QAction *)QObject::sender();
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    QUrl m_url = url;
    Media * media = (Media *)sel_list.at(0).data(Qt::UserRole).value<void *>();
    if (!url.isValid()) m_url = media->best_video().url;
    if (!url.isValid() && (source_object != NULL) && source_object->property("url").isValid()) m_url = source_object->property("url").toUrl();
    if (!m_url.isValid()) m_url = media->url();

    emit download_request(m_url,media->title());
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
            playerMenu.addAction(QIcon(":/images/res/tool-animator.png"),real_links[i].quality.toString(),this,SLOT(execPlayer()))->setProperty("url",real_links[i].url);
            download_menu.addAction(QIcon(":/images/res/tool-animator.png"),real_links[i].quality.toString(),this,SLOT(download()))->setProperty("url",real_links[i].url);
        }
        menu.addMenu(&playerMenu);
        menu.addMenu(&download_menu);
    }

    menu.addAction(QIcon(":/images/res/help-about.png"),tr("Information..."),this,SLOT(show_info()));
    menu.exec(e->globalPos());
}

void YoutubeView::show_info() {
    QModelIndexList sel_list = selectionModel()->selectedRows();
    if (sel_list.count() <= 0) return;

    YoutubeCommentsDialog((Media *)sel_list.at(0).data(Qt::UserRole).value<void *>(),this).exec();
}
