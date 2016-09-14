#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include "default_values.h"
#ifdef WIN32
#include "explorerstyle.h"
#endif

QSettings *theSettings = NULL;
QString TOOLS_BIN_PATH = TOOLS_BIN;

QMainWindow * findMainWindow() {
    foreach(QWidget *widget, qApp->topLevelWidgets()) {
        if(widget->inherits("QMainWindow")) {
            return (QMainWindow *)widget;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
#ifdef WIN32
    a.setStyle(new ExplorerStyle());
#endif

    QTranslator m_translator;
    QString lang = QLocale::system().name().split("_").at(0);
    if(!m_translator.load("youtubeviewer_" + lang, TRANS_DIR2))
        if (!m_translator.load("youtubeviewer_" + lang, TRANS_DIR1))
            m_translator.load("youtubeviewer_" + lang, QFileInfo(QCoreApplication::applicationFilePath()).dir().path());
    QApplication::installTranslator(&m_translator);

    QTranslator m_translator2;
    if (m_translator2.load(QLocale::system(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&m_translator2);
    }

    QCoreApplication::setOrganizationName("AlexL");
    QCoreApplication::setApplicationName("YoutubeViewer");
    QSettings settings(QSettings::IniFormat,QSettings::UserScope,QCoreApplication::organizationName(),QCoreApplication::applicationName());
    theSettings = &settings;

#ifdef WIN32
    TOOLS_BIN_PATH = QFileInfo(QCoreApplication::applicationFilePath()).dir().path() + "/utils";
#endif
    TOOLS_BIN_PATH = theSettings->value("tools_path",TOOLS_BIN_PATH).toString();

    if (!QDir(TOOLS_BIN_PATH).exists()) {
        QMessageBox::critical(NULL,QObject::tr("Critical error!"),QObject::tr("Looks like TOOL directory is incorrect!\nYou can change it in Settings dialog."));
    }

    MainWindow w;
    w.show();

    return a.exec();
}
