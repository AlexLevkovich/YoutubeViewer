#ifndef YOUTUBESEARCHWIDGET_H
#define YOUTUBESEARCHWIDGET_H

#include <QWidget>
#include "youtubesearch.h"

namespace Ui {
class YoutubeSearchWidget;
}

class QPushButton;

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
                         const QString & playlist_id,
                         YoutubeOrderBy orderby,
                         YoutubeTime time);

signals:
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          const QString & playlist_id,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);
    void accepted();
    void previousTermsClicked();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_channelEdit_textChanged(const QString &arg1);
    void on_playlistShowPopup();
    void on_playlistCombo_currentIndexChanged(int index);
    void on_searchEdit_textChanged(const QString &arg1);
    void update_menu_size();

private:
    Ui::YoutubeSearchWidget *ui;
    QPushButton * okButton;
};

#endif // YOUTUBESEARCHWIDGET_H
