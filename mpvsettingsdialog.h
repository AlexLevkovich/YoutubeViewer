#ifndef MPVSETTINGSDIALOG_H
#define MPVSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class MpvSettingsDialog;
}

class MpvSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit MpvSettingsDialog(QWidget *parent = 0);
    ~MpvSettingsDialog();

private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();

private:
    Ui::MpvSettingsDialog *ui;
};

#endif // MPVSETTINGSDIALOG_H
