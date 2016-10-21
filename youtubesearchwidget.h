#ifndef YOUTUBESEARCHWIDGET_H
#define YOUTUBESEARCHWIDGET_H

#include <QWidget>
#include "youtubesearch.h"

namespace Ui {
class YoutubeSearchWidget;
}

class YoutubeSearchWidget : public QWidget {
    Q_OBJECT

public:
    explicit YoutubeSearchWidget(QWidget *parent = 0);
    ~YoutubeSearchWidget();
    void setChannelId(const QString & channel_id);

signals:
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);
    void accepted();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::YoutubeSearchWidget *ui;
};

#endif // YOUTUBESEARCHWIDGET_H
