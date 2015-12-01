#include "widgetsmenu.h"
#include <QWidgetAction>
#include <QActionEvent>

WidgetsMenu::WidgetsMenu(QWidget *parent) : QMenu(parent) {
    connect(this,SIGNAL(aboutToShow()),this,SLOT(aboutToShow()));
}

void WidgetsMenu::addWidgetItem(QWidget * widget) {
    QWidgetAction * widget_action = new QWidgetAction(this);
    widget_action->setDefaultWidget(widget);
    widget->setProperty("parent",QVariant::fromValue((void *)widget_action));
    widget_action->setText(widget->windowTitle());
    if (actions().count() > 0) {
        QAction * sep = ((QMenu *)this)->addSeparator();
        widget->setProperty("separator",QVariant::fromValue((void *)sep));
    }
    ((QMenu *)this)->addAction(widget_action);
}

void WidgetsMenu::removeWidgetItem(QWidget * widget) {
    QList<QAction *> actions = this->actions();
    for (int i=0;i<actions.count();i++) {
        if (!actions[i]->inherits("QWidgetAction")) continue;
        QWidgetAction * action = (QWidgetAction *)actions[i];
        if (action->defaultWidget() != widget) continue;
        delete action;
        return;
    }
}

void WidgetsMenu::aboutToShow() {
    QList<QAction *> actions = this->actions();
    for (int i=0;i<actions.count();i++) {
        if (!actions[i]->inherits("QWidgetAction")) continue;
        QWidget * widget = ((QWidgetAction *)actions[i])->defaultWidget();
        if (widget != NULL) widget->setMinimumHeight(widget->sizeHint().height());
    }
}

void WidgetsMenu::actionEvent(QActionEvent * e) {
    QMenu::actionEvent(e);
    if (e->type() == QEvent::ActionRemoved) {
        if (actions().count() <= 0) hide();
    }
}
