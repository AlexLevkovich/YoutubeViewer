#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "youtubeview.h"
#include "waitview.h"
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include "default_values.h"
#include "youtubesearch.h"
#include <QMessageBox>
#include "createnewyoutubekeydialog.h"

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
    connect(ui->mainToolBar,SIGNAL(search_started()),this,SLOT(search_started()));
    connect(youtube_view,SIGNAL(download_request(const QUrl &,const QString &)),this,SLOT(adding_download(const QUrl &,const QString &)));
    connect(youtube_view,SIGNAL(search_requested(const QString &,
                                                  const QString &,
                                                  const QString &,
                                                  YoutubeOrderBy,
                                                  YoutubeTime)),
                       ui->mainToolBar,SLOT(search_requested(const QString &,
                                                  const QString &,
                                                  const QString &,
                                                  YoutubeOrderBy,
                                                  YoutubeTime)));
    connect(youtube_view,SIGNAL(channel_videos_popup_requested(const QString &)),ui->mainToolBar,SLOT(show_search_videos_popup(const QString &)));

    start_getting_categories();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::start_getting_categories() {
    wait_view->setVisible(true);
    youtube_view->setVisible(false);

    YouTubeSearch * youtube_search = new YouTubeSearch(this);
    connect(youtube_search,SIGNAL(categories_completed()),this,SLOT(categories_completed()));
    connect(youtube_search,SIGNAL(error(int,const QString &)),this,SLOT(categories_error(int,const QString &)));
    youtube_search->download_categories(theSettings->value("youtube_user_key","").toString());
}

void MainWindow::categories_completed() {
    ui->mainToolBar->init();
    wait_view->setVisible(false);
    youtube_view->setVisible(true);
}

void MainWindow::categories_error(int code,const QString & err) {
    YouTubeSearch * m_search = (YouTubeSearch *)QObject::sender();

    if ((code == 400) || (code == 403)) {
        CreateNewYoutubeKeyDialog dlg(this);
        if (dlg.exec() == QDialog::Rejected) {
            qApp->quit();
            return;
        }
        theSettings->setValue("youtube_user_key",dlg.key());
        m_search->deleteLater();
        start_getting_categories();
        return;
    }
    QMessageBox::critical(this,tr("Error during Categories returning!"),err);
    qApp->quit();
}

void MainWindow::search_completed(const QList<Media> & medias) {
    youtube_view->setData(medias);
    wait_view->setVisible(false);
    youtube_view->setVisible(true);
}

void MainWindow::search_started() {
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
    QString url_str = url.toString();

    QString save_name = filename;
    if (save_name.isEmpty()) save_name="video.mp4";
    QString ext = QFileInfo(save_name).suffix();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),theSettings->value("saved_download_path",QDir::homePath()).toString()+"/"+save_name,tr("Videos")+" (*."+ext+")");
    if (!fileName.isEmpty()) {
        ui->mainToolBar->addNewDownload(url,fileName,theSettings->value("threads_count",THREADS_COUNT).toInt());
        theSettings->setValue("saved_download_path",QFileInfo(fileName).dir().path());
    }
}
