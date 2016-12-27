#ifndef MULTIDOWNLOADER_H
#define MULTIDOWNLOADER_H

#include <QThread>
#include <QUrl>
#include <QFile>
#include <QList>
#include <QNetworkReply>
#include <QTimer>
#include <QDataStream>
#include <QReadWriteLock>
#include <QWriteLocker>
#include <QReadLocker>

template <class T> class Protected
{
public:
    Protected() {}

    void set(const T & value) {
        QWriteLocker wlocker(&lock);
        this->value=value;
    }

    T get() {
        QReadLocker rlocker(&lock);
        return value;
    }

    Protected<T> & operator=(const T & other) {
        set(other);
        return *this;
    }

    operator T () {
        return get();
    }

    operator T () const {
        return ((Protected<T> *)this)->get();
    }

private:
    T value;
    QReadWriteLock lock;
};

template <class T> QDataStream & operator<<(QDataStream &stream, const Protected<T> & part) {
    stream << (T)part;
    return stream;
}

template <class T> QDataStream & operator>>(QDataStream &stream, Protected<T> & part) {
    T t;
    stream >> t;
    part.set(t);
    return stream;
}

class QNetworkAccessManager;

class PartManager {
protected:
    PartManager(const QString & outputName,int part_length);
    bool open();
    bool clear();
    void close();
    bool isOpen();
    inline void clearErrorString() { m_error.clear(); }
    inline int partLenght() { return m_part_length; }
    void setPartLength(int part_length);
    inline QString errorString() { return m_error; }
    inline bool isValid() { return m_file.isOpen() && m_error.isEmpty(); }
    // rc=-1 == error; rc>0 == part_id
    int createOrFindEmptyPart(qint64 maxSize,const QList<int> & ignore_indexes = QList<int>());
    bool reservePartsForPos(qint64 pos,qint64 maxSize);
    inline qint64 partCurrPos(int part_id) { return m_parts.at(part_id).curr_pos; }
    inline qint64 partRest(int part_id) { return m_parts.at(part_id).rest; }
    inline bool hasLastPart(qint64 maxSize) { return (m_parts.last().next_part_begin_pos() >= maxSize); }
    inline bool isLastPart(int part_id,qint64 maxSize) { return (m_parts.at(part_id).next_part_begin_pos() >= maxSize); }
    int findPartByPos(qint64 pos);
    bool writeToPart(int part_id,const QByteArray & data);
    bool writeToPart(int part_id,char * data,int len);
    inline bool partIsFull(int part_id) { return (m_parts.at(part_id).rest <= 0); }
    inline bool partExists(int part_id) { return (part_id < m_parts.count()); }
    inline qint64 maxAllocatedLength() { return (m_parts.count() <= 0)?0:m_parts.last().next_part_begin_pos(); }
    inline qint64 writtenLength() { return m_progress; }
    void setOutputName(const QString & outputName);
    QList<int> incompletedPartIndexes();

private:
    int part_zero_fill(int id);

    struct Part {
        qint64 begin_pos;
        qint64 curr_pos;
        int rest;

        inline qint64 next_part_begin_pos() const {
            return curr_pos + rest;
        }

        inline bool contains(qint64 pos) const {
            return (begin_pos <= pos) && (pos < next_part_begin_pos());
        }

        inline bool operator<(const Part & other) const {
            if (rest < 0) return other.contains(curr_pos);
            if (other.rest < 0) return (next_part_begin_pos() <= other.curr_pos);
            return curr_pos < other.curr_pos;
        }
        friend QDataStream & operator<<(QDataStream &stream, const Part & part);
        friend QDataStream & operator>>(QDataStream &stream, Part & part);
    };

    int m_part_length;
    QFile m_file;
    QString m_error;
    QList<Part> m_parts;
    qint64 m_progress;
    int m_last_added_part_id;

    friend QDataStream & operator<<(QDataStream &stream, const Part & part);
    friend QDataStream & operator>>(QDataStream &stream, Part & part);
    friend QDataStream & operator<<(QDataStream &stream, const PartManager & part_manager);
    friend QDataStream & operator>>(QDataStream &stream, PartManager & part_manager);
    friend class MultiDownloader;
};

QDataStream & operator<<(QDataStream &stream, const PartManager::Part & part);
QDataStream & operator>>(QDataStream &stream, PartManager::Part & part);
QDataStream & operator<<(QDataStream &stream, const PartManager & part_manager);
QDataStream & operator>>(QDataStream &stream, PartManager & part_manager);

class MultiDownloader : public QThread {
    Q_OBJECT
public:
    MultiDownloader(const QUrl & url,int threads_count,const QString & outputName);
    ~MultiDownloader();
    int partLenght();
    bool isStarted();
    void setPartLength(int part_length);
    bool start();
    bool continueSaved(const QString & inputName);
    QString errorString();
    qint64 dataLength();
    bool terminate();
    QUrl url();
    bool setUrl(const QUrl & url);
    int threadsCount();
    void setThreadCount(int threads_count);
    QString outputName();
    bool setOutputName(const QString & outputName);
    bool saveState(const QString & outputName);
    bool moveDownloadPointer(qint64 pos);

private slots:
    void was_error(QNetworkReply::NetworkError error);
    void child_finished();
    void mainMetaDataChanged();
    void child_readyRead();
    void emit_download_completed();
    void timeout();
    bool addNewPartDownload(int part_id = -1);

signals:
    void error_occured();
    void download_completed();
    void progress(qint64 downloaded,int percents,qint64 speed);

private slots:
    void aboutToDestroyed();
    void private_start();
    void private_terminate();
    void private_continueSaved();

private:
    void was_error(const QString & error,QNetworkReply * reply = NULL);
    QList<int> workingPartIndexes();
    int countWorkingParts();
    void setErrorString(const QString & error);
    void setDataLength(qint64 size);
    bool invokeMethod(const char *member,QGenericArgument val0 = QGenericArgument(Q_NULLPTR),QGenericArgument val1 = QGenericArgument(),QGenericArgument val2 = QGenericArgument(),QGenericArgument val3 = QGenericArgument(),QGenericArgument val4 = QGenericArgument(),QGenericArgument val5 = QGenericArgument(),QGenericArgument val6 = QGenericArgument(),QGenericArgument val7 = QGenericArgument(),QGenericArgument val8 = QGenericArgument(),QGenericArgument val9 = QGenericArgument());

    Protected<QTimer *> m_timer;
    Protected<QUrl> m_url;
    Protected<int> m_threads_count;
    Protected<QString> m_outputName;
    Protected<PartManager *> m_part_manager;
    QNetworkAccessManager * m_manager;
    Protected<QString> m_error;
    Protected<qint64> m_size;
    Protected<qint64> m_prev_bytes_written;
    Protected<qint64> m_save_size;
};

#endif // MULTIDOWNLOADER_H
