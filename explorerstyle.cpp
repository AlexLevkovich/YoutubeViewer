#include "explorerstyle.h"

/****************************************************************************
**
** Copyright (c) 2007 Trolltech ASA <info@trolltech.com>
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "explorerstyle.h"

#include <qt_windows.h>
#include <uxtheme.h>

#ifndef HTHEME
#define HTHEME void*
#endif

typedef HANDLE (WINAPI *PtrOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (WINAPI *PtrDrawThemeBackground)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId,
                                                 const RECT *pRect, OPTIONAL const RECT *pClipRect);
typedef bool (WINAPI *PtrIsAppThemed)();

static PtrDrawThemeBackground pDrawThemeBackground = 0;
static PtrOpenThemeData pOpenThemeData = 0;
static PtrIsAppThemed pIsAppThemed = 0;

HDC qt_win_display_dc();

bool isAppThemed()
{
#ifdef Q_OS_WIN
    return pIsAppThemed && pIsAppThemed();
#else
    return false;
#endif
}

ExplorerStyle::ExplorerStyle()
    : QProxyStyle()
{
#ifdef Q_OS_WIN
    QLibrary themeLib(QLatin1String("uxtheme"));
    themeLib.load();
    if (themeLib.isLoaded()) {  //resolve uxtheme functions
        pIsAppThemed =          (PtrIsAppThemed)themeLib.resolve("IsAppThemed");
        pDrawThemeBackground =  (PtrDrawThemeBackground)themeLib.resolve("DrawThemeBackground");
        pOpenThemeData  =       (PtrOpenThemeData)themeLib.resolve("OpenThemeData");
    }
#endif
}

void drawRebarBackground(const QRect &rect,  QPainter *painter) {
    if (rect.isEmpty())
        return;
    QString cacheKey = QLatin1String("q_rebar_") + QString::number(rect.size().width())
                        + QLatin1Char('x') + QString::number(rect.size().height());
    QPixmap pixmap;
    if (!QPixmapCache::find(cacheKey, pixmap)) {
        pixmap = QPixmap(rect.size());
        const RECT wRect = {0, 0, pixmap.width(), pixmap.height()};
        HBITMAP bitmap = QtWin::toHBITMAP(pixmap);
        HDC hdc = CreateCompatibleDC(qt_win_display_dc());
        HGDIOBJ oldhdc = (HBITMAP)SelectObject(hdc, bitmap);
        HTHEME theme = pOpenThemeData(0, L"REBAR");
        pDrawThemeBackground(theme, hdc, 0, 0, &wRect, NULL);
        pixmap = QtWin::fromHBITMAP(bitmap);
        SelectObject(hdc, oldhdc);
        DeleteObject(bitmap);
        DeleteDC(hdc);
        QPixmapCache::insert(cacheKey, pixmap);
    }
    painter->drawPixmap(rect.topLeft(), pixmap);
}

void ExplorerStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                                    QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_Widget:
        if (isAppThemed()) {
            if (QMainWindow *window = qobject_cast<QMainWindow*>(widget->window())) {
                QRegion topreg;
                QRegion bottomreg;
                QMenuBar *menubar = window->findChild<QMenuBar*>();

                //We draw the menubar as part of the top toolbar area
                if (menubar) {
                    QRect rect(menubar->mapToParent(menubar->rect().topLeft()), menubar->rect().size());
                    topreg += rect;
                }

                //We need the bounding rect for all toolbars
                QList<QToolBar*> toolbars = window->findChildren<QToolBar*>();
                foreach (const QToolBar *tb, toolbars) {
                    if (!tb->isFloating()) {
                        QRect rect(tb->mapToParent(tb->rect().topLeft()), tb->rect().size());
                        if (window->toolBarArea(const_cast<QToolBar*>(tb)) == Qt::TopToolBarArea)
                            topreg += rect;
                        else if (window->toolBarArea(const_cast<QToolBar*>(tb)) == Qt::BottomToolBarArea)
                            bottomreg += rect;
                    }
                }

                //This is a hack to workaround missing toolbar updates since
                //we now require updates that span across the whole toolbar area:
                QRect topRect = topreg.boundingRect();
                topRect.setWidth(window->width());
                if (m_currentTopRect != topRect) {
                    m_currentTopRect = topRect;
                    window->update(topRect);
                }

                QRect bottomRect = bottomreg.boundingRect();
                bottomRect.setWidth(window->width());
                if (m_currentBottomRect != bottomRect) {
                    m_currentBottomRect = bottomRect;
                    window->update(bottomRect);
                }

                //Fill top toolbar area with gradient
                drawRebarBackground(topRect, painter);
                //Fill bottom toolbar area with gradient
                drawRebarBackground(bottomRect, painter);
            }
            break;

        } //fall through
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void ExplorerStyle::drawControl(ControlElement element, const QStyleOption *option,
                                  QPainter *painter, const QWidget *widget) const
{
    QColor shadow = option->palette.dark().color();
    shadow.setAlpha(120);

    switch (element) {
    case CE_DockWidgetTitle:
        if (isAppThemed()) {
            if (const QStyleOptionDockWidget *dwOpt = qstyleoption_cast<const QStyleOptionDockWidget*>(option)) {
                painter->save();
                QStyleOptionDockWidget adjustedOpt = *dwOpt;
                QRect rect = option->rect.adjusted(0, 1, -1, -2);
                adjustedOpt.palette.setBrush(QPalette::All, QPalette::Dark, Qt::transparent);
                painter->save();
                painter->setClipRect(rect);
                drawRebarBackground(rect.adjusted(0, 0, 1, 1), painter);
                painter->restore();
                painter->setPen(shadow);
                painter->drawRect(rect);
                QProxyStyle::drawControl(element, &adjustedOpt, painter, widget);
                painter->restore();
            }
            break;
        } //fall through

    case CE_MenuBarItem:
        if (isAppThemed()) {
            if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
                QStyleOptionMenuItem adjustedItem = *mbi;
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Dark, shadow);
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Button, Qt::NoBrush);
                QProxyStyle::drawControl(element, &adjustedItem, painter, widget);
            }
            break;
        } //fall through

    case CE_MenuBarEmptyArea:
        if (isAppThemed()) {
            if (const QStyleOptionMenuItem *mbi = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
                QStyleOptionMenuItem adjustedItem = *mbi;
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Dark, shadow);
                adjustedItem.palette.setBrush(QPalette::All, QPalette::Button, Qt::NoBrush);
                QProxyStyle::drawControl(element, &adjustedItem, painter, widget);
            }
            break;
        } //fall through

    case CE_ToolBar:
        if (isAppThemed()) {
            if (const QStyleOptionToolBar *toolbar = qstyleoption_cast<const QStyleOptionToolBar*>(option)) {
                QStyleOptionToolBar adjustedToolBar = *toolbar;
                adjustedToolBar.palette.setBrush(QPalette::All, QPalette::Dark, shadow);
                QProxyStyle::drawControl(element, &adjustedToolBar, painter, widget);
            }
        break;
        } //fall through

    default:
        return QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
}

void ExplorerStyle::polish(QWidget *widget)
{
    if (QMainWindow *window = qobject_cast<QMainWindow*>(widget))
        window->setAttribute(Qt::WA_StyledBackground, true);
    return QProxyStyle::polish(widget);
}
