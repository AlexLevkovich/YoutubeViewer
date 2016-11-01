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
#define DEF_VIDEO_HEIGHT 720
#define DEF_VIDEO_CODEC  "avc"
#define VERSION "1.0"
#ifndef WIN32
const QString VLC_PATH = QString(INSTALL_PREFIX)+"/bin/vlc";
const QString MPV_PATH = QString(INSTALL_PREFIX)+"/bin/mpv";
#else
const QString MPV_PATH = "C:/Program Files/mpv/mpv.exe";
#ifdef __x86_64__
const QString MPC_PATH = "C:/Program Files/MPC-HC/mpc-hc64.exe";
const QString VLC_PATH = "C:/Program Files (x86)/VideoLAN/vlc.exe";
#else
const QString MPC_PATH = "C:/Program Files/MPC-HC/mpc-hc.exe";
const QString VLC_PATH = "C:/Program Files/VideoLAN/vlc.exe";
#endif
#endif

const int video_heights[] = {
    144,
    180,
    240,
    360,
    480,
    720,
    1080,
    1366,
    2048,
    0
};

const QString video_codecs[] = {
    "vp9",
    "vp8",
    "avc",
    "mp4",
    ""
};

#endif // DEFAULT_VALUES_H
