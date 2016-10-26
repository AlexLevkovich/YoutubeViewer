#include "advancedsearchbutton.h"
#include "widgetmenu.h"
#include "youtubesearchwidget.h"
#include <QLineEdit>
#include <QSettings>
#include "default_values.h"

extern QSettings *theSettings;

AdvancedSearchButton::AdvancedSearchButton(QWidget *parent) : ComboToolButton(parent) {
    m_edit = NULL;

    setIcon(QIcon(":/images/res/adv_search.png"));
    setToolTip(tr("Search with advanced parameters"));
    QMenu * old_menu = this->menu();
    m_searchWidget = new YoutubeSearchWidget(this);
    setMenu(new WidgetMenu(m_searchWidget,this));
    if (old_menu != NULL) delete old_menu;
    connect(this,SIGNAL(released()),this,SLOT(search_triggered()));
    connect(m_searchWidget,SIGNAL(search_requested(const QString &,
                                                 const QString &,
                                                 const QString &,
                                                 YoutubeOrderBy,
                                                 YoutubeTime)),
            this,SLOT(on_search_requested(const QString &,
                                          const QString &,
                                          const QString &,
                                          YoutubeOrderBy,
                                          YoutubeTime)));
    connect(menu(),SIGNAL(aboutToShow()),this,SLOT(menu_up()));
    connect(m_searchWidget,SIGNAL(previousTermsClicked()),this,SLOT(on_previous_terms_clicked()));
}

void AdvancedSearchButton::on_previous_terms_clicked() {
    if (terms_history.count() <= 0) return;
    SearchTerms terms = terms_history.at(terms_history.count()-2);
    m_searchWidget->setFieldsValues(terms.query,terms.category,terms.author,terms.orderby,terms.time);
}

void AdvancedSearchButton::on_search_requested(const QString & query,const QString & category,const QString & author,YoutubeOrderBy orderby,YoutubeTime time) {
    emit search_requested(query,category,author,orderby,time);
    last_terms.query = query;
    last_terms.category = category;
    last_terms.author = author;
    last_terms.orderby = orderby;
    last_terms.time = time;
    if (terms_history.count() > 1) {
        if (terms_history.at(terms_history.count()-2) == last_terms) {
            terms_history.removeLast();
            return;
        }
    }
    terms_history.append(last_terms);
}

void AdvancedSearchButton::menu_up() {
    m_searchWidget->setEnableFillButton(terms_history.count() > 1 || (terms_history.count() == 1 && terms_history.last() != last_terms));
}

void AdvancedSearchButton::search_triggered() {
    if (m_edit != NULL) {
        on_search_requested(m_edit->text(),"","",relevance,YoutubeTime());
    }
}

void AdvancedSearchButton::setSearchLinePointer(QLineEdit * edit) {
    m_edit = edit;
}

void AdvancedSearchButton::setPopupChannel(const QString & channel) {
    m_searchWidget->setChannel(channel);
}

