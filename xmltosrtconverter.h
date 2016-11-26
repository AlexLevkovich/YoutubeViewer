#ifndef XMLTOSRTCONVERTER_H
#define XMLTOSRTCONVERTER_H

#include <QByteArray>
#include <QString>

class XmlToSrtConverter {
public:
    XmlToSrtConverter(const QByteArray & input);
    inline QByteArray result() const { return m_buffer; }
    static bool writeToSrtFile(const QByteArray & input,const QString & fileName);
    static bool writeToSrtTempFile(const QByteArray & input,QString & tempFileName);
    inline bool isValid() { return !m_buffer.isEmpty(); }
private:
    QString formatSrtTime(const QString & time) const;

    QByteArray m_buffer;
};

#endif // XMLTOSRTCONVERTER_H
