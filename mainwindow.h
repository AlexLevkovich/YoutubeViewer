#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include "youtubesearch.h"

namespace Ui {
class MainWindow;
}

class YoutubeView;
class WaitView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void search_completed(const QList<Media> & medias);
    void search_started();
    void set_window_size();
    void adding_download(const QUrl & url,const QString & title);
    void categories_completed();
    void categories_error(int code,const QString & err);

protected:
    void closeEvent(QCloseEvent * event);
    void showEvent(QShowEvent * event);

private:
    void start_getting_categories();

    Ui::MainWindow *ui;
    YoutubeView * youtube_view;
    WaitView * wait_view;
    bool showFirstTime;
};

#endif // MAINWINDOW_H
