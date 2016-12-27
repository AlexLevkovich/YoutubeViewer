#include "multidownloader.h"
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QDataStream>
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>

#define DEF_PART_LENGTH 524288
#define TIMER_INTERVAL 2000

PartManager::PartManager(const QString & outputName,int part_length) {
    m_last_added_part_id = 0;
    m_progress = 0;
    m_part_length = (part_length <= 0)?DEF_PART_LENGTH:part_length;
    m_file.setFileName(outputName);
}

bool PartManager::open() {
    if (!m_file.open(QIODevice::WriteOnly)) {
        m_error = m_file.errorString();
        return false;
    }
    return true;
}

bool PartManager::isOpen() {
    return m_file.isOpen();
}

void PartManager::close() {
    m_file.close();
}

void PartManager::setOutputName(const QString & outputName) {
    m_error.clear();
    m_parts.clear();
    m_progress = 0;
    m_last_added_part_id = 0;
    bool was_open = isOpen();
    if (was_open) close();
    m_file.setFileName(outputName);
    if (was_open) open();
}

bool PartManager::clear() {
    m_error.clear();
    m_parts.clear();
    m_progress = 0;
    m_last_added_part_id = 0;
    if (isOpen()) close();
    return open();
}

void PartManager::setPartLength(int part_length) {
    if (part_length < 0) return;
    m_part_length = part_length;
}

int PartManager::createOrFindEmptyPart(qint64 maxSize,const QList<int> & ignore_indexes) {
    if (!isOpen()) return -1;

    Part part;
    if (m_parts.isEmpty()) {
        part.begin_pos = 0;
        part.curr_pos = 0;
        part.rest = m_part_length;
        if (m_part_length > maxSize) part.rest = maxSize;
        m_parts.append(part);
        m_last_added_part_id = 0;

        return part_zero_fill(0);
    }
    else {
        int i;
        for (i=(m_last_added_part_id + 1);i < m_parts.count();i++) {
            if (!partIsFull(i) && !ignore_indexes.contains(i)) {
                m_last_added_part_id = i;
                return i;
            }
        }

        if (!isLastPart(m_parts.count()-1,maxSize)) {
            part.begin_pos = part.curr_pos = m_parts.last().next_part_begin_pos();
            part.rest = m_part_length;
            if (part.next_part_begin_pos() > maxSize) part.rest -= part.next_part_begin_pos() - maxSize;
            m_parts.append(part);
            m_last_added_part_id = m_parts.count() - 1;
            return part_zero_fill(m_last_added_part_id);
        }

        for (i=0;i < m_last_added_part_id;i++) {
            if (!partIsFull(i) && !ignore_indexes.contains(i)) {
                m_last_added_part_id = i;
                return i;
            }
        }
    }

    return -1;
}

int PartManager::findPartByPos(qint64 pos) {
    Part search_part;
    search_part.begin_pos = 0;
    search_part.curr_pos = pos;
    search_part.rest = -1;
    QList<Part>::iterator i = qBinaryFind(m_parts.begin(), m_parts.end(), search_part);
    if (i == m_parts.end()) return -1;
    return i - m_parts.begin();
}

bool PartManager::reservePartsForPos(qint64 pos,qint64 maxSize) {
    if (pos < m_parts.last().next_part_begin_pos()) {
        int part_id = findPartByPos(pos);
        if (part_id < 0) return false;
        m_last_added_part_id = (part_id == 0)?part_id:part_id - 1;
        return true;
    }

    for (int i=m_parts.count()-1;!hasLastPart(maxSize);i++) {
        m_last_added_part_id = i;
        if (createOrFindEmptyPart(maxSize) < 0) return false;
        if (m_parts.last().contains(pos)) {
            int part_id = m_parts.count() - 1;
            m_last_added_part_id = (part_id == 0)?part_id:part_id - 1;
            return true;
        }
    }

    return false;
}

