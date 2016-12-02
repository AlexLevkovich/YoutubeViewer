#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include "youtubesearch.h"
#include "youtubebar.h"

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
    void adding_subs_download(const Subtitle & subtitle,const QString & title);
    void play_stop_requested(ButtonPlayMode mode);
    void indexSelected(const QModelIndex & index);
    void mediaListIsEmpty();
    void play_next(const QString & prev_error);
    void play_prev(const QString & prev_error);

protected:
    void closeEvent(QCloseEvent * event);
    void showEvent(QShowEvent * event);

private:
    Ui::MainWindow *ui;
    YoutubeView * youtube_view;
    WaitView * wait_view;
    bool showFirstTime;
    QModelIndex m_current_index;
    QString m_play_errors;
};

#endif // MAINWINDOW_H
