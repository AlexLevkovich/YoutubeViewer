#ifndef CREATENEWYOUTUBEKEYDIALOG_H
#define CREATENEWYOUTUBEKEYDIALOG_H

#include <QDialog>

namespace Ui {
class CreateNewYoutubeKeyDialog;
}

class CreateNewYoutubeKeyDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateNewYoutubeKeyDialog(QWidget *parent = 0);
    ~CreateNewYoutubeKeyDialog();
    QString key() const;

private slots:
    void on_keyEdit_textChanged(const QString &arg1);

private:
    Ui::CreateNewYoutubeKeyDialog *ui;
};

#endif // CREATENEWYOUTUBEKEYDIALOG_H
