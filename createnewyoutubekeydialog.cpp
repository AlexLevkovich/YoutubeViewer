#include "createnewyoutubekeydialog.h"
#include "ui_createnewyoutubekeydialog.h"
#include <QSettings>
#include <QPushButton>

extern QSettings *theSettings;

CreateNewYoutubeKeyDialog::CreateNewYoutubeKeyDialog(QWidget *parent) : QDialog(parent), ui(new Ui::CreateNewYoutubeKeyDialog) {
    ui->setupUi(this);
    ui->keyEdit->setText(theSettings->value("youtube_user_key","").toString());
    on_keyEdit_textChanged(ui->keyEdit->text());
}

CreateNewYoutubeKeyDialog::~CreateNewYoutubeKeyDialog() {
    delete ui;
}

QString CreateNewYoutubeKeyDialog::key() const {
    return ui->keyEdit->text();
}

void CreateNewYoutubeKeyDialog::on_keyEdit_textChanged(const QString & text) {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}