int PartManager::part_zero_fill(int id) {
    if (!isOpen()) return -1;

    Part part = m_parts.at(id);
    int part_rest = part.rest;
    m_file.seek(part.curr_pos);

    int ret;
    while (part_rest > 0) {
        ret = m_file.write(QByteArray(part_rest,(char)0));
        if (ret < 0) {
            m_error = m_file.errorString();
            return -1;
        }
        part_rest -= ret;
    }

    return id;
}

bool PartManager::writeToPart(int part_id,char * data,int len) {
    if (!isOpen()) return false;

    if (len <= 0 || data == NULL || !isValid()) {
        m_error = QObject::tr("Nothing to write!!!");
        return false;
    }

    Part & part = m_parts[part_id];

    if (part.rest <= 0) {
        m_error = QObject::tr("No way to write. Part is full!!!");
        return false;
    }

    m_file.seek(part.curr_pos);

    int ret;
    int written = 0;
    if (len > part.rest) len = part.rest;

    while (written < len) {
        ret = m_file.write(data+written,len-written);
        if (ret < 0) {
            m_error = m_file.errorString();
            return false;
        }
        part.rest -= ret;
        part.curr_pos += ret;
        written += ret;
        m_progress += ret;
    }

    return true;
}

bool PartManager::writeToPart(int part_id,const QByteArray & data) {
    if (!isOpen()) return false;

    if (data.length() <= 0 || !isValid()) {
        m_error = QObject::tr("Nothing to write!!!");
        return false;
    }

    Part & part = m_parts[part_id];

    if (part.rest <= 0) {
        m_error = QObject::tr("No way to write. Part is full!!!");
        return false;
    }

    m_file.seek(part.curr_pos);

    int ret;
    int written = 0;
    int len = data.length();
    if (len > part.rest) len = part.rest;

    while (written < len) {
        ret = m_file.write(data.data()+written,len-written);
        if (ret < 0) {
            m_error = m_file.errorString();
            return false;
        }
        part.rest -= ret;
        part.curr_pos += ret;
        written += ret;
        m_progress += ret;
    }

    return true;
}

QList<int> PartManager::incompletedPartIndexes() {
    QList<int> ret_list;
    for (int i=0;i<m_parts.count();i++) {
        if (!partIsFull(i)) ret_list.append(i);
    }
    qSort(ret_list.begin(),ret_list.end());
    return ret_list;
}

QDataStream & operator<<(QDataStream &stream, const PartManager::Part & part) {
    stream << part.begin_pos;
    stream << part.curr_pos;
    stream << part.rest;
    return stream;
}

QDataStream & operator>>(QDataStream &stream,PartManager::Part & part) {
    stream >> part.begin_pos;
    stream >> part.curr_pos;
    stream >> part.rest;
    return stream;
}

QDataStream & operator<<(QDataStream &stream, const PartManager & part_manager) {
    stream << part_manager.m_part_length;
    stream << part_manager.m_file.fileName();
    stream << part_manager.m_error;
    stream << part_manager.m_parts;
    stream << part_manager.m_progress;
    stream << part_manager.m_last_added_part_id;
    return stream;
}

QDataStream & operator>>(QDataStream &stream,PartManager & part_manager) {
    part_manager.m_error.clear();
    stream >> part_manager.m_part_length;
    QString file_name;
    stream >> file_name;
    if (part_manager.m_file.isOpen()) part_manager.m_file.close();
    part_manager.m_file.setFileName(file_name);
    if (!part_manager.m_file.open(QIODevice::ReadWrite)) part_manager.m_error = part_manager.m_file.errorString();
    QString m_error;
    stream >> m_error;
    if (part_manager.m_error.isEmpty()) part_manager.m_error = m_error;
    stream >> part_manager.m_parts;
    stream >> part_manager.m_progress;
    stream >> part_manager.m_last_added_part_id;
    return stream;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MultiDownloader::MultiDownloader(const QUrl & url,int threads_count,const QString & outputName) : QThread() {
    m_url = url;
    m_threads_count = threads_count;
    m_outputName = outputName;
    m_size = 0;
    m_prev_bytes_written = 0;

    m_manager = new QNetworkAccessManager(this);
    m_manager->setConfiguration(QNetworkConfigurationManager().defaultConfiguration());
    m_part_manager = new PartManager(outputName,0);
    m_timer = new QTimer(this);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(timeout()));

    connect(this,SIGNAL(destroyed()),this,SLOT(aboutToDestroyed()));

    moveToThread(this);
    QThread::start();
}

