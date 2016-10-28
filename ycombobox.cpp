#include "ycombobox.h"

YComboBox::YComboBox(QWidget *parent) : QComboBox(parent) {}

void YComboBox::showPopup() {
    emit aboutShowPopup();
    QComboBox::showPopup();
}
