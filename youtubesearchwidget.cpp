#include "youtubesearchwidget.h"
#include "ui_youtubesearchwidget.h"
#include <QMenu>
#include <QSettings>
#include <QMessageBox>
#include <QFontMetrics>
#include <QPixmap>
#include "default_values.h"
#include "youtubesearch.h"

extern QSettings *theSettings;
extern QString LOCALE_DATETIME_FORMAT();

YoutubeSearchWidget::YoutubeSearchWidget(QWidget *parent) : QWidget(parent), ui(new Ui::YoutubeSearchWidget) {
    ui->setupUi(this);
    setWindowTitle(tr("Search"));

    ui->playlistCombo->setEnabled(false);
    ui->timeEdit->setDisplayFormat(LOCALE_DATETIME_FORMAT());

    okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(false);

    ui->searchEdit->setHistoryCompleter("youtube_completer");
    ui->channelEdit->setHistoryCompleter("channelid_completer");

    int i;
    QStringList categories = YouTubeSearch::categories();
    for (i=0;i<categories.count();i++) {
        ui->categoryCombo->addItem(categories[i]);
    }

    ui->searchEdit->setText(theSettings->value("search_def_query","").toString());
    ui->categoryCombo->setCurrentIndex(theSettings->value("search_def_category",0).toInt());
    ui->channelEdit->setText(theSettings->value("search_def_channelid","").toString());

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
    YoutubeTime time;
    ui->timeCombo->setCurrentIndex(theSettings->value("search_def_time_id",(int)time.operation()).toInt());
    ui->timeEdit->setDateTime(theSettings->value("search_def_time",time.date()).toDateTime());

    connect(ui->prevButton,SIGNAL(clicked()),this,SIGNAL(previousTermsClicked()));
    connect(ui->playlistCombo,SIGNAL(aboutShowPopup()),this,SLOT(on_playlistShowPopup()));
}

YoutubeSearchWidget::~YoutubeSearchWidget() {
    delete ui;
}

void YoutubeSearchWidget::setFieldsValues(const QString & query,const QString & category,const QString & author,const QString & playlist_id,YoutubeOrderBy orderby,YoutubeTime time) {
    ui->searchEdit->setText(query);
    ui->categoryCombo->setCurrentText(category);
    ui->channelEdit->setText(author);
    ui->orderbyCombo->setCurrentIndex((int)orderby);
    ui->timeCombo->setCurrentIndex(time.operation());
    ui->timeEdit->setDateTime(time.date());
    on_playlistShowPopup();
    for (int i=0;i<ui->playlistCombo->count();i++) {
        if (ui->playlistCombo->itemData(i).toString() == playlist_id) {
            ui->playlistCombo->setCurrentIndex(i);
            break;
        }
    }
}

void YoutubeSearchWidget::on_buttonBox_accepted() {
    ((QWidget *)parent())->hide();
     emit search_requested(ui->searchEdit->text(),
                           ui->categoryCombo->currentText(),
                           ui->channelEdit->text(),
                           ui->playlistCombo->itemData(ui->playlistCombo->currentIndex()).toString(),
                           (YoutubeOrderBy)ui->orderbyCombo->currentIndex(),
                           YoutubeTime((YoutubeTimeId)ui->timeCombo->currentIndex(),ui->timeEdit->dateTime()));
}

void YoutubeSearchWidget::on_buttonBox_rejected() {
    ((QWidget *)parent())->hide();
}

void YoutubeSearchWidget::setChannel(const QString & channel) {
    ui->channelEdit->setText(channel);
}

void YoutubeSearchWidget::setEnableFillButton(bool flag) {
    ui->prevButton->setEnabled(flag);
}

void YoutubeSearchWidget::on_channelEdit_textChanged(const QString &text) {
    ui->playlistCombo->setEnabled(!text.isEmpty());
    okButton->setEnabled((!text.isEmpty() && ui->channelEdit->isEnabled()) ||
                         (!ui->searchEdit->text().isEmpty() && ui->searchEdit->isEnabled()));
    if (ui->playlistCombo->count() > 0) ui->playlistCombo->clear();
}

void YoutubeSearchWidget::on_playlistShowPopup() {
    if (ui->playlistCombo->count() > 0) return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    int comboPrevWidth = ui->playlistCombo->width();
    QStyleOptionComboBox option;
    option.initFrom(ui->playlistCombo);
    int comboAddWidth = qApp->style()->subControlRect(QStyle::CC_ComboBox,&option,QStyle::SC_ComboBoxArrow).width() + 10 + ui->playlistCombo->iconSize().width();

    bool updated = false;
    int comboNewWidth = comboPrevWidth;
    QString error;
    QString userKey = theSettings->value("youtube_user_key","").toString();
    QString channel_id = YouTubeSearch::downloadChannelId(ui->channelEdit->text(),userKey,error);
    YoutubeOrderBy order_by = (YoutubeOrderBy)ui->orderbyCombo->currentIndex();
    if (!channel_id.isEmpty() && error.isEmpty()) {
        QList<YPlayList> list = YouTubeSearch::downloadChannelPlaylists(channel_id,userKey,order_by,error);
        if (error.isEmpty() && list.count() > 0) {
            ui->playlistCombo->addItem("","");
            for (int i=0;i<list.count();i++) {
                YPlayList item = list.at(i);
                ui->playlistCombo->addItem(QIcon(QPixmap::fromImage(item.image())),item.title(),item.id());
                comboNewWidth = qMax(QFontMetrics(ui->playlistCombo->font()).width(item.title())+comboAddWidth,comboNewWidth);
                updated = true;
            }
        }
    }

    if (updated && parent()->inherits("QMenu")) {
        ui->playlistCombo->setMinimumWidth(comboNewWidth);
        QMetaObject::invokeMethod(this,"update_menu_size",Qt::QueuedConnection);
    }

    QApplication::restoreOverrideCursor();

    if (!error.isEmpty()) QMessageBox::critical(this,tr("Error during downloading!"),error);
}

void YoutubeSearchWidget::update_menu_size() {
    QMenu * menu = (QMenu *)parent();
    menu->setMinimumWidth(sizeHint().width());
    menu->setMinimumHeight(sizeHint().height());
    menu->updateGeometry();
}

void YoutubeSearchWidget::on_playlistCombo_currentIndexChanged(int index) {
    ui->timeEdit->setEnabled(index <= 0 || !ui->playlistCombo->isEnabled());
    ui->categoryCombo->setEnabled(index <= 0 || !ui->playlistCombo->isEnabled());
    ui->searchEdit->setEnabled(index <= 0 || !ui->playlistCombo->isEnabled());
    ui->timeCombo->setEnabled(index <= 0 || !ui->playlistCombo->isEnabled());
    ui->timeEdit->setEnabled(index <= 0 || !ui->playlistCombo->isEnabled());
}

void YoutubeSearchWidget::on_searchEdit_textChanged(const QString &text) {
    okButton->setEnabled((!text.isEmpty() && ui->searchEdit->isEnabled()) ||
                         (!ui->channelEdit->text().isEmpty() && ui->channelEdit->isEnabled()));
}

void YoutubeSearchWidget::on_orderbyCombo_currentIndexChanged(int) {
    if (ui->playlistCombo->count() > 0) ui->playlistCombo->clear();
}
