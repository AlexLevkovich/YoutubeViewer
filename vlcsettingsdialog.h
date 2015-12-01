#ifndef VLCSETTINGSDIALOG_H
#define VLCSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class VlcSettingsDialog;
}

class VlcSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit VlcSettingsDialog(QWidget *parent = 0);
    ~VlcSettingsDialog();

private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();

private:
    Ui::VlcSettingsDialog *ui;
};

#endif // VLCSETTINGSDIALOG_H
