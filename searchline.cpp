#include "searchline.h"
#include <QSettings>
#include "default_values.h"

extern QSettings *theSettings;

SearchLine::SearchLine(QWidget *parent) : FancyLineEdit(parent) {
    setHistoryCompleter("youtube_completer");

    connect(this,SIGNAL(returnPressed()),this,SLOT(returnPressed()));
}

void SearchLine::returnPressed() {
    emit search_requested(text(),"","",relevance,YoutubeTime());
}
