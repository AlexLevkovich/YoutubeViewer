#ifndef ADVANCEDSEARCHBUTTON_H
#define ADVANCEDSEARCHBUTTON_H

#include "combotoolbutton.h"
#include "youtubesearch.h"
#include <QList>

class QLineEdit;
class YoutubeSearchWidget;

class AdvancedSearchButton : public ComboToolButton {
    Q_OBJECT
public:
    explicit AdvancedSearchButton(QWidget *parent = 0);
    void setSearchLinePointer(QLineEdit * edit);
    void setPopupChannel(const QString & channel);

signals:
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);

private slots:
    void search_triggered();
    void menu_up();
    void on_previous_terms_clicked();
    void on_search_requested(const QString & query,
                             const QString & category,
                             const QString & author,
                             YoutubeOrderBy orderby,
                             YoutubeTime time);

private:
    struct SearchTerms {
        QString query;
        QString category;
        QString author;
        YoutubeOrderBy orderby;
        YoutubeTime time;

        bool operator==(const SearchTerms & other) const {
            return query == other.query &&
                   category == other.category &&
                   author == other.author &&
                   orderby == other.orderby &&
                   time == other.time;
        }

        bool operator!=(const SearchTerms & other) const {
            return !(*this == other);
        }
    };


    SearchTerms last_terms;
    QList<SearchTerms> terms_history;
    QLineEdit * m_edit;
    YoutubeSearchWidget * m_searchWidget;
};

#endif // ADVANCEDSEARCHBUTTON_H
