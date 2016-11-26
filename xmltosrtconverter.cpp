#include "xmltosrtconverter.h"
#include <QDir>
#include <QFile>
#include <QBuffer>
#include <QStringList>
#include <QXmlStreamReader>
#include <QTemporaryFile>

#define BUF_SIZE   8096

XmlToSrtConverter::XmlToSrtConverter(const QByteArray & input) {
    QBuffer buffer(&m_buffer);
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamReader xml_reader(input);
    if (xml_reader.readNextStartElement() && xml_reader.name() == "transcript") {
        int index = 0;
        while (xml_reader.readNextStartElement()) {
            QXmlStreamAttributes attributes = xml_reader.attributes();
            if (xml_reader.name() != "text" || !attributes.hasAttribute("start") || !attributes.hasAttribute("dur")) {
                xml_reader.skipCurrentElement();
                continue;
            }
            bool ok;
            QString s0 = attributes.value("start").toString();
            buffer.write(QString("%1\n%2 --> %3\n%4\n\n").arg(++index).arg(formatSrtTime(s0)).arg(formatSrtTime(QString::number(s0.toDouble(&ok) + attributes.value("dur").toDouble(&ok),'f',3))).arg(xml_reader.readElementText().replace("&#39;", "'").replace("&quot;", "\"").replace("&gt;", ">").replace("&lt;", "<")).toUtf8());
            if (!ok) {
                xml_reader.skipCurrentElement();
                continue;
            }
        }
        if (buffer.size() > 0) return;
    }

    buffer.close();
    m_buffer.clear();
    return;
}

bool XmlToSrtConverter::writeToSrtTempFile(const QByteArray & input,QString & tempFileName) {
    QTemporaryFile file(QDir::tempPath() + QDir::separator() + "XXXXXX.srt");
    file.setAutoRemove(false);
    if (!file.open()) return false;

    tempFileName= file.fileName();
    return writeToSrtFile(input,tempFileName);
}

bool XmlToSrtConverter::writeToSrtFile(const QByteArray & input,const QString & fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) return false;

    QByteArray result = XmlToSrtConverter(input).result();
    QBuffer in_buffer(&result);
    in_buffer.open(QBuffer::ReadOnly);

    while (!in_buffer.atEnd()) {
        if (file.write(in_buffer.read(BUF_SIZE)) == -1) return false;
    }

    return true;
}

QString XmlToSrtConverter::formatSrtTime(const QString & time) const {
    QStringList parts = time.split('.');
    if (parts.count() < 2) parts = (time+".0").split('.');

    bool ok;
    int s = parts.at(0).toInt(&ok);
    if (!ok) return QString();
    int m = s / 60;
    s = s % 60;
    int h = m / 60;
    m = m % 60;

    return QString::asprintf("%02d:%02d:%02d,%s",h,m,s,parts.at(1).toLocal8Bit().data());
}