void MultiDownloader::aboutToDestroyed() {
    if (isRunning()) quit();
    if (isRunning()) wait();
}

MultiDownloader::~MultiDownloader() {
    delete m_part_manager;
}

QUrl MultiDownloader::url() {
    return m_url;
}

bool MultiDownloader::setUrl(const QUrl & url) {
    if (isStarted()) return false;

    m_url = url;
    return true;
}

void MultiDownloader::setThreadCount(int threads_count) {
    m_threads_count = threads_count;
}

int MultiDownloader::partLenght() {
    return m_part_manager.get()->partLenght();
}

void MultiDownloader::setPartLength(int part_length) {
    if (part_length < 0) return;
    m_part_manager.get()->setPartLength(part_length);
}

int MultiDownloader::threadsCount() {
    return m_threads_count;
}

bool MultiDownloader::start() {
    setErrorString("");

    if (isStarted()) {
        setErrorString(tr("Downloading already has been started!!!"));
        return false;
    }

    if (QThread::currentThread() == this) {
        setErrorString(tr("MultiDownloader::start(): cannot be executed from the same thread!!!"));
        return false;
    }

    if (threadsCount() <= 0) {
        setErrorString(tr("Invalid count of threads has passed!!!"));
        return false;
    }

    if (m_manager == NULL) {
        setErrorString(tr("Invalid QNetworkAccessManager instance!!!"));
        return false;
    }

    m_size = 0;
    m_prev_bytes_written = 0;

    if (!m_part_manager.get()->clear()) {
        setErrorString(m_part_manager.get()->errorString());
        return false;
    }

    invokeMethod("private_start");

    return true;
}

void MultiDownloader::private_start() {
    QNetworkReply * m_reply = m_manager->get(QNetworkRequest(url()));
    m_reply->setParent(this);
    m_reply->ignoreSslErrors();
    m_reply->setProperty("type","main");
    connect(m_reply,SIGNAL(metaDataChanged()),this,SLOT(mainMetaDataChanged()));
    connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
}

void MultiDownloader::setDataLength(qint64 size) {
    m_size = size;
}

void MultiDownloader::mainMetaDataChanged() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    setDataLength(m_reply->header(QNetworkRequest::ContentLengthHeader).toLongLong());
    if (dataLength() <= 0) {
        m_reply->abort();
        m_reply->deleteLater();

        was_error(tr("Returned file length is zero. Multidownloading is not possible!!!"));
        return;
    }
    m_reply->abort();
    m_reply->deleteLater();

    if (!addNewPartDownload()) {
        if (errorString().isEmpty()) was_error(tr("Failed to start downloading of the first part!!!"));
        return;
    }
    m_timer.get()->start(TIMER_INTERVAL);
}

