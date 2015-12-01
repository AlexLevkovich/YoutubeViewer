#ifndef EXTERNALPLAYER_H
#define EXTERNALPLAYER_H

#include <QProcess>

class ExternalPlayer : public QProcess {
    Q_OBJECT
public:
    explicit ExternalPlayer(const QString & program,QObject *parent = 0);

private slots:
    void error(QProcess::ProcessError error);
    void finished(int exitCode,QProcess::ExitStatus exitStatus);

private:
    QString m_program;
};

#endif // EXTERNALPLAYER_H
