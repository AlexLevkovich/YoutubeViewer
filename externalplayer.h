#ifndef EXTERNALPLAYER_H
#define EXTERNALPLAYER_H

#include <QProcess>

class ExternalPlayer : public QObject {
    Q_OBJECT
public:
    ExternalPlayer(const QString & program,const QObject *receiver = NULL,const char * execute_at_exit = NULL,QObject *parent = 0);

private slots:
    void error(QProcess::ProcessError error);
    void finished(int exitCode,QProcess::ExitStatus exitStatus);
    void started();

private:
    QProcess m_process;
    QString m_program;
    char * m_execute_at_exit;
    QObject * m_receiver;
};

#endif // EXTERNALPLAYER_H
