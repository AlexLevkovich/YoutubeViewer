#include "errordialog.h"
#include <QShowEvent>
#include <QLayout>
#include <QTextEdit>

ErrorDialog::ErrorDialog(const QString& errorlabel,const QString & log,QWidget *parent,const QString & title) : QMessageBox(parent) {
    setWindowTitle(title);

    setIcon(QMessageBox::Critical);
    setText(errorlabel);
    setDetailedText(log);
    setStandardButtons(QMessageBox::Ok);
    setDefaultButton(QMessageBox::Ok);
    setSizeGripEnabled(true);
}

ErrorDialog::~ErrorDialog() {}

bool ErrorDialog::event(QEvent *e) {
    bool result = QMessageBox::event(e);

    if ( (e->type() == QEvent::LayoutRequest) || (e->type() == QEvent::Resize) ) {
        setMinimumHeight(0);
        setMaximumHeight(QWIDGETSIZE_MAX);
        setMinimumWidth(0);
        setMaximumWidth(QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QTextEdit *textEdit = findChild<QTextEdit *>();

        if (textEdit) {
            textEdit->setMinimumHeight(0);
            textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
            textEdit->setMinimumWidth(0);
            textEdit->setMaximumWidth(QWIDGETSIZE_MAX);
            textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
    }

    return result;
}
