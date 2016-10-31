#include "youtubebar.h"
#include "searchwidget.h"
#include "advancedsearchbutton.h"
#include "youtubesettingswidget.h"
#include <QMessageBox>
#include <QAction>

extern QSettings *theSettings;

YoutubeBar::YoutubeBar(QWidget *parent) : QToolBar(parent) {
    was_error = false;
    current_page = -1;
}

void YoutubeBar::init() {
    addWidget((search_widget  = new SearchWidget(this)));
    addSeparator();
    (m_first_page = addAction(QIcon(":/images/res/go-first-view.png"),"",this,SLOT(first_page())))->setToolTip(tr("Go to the first page"));
    (m_previous_page = addAction(QIcon(":/images/res/go-previous.png"),"",this,SLOT(previous_page())))->setToolTip(tr("Go to previous page"));
    (m_page_num = addAction("0"))->setEnabled(false);
    (m_next_page = addAction(QIcon(":/images/res/go-next.png"),"",this,SLOT(next_page())))->setToolTip(tr("Go to next page"));
    (m_refresh_page = addAction(QIcon(":/images/res/view-refresh.png"),"",this,SLOT(refresh_page())))->setToolTip(tr("Refresh the current page"));
    (m_stop_processing = addAction(QIcon(":/images/res/process-stop.png"),"",this,SLOT(stop_processing())))->setToolTip(tr("Stop the current processing"));
    m_previous_page->setEnabled(false);
    m_next_page->setEnabled(false);
    m_refresh_page->setEnabled(false);
    m_stop_processing->setEnabled(false);
    m_first_page->setEnabled(false);
    addWidget((settings_widget = new YoutubeSettingsWidget(this)));


    connect(search_widget,SIGNAL(search_requested(const QString &,
                                                  const QString &,
                                                  const QString &,
                                                  const QString &,
                                                  YoutubeOrderBy,
                                                  YoutubeTime)),
                       this,SLOT(on_search_requested(const QString &,
                                                     const QString &,
                                                     const QString &,
                                                     const QString &,
                                                     YoutubeOrderBy,
                                                     YoutubeTime)));
    connect(&youtube_search,SIGNAL(completed(const QList<Media> &)),this,SLOT(search_was_completed(const QList<Media> &)));
    connect(&youtube_search,SIGNAL(error(int,const QString &)),this,SLOT(search_error(int,const QString &)));
}

void YoutubeBar::on_search_requested(const QString & query,
                                  const QString & category,
                                  const QString & author,
                                  const QString & playlist_id,
                                  YoutubeOrderBy orderby,
                                  YoutubeTime time) {
    current_page = 1;
    current_page_token.clear();
    if (youtube_search.search(theSettings->value("youtube_user_key","").toString(),query,category,author,playlist_id,orderby,time)) search_was_started();
}

void YoutubeBar::search_error(int /*code*/,const QString & err_str) {
    was_error = true;
    QMessageBox::critical(this,tr("Network error!"),err_str);
}

void YoutubeBar::search_was_completed(const QList<Media> & medias) {
    QList<Media> m_medias = medias;
    if ((m_medias.count() > 0) && (m_medias[0].title() == "https://youtube.com/devicesupport")) m_medias.removeAt(0);
    emit search_completed(m_medias);
    m_page_num->setText(QString("%1").arg(current_page));
    m_next_page->setEnabled(was_error || youtube_search.hasNextPage());
    m_previous_page->setEnabled(youtube_search.hasPrevPage());
    m_first_page->setEnabled(!was_error && (current_page > 1));
    m_refresh_page->setEnabled(was_error || (current_page > 0));
    m_stop_processing->setEnabled(false);
    search_widget->setEnabled(true);
    settings_widget->setEnabled(true);
}

void YoutubeBar::search_was_started() {
    was_error = false;
    search_widget->setEnabled(false);
    settings_widget->setEnabled(false);
    m_next_page->setEnabled(false);
    m_previous_page->setEnabled(false);
    m_refresh_page->setEnabled(false);
    m_first_page->setEnabled(false);
    m_stop_processing->setEnabled(true);
    emit search_started();
}

void YoutubeBar::previous_page() {
    current_page--;
    current_page_token = youtube_search.prevPageToken();
    if (youtube_search.search_again(current_page_token)) search_was_started();
}

void YoutubeBar::next_page() {
    current_page++;
    current_page_token = youtube_search.nextPageToken();
    if (youtube_search.search_again(current_page_token)) search_was_started();
}

void YoutubeBar::refresh_page() {
    if (youtube_search.search_again(current_page_token)) search_was_started();
}

void YoutubeBar::stop_processing() {
    youtube_search.terminate_processing();
}

void YoutubeBar::first_page() {
    current_page = 1;
    if (youtube_search.search_again("")) search_was_started();
}

void YoutubeBar::addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count) {
    settings_widget->addNewDownload(url,out_file_name,threads_count);
}

bool YoutubeBar::areDownloadsInProgress() {
    return settings_widget->areDownloadsInProgress();
}

void YoutubeBar::show_search_videos_popup(const QString & channel) {
    search_widget->setSearchButtonPopupChannel(channel);
    search_widget->showSearchButtonPopup();
}
