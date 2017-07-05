#-------------------------------------------------
#
# Project created by QtCreator 2015-04-03T09:54:40
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
win32 {
    QT += winextras
}

TARGET = YoutubeViewer
TEMPLATE = app

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr
}

win32 {
    YOUTUBEDL_BIN=utils/youtube-dl.exe
}

!win32 {
    YOUTUBEDL_BIN = $$system(which youtube-dl 2>/dev/null)
    isEmpty( YOUTUBEDL_BIN ):error( "youtube-dl should be installed!!!" )

    VLC_PATH = $$system(which vlc 2>/dev/null)
    isEmpty( VLC_PATH ):VLC_PATH=/usr/bin/vlc
    DEFINES += VLC_PATH=\\\"$$VLC_PATH\\\"

    MPV_PATH = $$system(which mpv 2>/dev/null)
    isEmpty( MPV_PATH ):MPV_PATH=/usr/bin/mpv
    DEFINES += MPV_PATH=\\\"$$MPV_PATH\\\"
}

TRANS_DIR1 = $$OUT_PWD/translations
TRANS_DIR2 = $$INSTALL_PREFIX/share/youtubeviewer

DEFINES += INSTALL_PREFIX=\\\"$$INSTALL_PREFIX\\\"
DEFINES += TRANS_DIR1=\\\"$$TRANS_DIR1\\\"
DEFINES += TRANS_DIR2=\\\"$$TRANS_DIR2\\\"
DEFINES += YOUTUBEDL_BIN=\\\"$$YOUTUBEDL_BIN\\\"

RC_FILE = win_icon.rc

win32 {
    SOURCES +=  mpchcsettingsdialog.cpp \
                explorerstyle.cpp
    HEADERS +=  mpchcsettingsdialog.h \
                explorerstyle.h
}

SOURCES += main.cpp\
        mainwindow.cpp \
    youtubeview.cpp \
    youtubebar.cpp \
    searchwidget.cpp \
    searchline.cpp \
    youtubesearch.cpp \
    youtubelistitemdelegate.cpp \
    youtubelistmodel.cpp \
    fancylineedit.cpp \
    historycompleter.cpp \
    waitview.cpp \
    youtubeviewsplitter.cpp \
    widgetmenu.cpp \
    youtubesearchwidget.cpp \
    combotoolbutton.cpp \
    advancedsearchbutton.cpp \
    settingsbutton.cpp \
    settingsdialog.cpp \
    vlcsettingsdialog.cpp \
    errordialog.cpp \
    youtubesettingswidget.cpp \
    youtubecommentsreader.cpp \
    youtubecommentsdialog.cpp \
    youtubedownloader.cpp \
    byteshumanizer.cpp \
    widgetsmenu.cpp \
    downloadwidget.cpp \
    downloadbutton.cpp \
    providersdialog.cpp \
    mpvsettingsdialog.cpp \
    json.cpp \
    createnewyoutubekeydialog.cpp \
    ycombobox.cpp \
    externalplayer.cpp \
    xmltosrtconverter.cpp \
    startextplayerhelper.cpp \
    subtitlesdownloader.cpp \
    multidownloader.cpp

HEADERS  += mainwindow.h \
    youtubeview.h \
    youtubebar.h \
    searchwidget.h \
    searchline.h \
    youtubesearch.h \
    youtubelistitemdelegate.h \
    youtubelistmodel.h \
    fancylineedit.h \
    historycompleter.h \
    waitview.h \
    youtubeviewsplitter.h \
    default_values.h \
    widgetmenu.h \
    youtubesearchwidget.h \
    combotoolbutton.h \
    advancedsearchbutton.h \
    settingsbutton.h \
    settingsdialog.h \
    vlcsettingsdialog.h \
    errordialog.h \
    youtubesettingswidget.h \
    youtubecommentsreader.h \
    youtubecommentsdialog.h \
    youtubedownloader.h \
    byteshumanizer.h \
    widgetsmenu.h \
    downloadwidget.h \
    downloadbutton.h \
    providersdialog.h \
    mpvsettingsdialog.h \
    json.h \
    createnewyoutubekeydialog.h \
    ycombobox.h \
    externalplayer.h \
    xmltosrtconverter.h \
    startextplayerhelper.h \
    downloaderinterface.h \
    subtitlesdownloader.h \
    multidownloader.h

FORMS    += mainwindow.ui \
    searchwidget.ui \
    youtubesearchwidget.ui \
    settingsdialog.ui \
    vlcsettingsdialog.ui \
    errordialog.ui \
    youtubesettingswidget.ui \
    youtubecommentsdialog.ui \
    downloadwidget.ui \
    providersdialog.ui \
    mpvsettingsdialog.ui \
    createnewyoutubekeydialog.ui \
    mpchcsettingsdialog.ui

RESOURCES += youtubeviewer.qrc

TRANSLATIONS = $$PWD/translations/youtubeviewer_ru.ts \
               $$PWD/translations/youtubeviewer_be.ts

LRELEASE = $$[QT_INSTALL_BINS]/lrelease
for(tsfile, TRANSLATIONS) { 
    qmfile = $$basename(tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$OUT_PWD/translations
    qmfile = $$qmdir/$$qmfile
    !exists($$qmdir) {
        system($${QMAKE_MKDIR} \"$$qmdir\")
    }
    command = $$LRELEASE $$tsfile -qm $$qmfile
    system($$command) | error("Failed to run: $$command")
}

LUPDATE = $$[QT_INSTALL_BINS]/lupdate -locations relative -no-ui-lines -no-sort
updatets.files = TRANSLATIONS
updatets.commands = $$LUPDATE $$PWD/libqpacman.pro
QMAKE_EXTRA_TARGETS += updatets

qm.files = $$TRANS_DIR1/*.qm
qm.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/youtubeviewer/
qm.CONFIG += no_check_exist

desktop.files = YoutubeViewer.desktop
desktop.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/applications/

icon.files = res/youtubeviewer.png
icon.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/share/pixmaps/

target.path = $$INSTALL_ROOT/$$INSTALL_PREFIX/bin/

INSTALLS += target qm desktop icon
