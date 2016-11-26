#include "providersdialog.h"
#include "ui_providersdialog.h"
#include "default_values.h"
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>

extern QSettings *theSettings;

#ifdef WIN32
#define PROVIDERS_COUNT 3
#else
#define PROVIDERS_COUNT 2
#endif
const QString providers[PROVIDERS_COUNT] = {QObject::tr("VLC media player"),QObject::tr("MPV media player")
#ifdef WIN32
                                            ,QObject::tr("MPC-HC media player")
#endif
                                           };
const QString players[PROVIDERS_COUNT] = {"VLC","MPV"
#ifdef WIN32
                                          ,"MPC-HC"
#endif
                                         };
static QString paths[PROVIDERS_COUNT];
const QString icons[PROVIDERS_COUNT] = {":/images/res/vlc.png",":/images/res/mpv.png"
#ifdef WIN32
                                        ,":/images/res/mpc-hc.png"
#endif
                                       };

ProvidersDialog::ProvidersDialog(const QUrl & video_url,const QUrl & audio_url,QWidget *parent,const QStringList & tempFilesNames) : QDialog(parent), ui(new Ui::ProvidersDialog) {
    ui->setupUi(this);
    m_video_url = video_url;
    m_audio_url = audio_url;
    m_tempFilesNames = tempFilesNames;

    paths[0] = theSettings->value("vlc_path",VLC_PATH).toString();
    paths[1] = theSettings->value("mpv_path",MPV_PATH).toString();
#ifdef WIN32
    paths[2] = theSettings->value("mpc_path",MPC_PATH).toString();
#endif
#if QT_VERSION >= 0x050000
    ui->providersList->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);    
#else    
    ui->providersList->header()->setResizeMode(0,QHeaderView::ResizeToContents);
#endif    
    ui->providersList->setIconSize(QSize(32,32));

    fill();
}

ProvidersDialog::~ProvidersDialog() {
    delete ui;
}

void ProvidersDialog::fill() {
    QString player = theSettings->value("def_player","Ask").toString();
    int i;
    if (player != "Ask") {
        for (i=0;i<PROVIDERS_COUNT;i++) {
            if ((player == players[i]) && !QFileInfo(paths[i]).exists()) {
                player = "Ask";
                break;
            }
        }
    }
    QTreeWidgetItem * firstitem = NULL;
    for (i=0;i<PROVIDERS_COUNT;i++) {
        if (!QFileInfo(paths[i]).exists()) continue;
        if ((player != "Ask") && (player != players[i])) continue;
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->providersList);
        item->setIcon(0,QIcon(icons[i]));
        item->setText(0,providers[i]);
        item->setData(0,Qt::UserRole,i);
        if (firstitem == NULL) firstitem = item;
    }

    if (firstitem != NULL) {
        ui->providersList->setCurrentItem(firstitem);
    }
    else {
        QMessageBox::critical(this,tr("Cannot start the player..."),tr("Please install some supported one!"));
        return;
    }
}

void ProvidersDialog::on_buttonBox_accepted() {
    accept();
}

void ProvidersDialog::on_buttonBox_rejected() {
    ui->providersList->clear();
    reject();
}

QString ProvidersDialog::command() const {
    bool show_fullscreen = theSettings->value("player_fullscreen",false).toBool();
    QString ret;
    if (ui->providersList->currentItem() != NULL) {
        switch (ui->providersList->currentItem()->data(0,Qt::UserRole).toInt()) {
            case 0:
            {
                ret = "\"" + paths[0] + "\" --play-and-exit --no-video-title-show "+
                    (show_fullscreen?"-f ":"")+
                    theSettings->value("vlc_parms","").toString()+" "+
                    QString("--network-caching %1").arg(theSettings->value("vlc_cache",VLC_CACHE_SIZE).toInt())+" "+
                    QString("\"%1\"").arg(m_video_url.toString()) +
                    (!m_audio_url.isEmpty()?QString(" --input-slave \"%1\" ").arg(m_audio_url.toString()):" ");
                for (int i=0;i<m_tempFilesNames.count();i++) {
                    ret += QString("--sub-file \"%1\" ").arg(m_tempFilesNames[i]);
                }
                break;
            }
            case 1:
            {
                ret = "\"" + paths[1] + "\" "+ theSettings->value("mpv_parms","").toString()+" "+
                    (show_fullscreen?"--fullscreen ":"")+
                    QString("--ytdl-format=bestvideo+bestaudio --cache=%1").arg(theSettings->value("mpv_cache",MPV_CACHE_SIZE).toInt())+" "+
                    QString("\"%1\"").arg(m_video_url.toString()) +
                    (!m_audio_url.isEmpty()?QString(" --audio-file \"%1\" ").arg(m_audio_url.toString()):" ");
                for (int i=0;i<m_tempFilesNames.count();i++) {
                    ret += QString("--sub-file \"%1\" ").arg(m_tempFilesNames[i]);
                }
                break;
            }
            case 2:
                ret = "\"" + paths[2] + "\" "+ theSettings->value("mpc_parms","").toString()+" "+
                    (show_fullscreen?"/fullscreen ":"")+
                    QString("\"%1\"").arg(m_video_url.toString());
                break;
        }
        return ret;
    }
    return QString();
}

void ProvidersDialog::on_providersList_doubleClicked(const QModelIndex &/*index*/) {
    accept();
}

void ProvidersDialog::setVisible(bool visible) {
    if (!visible) {
        QDialog::setVisible(visible);
        return;
    }

    if (ui->providersList->topLevelItemCount() > 1) QDialog::setVisible(visible);
    else {
        if (ui->providersList->topLevelItemCount() > 0) QMetaObject::invokeMethod(this,"accept",Qt::QueuedConnection);
        else QMetaObject::invokeMethod(this,"reject",Qt::QueuedConnection);
    }
}
