#ifndef YCOMBOBOX_H
#define YCOMBOBOX_H

#include <QComboBox>

class YComboBox : public QComboBox {
    Q_OBJECT
public:
    YComboBox(QWidget *parent = NULL);
    void showPopup();
signals:
    void aboutShowPopup();
};

#endif // YCOMBOBOX_H
