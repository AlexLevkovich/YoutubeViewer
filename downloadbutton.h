#ifndef DOWNLOADBUTTON_H
#define DOWNLOADBUTTON_H

#include <QToolButton>
#include <QUrl>

class DownloadButton : public QToolButton {
    Q_OBJECT
public:
    explicit DownloadButton(QWidget *parent = 0);
    void addNewDownload(const QUrl & url,const QString & out_file_name,int threads_count);
};

#endif // DOWNLOADBUTTON_H
