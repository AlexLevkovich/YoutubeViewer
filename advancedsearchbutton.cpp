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

void AdvancedSearchButton::setPopupChannelId(const QString & channel_id) {
    m_searchWidget->setChannelId(channel_id);
}

