#include "externalplayer.h"
#include "errordialog.h"
#include <QMessageBox>
#include <QMainWindow>
#include <QApplication>

extern QMainWindow * findMainWindow();

ExternalPlayer::ExternalPlayer(const QString & program,const QObject *receiver,const char * execute_at_exit,QObject *parent) : QObject(parent) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_process.setProcessChannelMode(QProcess::MergedChannels);

#if QT_VERSION >= 0x050000
    connect(&m_process,SIGNAL(errorOccurred(QProcess::ProcessError)),SLOT(error(QProcess::ProcessError)));
#else
    connect(&m_process,SIGNAL(error(QProcess::ProcessError)),SLOT(error(QProcess::ProcessError)));
#endif
    connect(&m_process,SIGNAL(started()),this,SLOT(started()));
    connect(&m_process,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finished(int,QProcess::ExitStatus)));

    m_receiver = (QObject *)receiver;
    m_execute_at_exit = (char *) execute_at_exit;
    m_program = program;

    m_process.start(program);
}

void ExternalPlayer::started() {
    QApplication::restoreOverrideCursor();
}

void ExternalPlayer::error(QProcess::ProcessError /*error*/) {
    QApplication::restoreOverrideCursor();
    deleteLater();
    if (m_receiver != NULL && m_execute_at_exit != NULL) QMetaObject::invokeMethod(m_receiver,m_execute_at_exit,Qt::QueuedConnection,Q_ARG(QString,m_process.errorString()));
    else QMessageBox::critical(findMainWindow(),tr("Error during player starting!"),m_process.errorString());
}

void ExternalPlayer::finished(int exitCode,QProcess::ExitStatus /*exitStatus*/) {
    deleteLater();
    QString error = QString::fromLocal8Bit(m_process.readAll());
    if (exitCode == 0) error.clear();
    if (m_receiver != NULL && m_execute_at_exit != NULL) {
        QMetaObject::invokeMethod(m_receiver,m_execute_at_exit,Qt::QueuedConnection,Q_ARG(QString,error));
        return;
    }
    if (exitCode == 0) return;
    ErrorDialog(tr("Error(s) for the command:\n%1").arg(m_program),error,findMainWindow()).exec();
}