bool MultiDownloader::addNewPartDownload(int part_id) {
    QList<int> working_indexes = workingPartIndexes();

    if (m_part_manager.get()->maxAllocatedLength() >= dataLength() && working_indexes == m_part_manager.get()->incompletedPartIndexes()) return true;
    if (part_id < 0) {
        if (countWorkingParts() >= threadsCount()) return false;
        part_id = m_part_manager.get()->createOrFindEmptyPart(dataLength(),working_indexes);
        if (part_id < 0) {
            was_error(m_part_manager.get()->errorString());
            return false;
        }
    }
    else {
        if (!m_part_manager.get()->partExists(part_id)) return false;
        if (m_part_manager.get()->partIsFull(part_id)) return true;
    }

    QNetworkRequest part_request(url());
    qint64 curr_pos = m_part_manager.get()->partCurrPos(part_id);
    part_request.setRawHeader("Range",QString("bytes=%1-%2").arg(curr_pos).arg(curr_pos+m_part_manager.get()->partRest(part_id)-1).toLatin1());
    QNetworkReply * m_reply = m_manager->get(part_request);
    m_reply->setParent(this);
    m_reply->ignoreSslErrors();
    m_reply->setProperty("type","child");
    m_reply->setProperty("part",part_id);
    connect(m_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(was_error(QNetworkReply::NetworkError)));
    connect(m_reply,SIGNAL(finished()),this,SLOT(child_finished()));
    connect(m_reply,SIGNAL(readyRead()),this,SLOT(child_readyRead()));
    connect(this,SIGNAL(error_occured()),m_reply,SLOT(abort()));

    QMetaObject::invokeMethod(this,"addNewPartDownload",Qt::QueuedConnection);
    return true;
}

void MultiDownloader::was_error(QNetworkReply::NetworkError) {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    if (m_reply->error() == QNetworkReply::OperationCanceledError) return;

    was_error(m_reply->errorString(),m_reply);
}

void MultiDownloader::was_error(const QString & error,QNetworkReply * reply) {
    setErrorString(error);
    if (reply != NULL && reply->property("type").toString() == "main") {
        reply->abort();
        reply->deleteLater();
    }
    m_timer.get()->stop();
    m_part_manager.get()->close();
    emit error_occured();
}

void MultiDownloader::child_readyRead() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    int part_id = m_reply->property("part").toInt();
    if (!m_part_manager.get()->writeToPart(part_id,m_reply->readAll())) {
        was_error(m_part_manager.get()->errorString(),m_reply);
        return;
    }

    if (m_part_manager.get()->partIsFull(part_id)) m_reply->abort();
}

void MultiDownloader::child_finished() {
    QNetworkReply * m_reply = (QNetworkReply *)QObject::sender();

    m_reply->abort();
    m_reply->deleteLater();

    if (!m_timer.get()->isActive()) {
        return;
    }

    if (m_part_manager.get()->writtenLength() >= dataLength()) {
        m_timer.get()->stop();
        QMetaObject::invokeMethod(this,"emit_download_completed",Qt::QueuedConnection);
        return;
    }

    if (countWorkingParts() >= threadsCount()) return;

    addNewPartDownload();
}

void MultiDownloader::emit_download_completed() {
    m_part_manager.get()->close();
    emit download_completed();
}

void MultiDownloader::timeout() {
    qint64 all_written = m_part_manager.get()->writtenLength();

    if (all_written == m_prev_bytes_written) return;

    emit progress(all_written,(int)(((double)all_written)/((double)dataLength()/100.0)),(all_written - m_prev_bytes_written)/(TIMER_INTERVAL/1000));
    m_prev_bytes_written = all_written;
}

qint64 MultiDownloader::dataLength() {
    return m_size;
}

void MultiDownloader::private_terminate() {
    m_save_size = dataLength();
    setDataLength(0);
    QList<QNetworkReply *> list = findChildren<QNetworkReply *>();
    for (int i=0;i<list.count();i++) {
        list[i]->abort();
    }
}

bool MultiDownloader::terminate() {
    if (!isStarted()) {
        setErrorString(tr("Downloading has not been started!!!"));
        return false;
    }

    if (QThread::currentThread() == this) {
        setErrorString(tr("MultiDownloader::terminate(): cannot be executed from the same thread!!!"));
        return false;
    }

    invokeMethod("private_terminate");

    QEventLoop loop;
    connect(this,SIGNAL(download_completed()),&loop,SLOT(quit()));
    loop.exec();

    setDataLength(m_save_size);

    return true;
}

