#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "youtubeview.h"
#include "waitview.h"
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include "default_values.h"
#include "errordialog.h"
#include <QMessageBox>

extern QSettings *theSettings;
#define VIDEO_TAG "=video%2F"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    showFirstTime = true;
    youtube_view = new YoutubeView(ui->centralWidget);
    wait_view = new WaitView(ui->centralWidget);
    ui->centralWidget->setOrientation(Qt::Horizontal);
    ui->centralWidget->addWidget(youtube_view);
    ui->centralWidget->addWidget(wait_view);
    connect(ui->mainToolBar,SIGNAL(search_completed(const QList<Media> &)),this,SLOT(search_completed(const QList<Media> &)));
    connect(ui->mainToolBar,SIGNAL(play_stop_requested()),this,SLOT(play_stop_requested()));
    connect(ui->mainToolBar,SIGNAL(search_started()),this,SLOT(search_started()));
    connect(youtube_view,SIGNAL(download_request(const QUrl &,const QString &)),this,SLOT(adding_download(const QUrl &,const QString &)));
    connect(youtube_view,SIGNAL(download_subs_request(const Subtitle &,const QString &)),this,SLOT(adding_subs_download(const Subtitle &,const QString &)));
    connect(youtube_view,SIGNAL(search_requested(const QString &,
                                                 const QString &,
                                                 const QString &,
                                                 const QString &,
                                                 YoutubeOrderBy,
                                                 YoutubeTime)),
                       ui->mainToolBar,SIGNAL(search_requested(const QString &,
                                                               const QString &,
                                                               const QString &,
                                                               const QString &,
                                                               YoutubeOrderBy,
                                                               YoutubeTime)));
    connect(youtube_view,SIGNAL(channel_videos_popup_requested(const QString &)),ui->mainToolBar,SLOT(show_search_videos_popup(const QString &)));
    connect(youtube_view,SIGNAL(mediaListIsEmpty()),this,SLOT(mediaListIsEmpty()));
    connect(youtube_view,SIGNAL(model_changed()),this,SLOT(mediaListIsEmpty()));
    connect(youtube_view,SIGNAL(indexSelected(const QModelIndex &)),this,SLOT(indexSelected(const QModelIndex &)));

    ui->mainToolBar->init();
    wait_view->setVisible(false);
    youtube_view->setVisible(true);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::search_completed(const QList<Media> & medias) {
    youtube_view->setData(medias);
    wait_view->setVisible(false);
    youtube_view->setVisible(true);
}

void MainWindow::search_started() {
    ui->mainToolBar->setPlayMode(Disabled);
    youtube_view->setVisible(false);
    wait_view->setVisible(true);
}

void MainWindow::showEvent(QShowEvent * event) {
    QMainWindow::showEvent(event);
    if (showFirstTime) {
        QMetaObject::invokeMethod(this,"set_window_size",Qt::QueuedConnection);
        showFirstTime = false;
    }
}

void MainWindow::closeEvent(QCloseEvent * event) {
    if (ui->mainToolBar->areDownloadsInProgress()) {
        if (QMessageBox::question(this,tr("Exit confirmation"),tr("Some downloads are in progress.\nAre you sure about exiting?"),QMessageBox::Yes|QMessageBox::No) == QMessageBox::No) {
            event->ignore();
            return;
        }
    }

    theSettings->setValue("is_main_maximized",isMaximized());
    theSettings->setValue("mainwindow_geometry",saveGeometry());
    theSettings->setValue("mainwindow_state",saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::set_window_size() {
    restoreGeometry(theSettings->value("mainwindow_geometry").toByteArray());
    restoreState(theSettings->value("mainwindow_state").toByteArray());
    if (theSettings->value("is_main_maximized",false).toBool()) showMaximized();
}

void MainWindow::adding_download(const QUrl & url,const QString & filename) {
    QString save_name = filename;
    if (save_name.isEmpty()) save_name="video.mp4";
    QString ext = QFileInfo(save_name).suffix();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),theSettings->value("saved_download_path",QDir::homePath()).toString()+"/"+save_name,tr("Videos")+" (*."+ext+")");
    if (!fileName.isEmpty()) {
        ui->mainToolBar->addNewDownload(url,fileName,theSettings->value("threads_count",THREADS_COUNT).toInt());
        theSettings->setValue("saved_download_path",QFileInfo(fileName).dir().path());
    }
}

void MainWindow::adding_subs_download(const Subtitle & subtitle,const QString & filename) {
    QString save_name = filename;
    if (save_name.isEmpty()) save_name="video.srt";
    QString ext = QFileInfo(save_name).suffix();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),theSettings->value("saved_download_path",QDir::homePath()).toString()+"/"+save_name,tr("Subtitles")+" (*."+ext+")");
    if (!fileName.isEmpty()) {
        ui->mainToolBar->addNewDownload(subtitle,fileName);
        theSettings->setValue("saved_download_path",QFileInfo(fileName).dir().path());
    }
}

void MainWindow::play_stop_requested() {
    if (ui->mainToolBar->isPlaying()) {
        ui->mainToolBar->setPlayMode(Play);
        if (!m_play_errors.isEmpty()) ErrorDialog(tr("The last errors"),m_play_errors,this).exec();
    }
    else {
        m_play_errors.clear();
        ui->mainToolBar->setPlayMode(Stop);
        QMetaObject::invokeMethod(youtube_view,"execPlayer",Qt::QueuedConnection,Q_ARG(const QObject *,this),Q_ARG(const char *,"play_next"));
    }
}

void MainWindow::indexSelected(const QModelIndex & index) {
    if (!ui->mainToolBar->isPlaying()) m_current_index = index;
    if (ui->mainToolBar->playMode() == Disabled) ui->mainToolBar->setPlayMode(Play);
}

void MainWindow::mediaListIsEmpty() {
    ui->mainToolBar->setPlayMode(Disabled);
    m_current_index = QModelIndex();
}

void MainWindow::play_next(const QString & error) {
    if (!error.isEmpty()) m_play_errors += error + "\n";
    if (!ui->mainToolBar->isPlaying()) return;
    m_current_index = youtube_view->selectNextIndexAfter(m_current_index);
    if (!m_current_index.isValid()) {
        ui->mainToolBar->setPlayMode(Play);
        if (!m_play_errors.isEmpty()) ErrorDialog(tr("The last errors"),m_play_errors,this).exec();
        m_play_errors.clear();
        return;
    }
    QMetaObject::invokeMethod(youtube_view,"execPlayer",Qt::QueuedConnection,Q_ARG(const QObject *,this),Q_ARG(const char *,"play_next"));
}
