#ifndef ADVANCEDSEARCHBUTTON_H
#define ADVANCEDSEARCHBUTTON_H

#include "combotoolbutton.h"
#include "youtubesearch.h"

class QLineEdit;

class AdvancedSearchButton : public ComboToolButton {
    Q_OBJECT
public:
    explicit AdvancedSearchButton(QWidget *parent = 0);
    void setSearchLinePointer(QLineEdit * edit);

signals:
    void search_requested(const QString & query,
                          const QString & category,
                          const QString & author,
                          YoutubeOrderBy orderby,
                          YoutubeTime time);

private slots:
    void search_triggered();

private:
    QLineEdit * m_edit;
};

#endif // ADVANCEDSEARCHBUTTON_H
