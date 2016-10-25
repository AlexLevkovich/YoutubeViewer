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
    ui->channelidEdit->setHistoryCompleter("channelid_completer");

    int i;
    QStringList categories = YouTubeSearch::categories();
    for (i=0;i<categories.count();i++) {
        ui->categoryCombo->addItem(categories[i]);
    }

    ui->searchEdit->setText(theSettings->value("search_def_query","").toString());
    ui->categoryCombo->setCurrentIndex(theSettings->value("search_def_category",0).toInt());
    ui->channelidEdit->setText(theSettings->value("search_def_channelid","").toString());

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

    connect(ui->prevButton,SIGNAL(clicked()),this,SIGNAL(previousTermsClicked()));
}

YoutubeSearchWidget::~YoutubeSearchWidget() {
    delete ui;
}

void YoutubeSearchWidget::setFieldsValues(const QString & query,const QString & category,const QString & author,YoutubeOrderBy orderby,YoutubeTime time) {
    ui->searchEdit->setText(query);
    ui->categoryCombo->setCurrentText(category);
    ui->channelidEdit->setText(author);
    ui->orderbyCombo->setCurrentIndex((int)orderby);
    ui->timeCombo->setCurrentIndex(time.operation());
    ui->timeEdit->setDateTime(time.date());
}

void YoutubeSearchWidget::on_buttonBox_accepted() {
    ((QWidget *)parent())->hide();
     emit search_requested(ui->searchEdit->text(),
                           ui->categoryCombo->currentText(),
                           ui->channelidEdit->text(),
                           (YoutubeOrderBy)ui->orderbyCombo->currentIndex(),
                           YoutubeTime((YoutubeTimeId)ui->timeCombo->currentIndex(),ui->timeEdit->dateTime()));
}

void YoutubeSearchWidget::on_buttonBox_rejected() {
    ((QWidget *)parent())->hide();
}

void YoutubeSearchWidget::setChannelId(const QString & channel_id) {
    ui->channelidEdit->setText(channel_id);
}

void YoutubeSearchWidget::setEnableFillButton(bool flag) {
    ui->prevButton->setEnabled(flag);
}
