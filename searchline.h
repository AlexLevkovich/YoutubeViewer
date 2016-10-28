#ifndef SEARCHLINE_H
#define SEARCHLINE_H

#include "fancylineedit.h"
#include "youtubesearch.h"

class SearchLine : public FancyLineEdit {
    Q_OBJECT
public:
    explicit SearchLine(QWidget *parent = 0);

signals:
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          const QString & playlist_id,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);

private slots:
    void returnPressed();
};

#endif // SEARCHLINE_H
