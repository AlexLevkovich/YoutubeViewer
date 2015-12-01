#include "downloadbutton.h"
#include "widgetsmenu.h"
#include "downloadwidget.h"

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
