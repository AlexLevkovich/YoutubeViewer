#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QLocale>
#include <QFile>
#include <QFileInfo>
#include "default_values.h"
#ifdef WIN32
#include "explorerstyle.h"
#endif

QSettings *theSettings = NULL;
QString YOUTUBE_DL_BIN = YOUTUBEDL_BIN;
QDateTime MINIMUM_DATE;

QMainWindow * findMainWindow() {
    foreach(QWidget *widget, qApp->topLevelWidgets()) {
        if(widget->inherits("QMainWindow")) {
            return (QMainWindow *)widget;
        }
    }

    return NULL;
}

QString LOCALE_DATETIME_FORMAT() {
    QLocale locale = QLocale::system();
    QString ret = locale.dateFormat(QLocale::ShortFormat) + " " + locale.timeFormat(QLocale::ShortFormat);
    if (!ret.contains("yyyy")) ret.replace("yy","yyyy");
    return ret;
}


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
#ifdef WIN32
    a.setStyle(new ExplorerStyle());
#endif

    qRegisterMetaType<const QObject *>("const QObject *");
    qRegisterMetaType<const char *>("const char *");
    qRegisterMetaType<QNetworkAccessManager::NetworkAccessibility>("QNetworkAccessManager::NetworkAccessibility");
    MINIMUM_DATE = QDateTimeEdit().minimumDateTime();

    QTranslator m_translator;
    QString lang = QLocale::system().name().split("_").at(0);
    if(!m_translator.load("youtubeviewer_" + lang, TRANS_DIR1))
        if (!m_translator.load("youtubeviewer_" + lang, TRANS_DIR2))
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
    YOUTUBE_DL_BIN = QFileInfo(QCoreApplication::applicationFilePath()).dir().path() + "/" + YOUTUBE_DL_BIN;
#endif
    YOUTUBE_DL_BIN = theSettings->value("youtube_dl_path",YOUTUBE_DL_BIN).toString();

    if (!QFile(YOUTUBE_DL_BIN).exists()) {
        QMessageBox::critical(NULL,QObject::tr("Critical error!"),QObject::tr("Looks like PATH to youtube-dl is incorrect!\nYou can change it in Settings dialog."));
    }

    MainWindow w;
    w.show();

    return a.exec();
}
