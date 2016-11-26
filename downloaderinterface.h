#ifndef DOWNLOADERINTERFACE_H
#define DOWNLOADERINTERFACE_H

#include <QObject>

class DownloaderInterface : public QObject {
    Q_OBJECT
public:
    inline DownloaderInterface(const QString & out_file_name,QObject *parent = NULL) : QObject(parent) {
        m_out_file_name = out_file_name;
    }
    virtual void terminate() = 0;
    virtual void start() = 0;
    virtual bool isDownloading() const = 0;
    QString outputFileName() const {
        return m_out_file_name;
    }

protected:
    inline DownloaderInterface(QObject *parent = NULL) {}
    virtual void finished(const QString & err) = 0;
    virtual void progress(qreal bytes_downloaded,qreal length,int percents,qreal speed) = 0;

private:
    QString m_out_file_name;
};


#endif // DOWNLOADERINTERFACE_H