bool MultiDownloader::saveState(const QString & outputName) {
    if (isStarted()) return false;

    QFile file(outputName);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QDataStream stream(&file);
    stream << m_url;
    stream << m_threads_count;
    stream << m_outputName;
    stream << *m_part_manager.get();
    stream << m_error;
    stream << m_size;
    stream << m_prev_bytes_written;
    return (stream.status() == QDataStream::Ok);
}

void MultiDownloader::setErrorString(const QString & error) {
    m_error = error;
}

QString MultiDownloader::errorString() {
    return m_error;
}

bool MultiDownloader::continueSaved(const QString & inputName) {
    if (isStarted()) {
        setErrorString(tr("Downloading already has been started!!!"));
        return false;
    }

    if (QThread::currentThread() == this) {
        setErrorString(tr("MultiDownloader::continueSaved(): cannot be executed from the same thread!!!"));
        return false;
    }

    QUrl old_url = m_url;

    QFile file(inputName);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QDataStream stream(&file);
    stream >> m_url;
    stream >> m_threads_count;
    stream >> m_outputName;
    stream >> *m_part_manager.get();
    stream >> m_error;
    stream >> m_size;
    stream >> m_prev_bytes_written;
    if (stream.status() != QDataStream::Ok) return false;

    if (old_url != m_url) {
        file.close();
        file.remove();

        m_size = 0;
        m_prev_bytes_written = 0;

        if (!m_part_manager.get()->clear()) {
            setErrorString(m_part_manager.get()->errorString());
            return false;
        }

        invokeMethod("private_start");
    }
    else invokeMethod("private_continueSaved");

    return true;
}

void MultiDownloader::private_continueSaved() {
    bool started = false;

    m_timer.get()->start(TIMER_INTERVAL);
    QList<int> indexes = m_part_manager.get()->incompletedPartIndexes();
    for (int i=0;i<qMin(threadsCount(),indexes.count());i++) {
        started = true;
        addNewPartDownload(indexes.at(i));
    }

    if (!started) {
        if (countWorkingParts() >= threadsCount()) return;
        addNewPartDownload();
    }
}

bool MultiDownloader::isStarted() {
    return m_timer.get()->isActive();
}

QList<int> MultiDownloader::workingPartIndexes() {
    QList<int> ret_list;
    QList<QNetworkReply *> list = findChildren<QNetworkReply *>();
    for (int i=0;i<list.count();i++) {
        if (list.at(i)->property("type").toString() == "main") continue;
        int part_id = list.at(i)->property("part").toInt();
        if (!m_part_manager.get()->partIsFull(part_id)) ret_list.append(part_id);
    }
    qSort(ret_list.begin(),ret_list.end());
    return ret_list;
}

int MultiDownloader::countWorkingParts() {
    return findChildren<QNetworkReply *>().count();
}

QString MultiDownloader::outputName() {
    return m_outputName;
}

bool MultiDownloader::setOutputName(const QString & outputName) {
    m_outputName = outputName;
    m_part_manager.get()->setOutputName(m_outputName);
    return true;
}

bool MultiDownloader::moveDownloadPointer(qint64 pos) {
    return m_part_manager.get()->reservePartsForPos(pos,dataLength());
}

bool MultiDownloader::invokeMethod(const char *member,QGenericArgument val0,QGenericArgument val1,QGenericArgument val2,QGenericArgument val3,QGenericArgument val4,QGenericArgument val5,QGenericArgument val6,QGenericArgument val7,QGenericArgument val8,QGenericArgument val9) {
    return QMetaObject::invokeMethod(this,member,Qt::QueuedConnection,val0,val1,val2,val3,val4,val5,val6,val7,val8,val9);
}
