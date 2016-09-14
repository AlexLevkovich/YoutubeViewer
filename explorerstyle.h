/****************************************************************************
**
** Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#ifndef EXPLORERSTYLE_H
#define EXPLORERSTYLE_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtWinExtras>

class ExplorerStyle : public QProxyStyle
{
public:
    ExplorerStyle();
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = 0) const;
    void drawControl(ControlElement element, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const;
    void polish(QWidget *widget);

private:
    mutable QRect m_currentTopRect; //current toolbar top area size
    mutable QRect m_currentBottomRect; //current toolbar top area size
};

#endif //EXPLORERSTYLE_H
