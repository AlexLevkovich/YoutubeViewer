#include "downloadbutton.h"
#include "widgetsmenu.h"
#include "downloadwidget.h"
#include <QWidgetAction>

DownloadButton::DownloadButton(QWidget *parent) : QToolButton(parent) {
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);

    setIcon(QIcon(":/images/res/download.png"));
    setToolTip(tr("Download manager..."));
    QMenu * old_menu = this->menu();
    setMenu(new WidgetsMenu(this));
    if (old_menu != NULL) delete old_menu;
}

void DownloadButton::addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count) {
    ((WidgetsMenu *)menu())->addWidgetItem(new DownloadWidget(url,out_file_name,threads_count));
    showMenu();
}

bool DownloadButton::areDownloadsInProgress() {
    QList<QAction *> actions = ((WidgetsMenu *)menu())->actions();
    for (int i=0;i<actions.count();i++) {
        QAction * action = actions.at(i);
        if (!action->inherits("QWidgetAction")) continue;
        if (((DownloadWidget *)((QWidgetAction *)action)->defaultWidget())->isDownloading()) return true;
    }
    return false;
}
