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
    YoutubeSearchWidget * searchWidget = new YoutubeSearchWidget(this);
    setMenu(new WidgetMenu(searchWidget,this));
    if (old_menu != NULL) delete old_menu;
    connect(this,SIGNAL(released()),this,SLOT(search_triggered()));
    connect(searchWidget,SIGNAL(search_requested(const QString &,
                                                 const QString &,
                                                 const QString &,
                                                 YoutubeOrderBy,
                                                 YoutubeTime)),
            this,SIGNAL(search_requested(const QString &,
                                       const QString &,
                                       const QString &,
                                       YoutubeOrderBy,
                                       YoutubeTime)));
}

void AdvancedSearchButton::search_triggered() {
    if (m_edit != NULL) {
        emit search_requested(m_edit->text(),"","",relevance,YoutubeTime());
    }
}

void AdvancedSearchButton::setSearchLinePointer(QLineEdit * edit) {
    m_edit = edit;
}
