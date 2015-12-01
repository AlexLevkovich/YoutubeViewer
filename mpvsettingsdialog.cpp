#include "mpvsettingsdialog.h"
#include "ui_mpvsettingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include "default_values.h"

extern QSettings *theSettings;

MpvSettingsDialog::MpvSettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::MpvSettingsDialog) {
    ui->setupUi(this);
    ui->mpvPathLine->setText(theSettings->value("mpv_path",MPV_PATH).toString());
    ui->cacheSpin->setValue(theSettings->value("mpv_cache",MPV_CACHE_SIZE).toInt());
    ui->paramsLine->setText(theSettings->value("mpv_parms","").toString());
}

MpvSettingsDialog::~MpvSettingsDialog() {
    delete ui;
}

void MpvSettingsDialog::on_buttonBox_accepted() {
    theSettings->setValue("mpv_path",ui->mpvPathLine->text());
    theSettings->setValue("mpv_cache",ui->cacheSpin->value());
    theSettings->setValue("mpv_parms",ui->paramsLine->text());
}

void MpvSettingsDialog::on_pushButton_clicked() {
    QString s = QFileDialog::getOpenFileName(this,tr("Choose mpv executable..."),QString(INSTALL_PREFIX)+"/bin/","All Files (*)");
    if (!s.isEmpty()) ui->mpvPathLine->setText(s);
}
