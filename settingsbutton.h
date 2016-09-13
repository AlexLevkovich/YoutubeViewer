#ifndef SETTINGSBUTTON_H
#define SETTINGSBUTTON_H

#include <QToolButton>

class SettingsButton : public QToolButton {
    Q_OBJECT
public:
    explicit SettingsButton(QWidget *parent = 0);

private slots:
    void settings();
    void key();
    void vlc();
    void mpv();
    void mpc();
    void about();
};

#endif // SETTINGSBUTTON_H
