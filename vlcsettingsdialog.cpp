#include "vlcsettingsdialog.h"
#include "ui_vlcsettingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include "default_values.h"

extern QSettings *theSettings;

VlcSettingsDialog::VlcSettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::VlcSettingsDialog) {
    ui->setupUi(this);
    ui->vlcPathLine->setText(theSettings->value("vlc_path",VLC_PATH).toString());
    ui->cacheSpin->setValue(theSettings->value("vlc_cache",VLC_CACHE_SIZE).toInt());
    ui->paramsLine->setText(theSettings->value("vlc_parms","").toString());
}

VlcSettingsDialog::~VlcSettingsDialog() {
    delete ui;
}

void VlcSettingsDialog::on_buttonBox_accepted() {
    theSettings->setValue("vlc_path",ui->vlcPathLine->text());
    theSettings->setValue("vlc_cache",ui->cacheSpin->value());
    theSettings->setValue("vlc_parms",ui->paramsLine->text());
}

void VlcSettingsDialog::on_pushButton_clicked() {
    QString s = QFileDialog::getOpenFileName(this,tr("Choose vlc executable..."),QString(INSTALL_PREFIX)+"/bin/","All Files (*)");
    if (!s.isEmpty()) ui->vlcPathLine->setText(s);
}
