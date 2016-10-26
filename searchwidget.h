#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include "youtubesearch.h"

namespace Ui {
class SearchWidget;
}

class SearchWidget : public QWidget {
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = 0);
    ~SearchWidget();
    void setSearchButtonPopupChannel(const QString & channel);
    void showSearchButtonPopup();

signals:
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);

private:
    Ui::SearchWidget *ui;
};

#endif // SEARCHWIDGET_H
