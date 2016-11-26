#ifndef STARTEXTPLAYERHELPER_H
#define STARTEXTPLAYERHELPER_H

#include <QWidget>
#include <QList>
#include <QUrl>

class ProvidersDialog;

class StartExtPlayerHelper : public QObject {
    Q_OBJECT
public:
    StartExtPlayerHelper(const QUrl & video_url,const QUrl & audio_url,QWidget *parent = NULL,const QList<QByteArray> & subtitles = QList<QByteArray>(),const QObject *receiver = NULL,const char * execute_at_exit = NULL);
    ~StartExtPlayerHelper();

private:
    QStringList m_tempFilesNames;
    ProvidersDialog * providers;
};

#endif // STARTEXTPLAYERHELPER_H
