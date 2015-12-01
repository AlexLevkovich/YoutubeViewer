#include "youtubesearchwidget.h"
#include "ui_youtubesearchwidget.h"
#include <QMenu>
#include <QSettings>
#include "default_values.h"
#include "youtubesearch.h"

extern QSettings *theSettings;

YoutubeSearchWidget::YoutubeSearchWidget(QWidget *parent) : QWidget(parent), ui(new Ui::YoutubeSearchWidget) {
    ui->setupUi(this);
    setWindowTitle(tr("Search"));

    ui->searchEdit->setHistoryCompleter("youtube_completer");
    ui->authorEdit->setHistoryCompleter("author_completer");

    int i;
    QStringList categories = YouTubeSearch::categories();
    for (i=0;i<categories.count();i++) {
        ui->categoryCombo->addItem(categories[i]);
    }

    ui->searchEdit->setText(theSettings->value("search_def_query","").toString());
    ui->categoryCombo->setCurrentIndex(theSettings->value("search_def_category",0).toInt());
    ui->authorEdit->setText(theSettings->value("search_def_author","").toString());

    for (i=0;;i++) {
        QString text = YouTubeSearch::orderByParameterString((YoutubeOrderBy)i);
        if (text.isEmpty()) break;
        ui->orderbyCombo->addItem(text);
    }
    for (i=0;;i++) {
        QString text = YoutubeTime::timeParameterString((YoutubeTimeId)i);
        if (text.isEmpty()) break;
        ui->timeCombo->addItem(text);
    }

    ui->orderbyCombo->setCurrentIndex(theSettings->value("search_def_orderby",0).toInt());
    ui->timeCombo->setCurrentIndex(theSettings->value("search_def_time_id",0).toInt());
    ui->timeEdit->setDateTime(theSettings->value("search_def_time",QDateTime::currentDateTime()).toDateTime());
}

YoutubeSearchWidget::~YoutubeSearchWidget() {
    delete ui;
}

void YoutubeSearchWidget::on_buttonBox_accepted() {
    ((QWidget *)parent())->hide();
     emit search_requested(ui->searchEdit->text(),
                           ui->categoryCombo->currentText(),
                           ui->authorEdit->text(),
                           (YoutubeOrderBy)ui->orderbyCombo->currentIndex(),
                           YoutubeTime((YoutubeTimeId)ui->timeCombo->currentIndex(),ui->timeEdit->dateTime()));
}

void YoutubeSearchWidget::on_buttonBox_rejected() {
    ((QWidget *)parent())->hide();
}
