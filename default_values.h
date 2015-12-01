#ifndef DEFAULT_VALUES_H
#define DEFAULT_VALUES_H

#include <QString>

#define PREVIEW_WIDTH 200
#define PREVIEW_HEIGHT 150
#define SEARCH_MAX_COUNT 50
#define VLC_CACHE_SIZE 500
#define MPV_CACHE_SIZE 1024
#define DESC_MAX_SYM_COUNT 500
#define THREADS_COUNT 8
#define VERSION "1.0"
#ifndef WIN32
const QString VLC_PATH = QString(INSTALL_PREFIX)+"/bin/vlc";
const QString MPV_PATH = QString(INSTALL_PREFIX)+"/bin/mpv";
#else
const QString MPV_PATH = "C:/Program Files/mpv/mpv.exe";
#ifdef __x86_64__
const QString VLC_PATH = "C:/Program Files (x86)/VideoLAN/vlc.exe";
#else
const QString VLC_PATH = "C:/Program Files/VideoLAN/vlc.exe";
#endif
#endif

#endif // DEFAULT_VALUES_H
