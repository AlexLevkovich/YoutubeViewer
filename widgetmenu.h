#ifndef WIDGETMENU_H
#define WIDGETMENU_H

#include <QMenu>

class QWidgetAction;
class QWidget;

class WidgetMenu : public QMenu {
    Q_OBJECT
public:
    explicit WidgetMenu(QWidget * widget, QWidget *parent = 0);

private slots:
    void aboutToShow();

private:
    QWidgetAction * widget_action;
    bool isFirstTime;
    QWidget * m_widget;
};

#endif // WIDGETMENU_H
