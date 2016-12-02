#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QMessageBox>

class QEvent;

class ErrorDialog : public QMessageBox {
    Q_OBJECT

public:
    explicit ErrorDialog(const QString& errorlabel,const QString & log,QWidget *parent = 0,const QString & title = tr("Error..."));
    ~ErrorDialog();

protected:
    bool event(QEvent *event);
};

#endif // ERRORDIALOG_H
