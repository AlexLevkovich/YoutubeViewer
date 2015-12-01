#include "widgetmenu.h"
#include <QWidgetAction>
#include <QApplication>
#include <QDesktopWidget>

#define min(a,b) ((a<b)?a:b)

WidgetMenu::WidgetMenu(QWidget * widget, QWidget *parent) : QMenu(parent) {
    isFirstTime = true;
    m_widget = widget;

    widget->setParent(this);
    widget_action = new QWidgetAction(this);
    widget_action->setDefaultWidget(widget);
    widget_action->setText(widget->windowTitle());
    addAction(widget_action);
    connect(this,SIGNAL(aboutToShow()),this,SLOT(aboutToShow()));
}

void WidgetMenu::aboutToShow() {
    if (!isFirstTime) return;
    isFirstTime = false;

    m_widget->setMinimumHeight(min(m_widget->sizeHint().height(),QApplication::desktop()->availableGeometry(this).height()));
}

