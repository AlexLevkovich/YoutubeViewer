#include "settingsbutton.h"
#include <QMenu>
#include <QMessageBox>
#include "settingsdialog.h"
#include "vlcsettingsdialog.h"
#include "mpvsettingsdialog.h"
#ifdef WIN32
#include "mpchcsettingsdialog.h"
#endif
#include "default_values.h"
#include "createnewyoutubekeydialog.h"

SettingsButton::SettingsButton(QWidget *parent) : QToolButton(parent) {
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);

    setIcon(QIcon(":/images/res/configure.png"));
    setToolTip(tr("Configure the program's parameters"));
    QMenu * old_menu = this->menu();
    QMenu * menu = new QMenu(this);
    menu->addAction(icon(),tr("Settings..."),this,SLOT(settings()));
    menu->addAction(QIcon(":/images/res/dialog-password.png"),tr("Change the developer key..."),this,SLOT(key()));
    menu->addAction(QIcon(":/images/res/vlc.png"),"Vlc...",this,SLOT(vlc()));
    menu->addAction(QIcon(":/images/res/mpv.png"),"Mpv...",this,SLOT(mpv()));
#ifdef WIN32
    menu->addAction(QIcon(":/images/res/mpc-hc.png"),"MPC-HC...",this,SLOT(mpc()));
#endif
    menu->addAction(QIcon(":/images/res/youtube.png"),tr("About..."),this,SLOT(about()));
    setMenu(menu);
    if (old_menu != NULL) delete old_menu;
}

void SettingsButton::settings() {
    SettingsDialog(this).exec();
}

void SettingsButton::key() {
    CreateNewYoutubeKeyDialog(this).exec();
}

void SettingsButton::vlc() {
    VlcSettingsDialog(this).exec();
}

void SettingsButton::mpv() {
    MpvSettingsDialog(this).exec();
}

void SettingsButton::mpc() {
#ifdef WIN32
    MpcHcSettingsDialog(this).exec();
#endif
}

void SettingsButton::about() {
    QMessageBox::about(this,tr("About YoutubeViewer..."),tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">YoutubeViewer</span> is youtube viewer.</span></p>"
                                                            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">YoutubeViewer version is %1.</p>"
                                                            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Developer: Alex Levkovich (<a href=\"mailto:alevkovich@tut.by\"><span style=\" text-decoration: underline; color:#0057ae;\">alevkovich@tut.by</span></a>)</p>"
                                                            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">License: GPL</p>").arg(VERSION));
}
