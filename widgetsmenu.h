#ifndef WIDGETSMENU_H
#define WIDGETSMENU_H

#include <QMenu>

class WidgetsMenu : public QMenu {
    Q_OBJECT
public:
    explicit WidgetsMenu(QWidget *parent = 0);
    void addWidgetItem(QWidget * widget);
    void removeWidgetItem(QWidget * widget);

protected:
    void actionEvent(QActionEvent * e);

private:
    inline QAction * addAction(const QString & /*text*/) {
        return NULL;
    }
    inline QAction * addAction(const QIcon & /*icon*/, const QString & /*text*/) {
        return NULL;
    }
    inline QAction * addAction(const QString & /*text*/, const QObject * /*receiver*/, const char * /*member*/, const QKeySequence & /*shortcut*/) {
        return NULL;
    }
    inline QAction * addAction(const QIcon & /*icon*/, const QString & /*text*/, const QObject * /*receiver*/, const char * /*member*/, const QKeySequence & /*shortcut*/) {
        return NULL;
    }
    inline void addAction(QAction * /*action*/) {}
    inline QAction * addMenu(QMenu * /*menu*/){
        return NULL;
    }
    inline QMenu * addMenu(const QString & /*title*/) {
        return NULL;
    }
    inline QMenu * addMenu(const QIcon & /*icon*/, const QString & /*title*/) {
        return NULL;
    }
    inline QAction * insertMenu(QAction * /*before*/, QMenu * /*menu*/) {
        return NULL;
    }
    inline QAction * addSeparator() {
        return NULL;
    }
    inline QAction * insertSeparator(QAction * /*before*/) {
        return NULL;
    }

private slots:
    void aboutToShow();
};

#endif // WIDGETSMENU_H
