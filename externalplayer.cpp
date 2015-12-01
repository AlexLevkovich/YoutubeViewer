#include "externalplayer.h"
#include <QMessageBox>
#include <QApplication>
#include "errordialog.h"
#include <QSettings>
#include <QMainWindow>
#include "default_values.h"

extern QSettings *theSettings;
extern QMainWindow * findMainWindow();

ExternalPlayer::ExternalPlayer(const QString & program,QObject *parent) : QProcess(parent) {
    setProcessChannelMode(QProcess::MergedChannels);
    connect(this,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)));
    connect(this,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finished(int,QProcess::ExitStatus)));
    m_program = program;
    start(program);
}

void ExternalPlayer::error(QProcess::ProcessError /*error*/) {
    QMessageBox::critical(findMainWindow(),tr("Error during player starting!"),errorString());
    deleteLater();
}

void ExternalPlayer::finished(int exitCode,QProcess::ExitStatus /*exitStatus*/) {
    deleteLater();
    if (exitCode == 0) return;
    ErrorDialog(tr("Error(s) for the command:\n%1").arg(m_program),QString::fromLocal8Bit(readAll()),findMainWindow()).exec();
}
