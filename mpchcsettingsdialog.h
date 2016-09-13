#ifndef MPCHCSETTINGSDIALOG_H
#define MPCHCSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class MpcHcSettingsDialog;
}

class MpcHcSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MpcHcSettingsDialog(QWidget *parent = 0);
    ~MpcHcSettingsDialog();

private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();

private:
    Ui::MpcHcSettingsDialog *ui;
};

#endif // MPCHCSETTINGSDIALOG_H
