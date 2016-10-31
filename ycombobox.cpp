#include "ycombobox.h"

YComboBox::YComboBox(QWidget *parent) : QComboBox(parent) {}

void YComboBox::showPopup() {
    emit aboutShowPopup();
    QComboBox::showPopup();
    QMetaObject::invokeMethod(this,"popup_list",Qt::QueuedConnection);
}

void YComboBox::popup_list() {
    if (count() <= 0) return;
    QComboBox::showPopup();
}
