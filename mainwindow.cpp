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

void MainWindow::adding_download(const QUrl & url,const QString & title) {
    QString url_str = url.toString();
    QString ext;
    int index = url_str.indexOf(VIDEO_TAG);
    if (index != -1) {
        ext = url_str.mid(index+strlen(VIDEO_TAG));
        index = ext.indexOf('&');
        if (index != -1) {
            ext = ext.left(index);
        }
        else ext.clear();
    }

    QString save_name = title.trimmed();
    save_name.replace(" ","_");
    if (save_name.isEmpty()) save_name="video";

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),theSettings->value("saved_download_path",QDir::homePath()).toString()+"/"+save_name+(ext.isEmpty()?ext:("."+ext)),tr("Videos")+" (*.mp4 *.webm"+(ext.isEmpty()?ext:(" *."+ext))+")");
    if (!fileName.isEmpty()) {
        ui->mainToolBar->addNewDownload(url,fileName,theSettings->value("threads_count",THREADS_COUNT).toInt());
        theSettings->setValue("saved_download_path",QFileInfo(fileName).dir().path());
    }
}
