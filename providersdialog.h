#ifndef PROVIDERSDIALOG_H
#define PROVIDERSDIALOG_H

#include <QDialog>
#include <QModelIndex>
#include <QUrl>

namespace Ui {
class ProvidersDialog;
}

class ProvidersDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProvidersDialog(const QUrl & video_url,const QUrl & audio_url,QWidget *parent = 0);
    ~ProvidersDialog();
    QString command() const;

protected:
    void setVisible(bool visible);


protected slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_providersList_doubleClicked(const QModelIndex &index);

private:
    void fill();

    Ui::ProvidersDialog *ui;
    QUrl m_video_url;
    QUrl m_audio_url;
};

#endif // PROVIDERSDIALOG_H
