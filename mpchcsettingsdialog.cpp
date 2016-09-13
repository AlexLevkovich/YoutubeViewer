#include "mpchcsettingsdialog.h"
#include "ui_mpchcsettingsdialog.h"
#include "default_values.h"
#include <QSettings>
#include <QFileDialog>

extern QSettings *theSettings;

MpcHcSettingsDialog::MpcHcSettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MpcHcSettingsDialog) {
    ui->setupUi(this);
    ui->mpcPathLine->setText(theSettings->value("mpc_path",MPC_PATH).toString());
    ui->paramsLine->setText(theSettings->value("mpc_parms","").toString());
}

MpcHcSettingsDialog::~MpcHcSettingsDialog() {
    delete ui;
}

void MpcHcSettingsDialog::on_pushButton_clicked() {
    QString s = QFileDialog::getOpenFileName(this,tr("Choose MPC-HC executable..."),QString(INSTALL_PREFIX)+"/bin/","All Files (*)");
    if (!s.isEmpty()) ui->mpcPathLine->setText(s);
}

void MpcHcSettingsDialog::on_buttonBox_accepted() {
    theSettings->setValue("mpc_path",ui->mpcPathLine->text());
    theSettings->setValue("mpc_parms",ui->paramsLine->text());
}

