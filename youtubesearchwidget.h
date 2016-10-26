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
    void setChannel(const QString & channel);
    void setEnableFillButton(bool flag);
    void setFieldsValues(const QString & query,
                         const QString & category,
                         const QString & author,
                         YoutubeOrderBy orderby,
                         YoutubeTime time);

signals:
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);
    void accepted();
    void previousTermsClicked();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::YoutubeSearchWidget *ui;
};

#endif // YOUTUBESEARCHWIDGET_H
