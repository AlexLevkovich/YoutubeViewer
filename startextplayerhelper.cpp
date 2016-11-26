#include "startextplayerhelper.h"
#include "providersdialog.h"
#include "externalplayer.h"
#include "xmltosrtconverter.h"
#include <QFile>
#include <QSettings>

extern QSettings *theSettings;

StartExtPlayerHelper::StartExtPlayerHelper(const QUrl & video_url,const QUrl & audio_url,QWidget *parent,const QList<QByteArray> & subtitles,const QObject *receiver,const char * execute_at_exit) : QObject(parent) {
    if (theSettings->value("player_subtitles",true).toBool()) {
        QString tempFileName;
        for (int i=0;i<subtitles.count();i++) {
            if (!XmlToSrtConverter::writeToSrtTempFile(subtitles[i],tempFileName)) continue;
            m_tempFilesNames.append(tempFileName);
        }
    }

    providers = new ProvidersDialog(video_url,audio_url,parent,m_tempFilesNames);
    if (providers->exec() == QDialog::Rejected) return;
    connect(new ExternalPlayer(providers->command(),receiver,execute_at_exit),SIGNAL(destroyed()),this,SLOT(deleteLater()));
}

StartExtPlayerHelper::~StartExtPlayerHelper() {
    delete providers;
    for (int i=0;i<m_tempFilesNames.count();i++) {
        QFile(m_tempFilesNames[i]).remove();
    }
}
