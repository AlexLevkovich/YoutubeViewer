#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include <QFileInfo>
#include <QIcon>
#include "default_values.h"
#include "youtubesearch.h"

#ifdef WIN32
#define PROVIDERS_COUNT 3
#else
#define PROVIDERS_COUNT 2
#endif
extern QString LOCALE_DATETIME_FORMAT();
extern QSettings *theSettings;
extern QString YOUTUBE_DL_BIN;
static QString paths[PROVIDERS_COUNT];
const QString players[PROVIDERS_COUNT] = {"VLC","MPV"
#ifdef WIN32
                                          ,"MPC-HC"
#endif
                                         };
const QString icons[PROVIDERS_COUNT] = {":/images/res/vlc.png",":/images/res/mpv.png"
#ifdef WIN32
                                        ,":/images/res/mpc-hc.png"
#endif
                                       };

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog) {
    ui->setupUi(this);

    paths[0] = theSettings->value("vlc_path",VLC_PATH).toString();
    paths[1] = theSettings->value("mpv_path",MPV_PATH).toString();
#ifdef WIN32
    paths[2] = theSettings->value("mpc_path",MPC_PATH).toString();
#endif

    int i;
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
    QStringList categories = YouTubeSearch::categories();
    for (i=0;i<categories.count();i++) {
        ui->categoryCombo->addItem(categories[i]);
    }

#ifdef WIN32
    ui->label_9->setText(ui->label_9->text()+".exe:");
#else
    ui->label_9->setText(ui->label_9->text()+":");
#endif
    ui->authorEdit->setHistoryCompleter("author_completer");

    ui->categoryCombo->setCurrentIndex(theSettings->value("search_def_category",0).toInt());
    ui->authorEdit->setText(theSettings->value("search_def_author","").toString());
    ui->orderbyCombo->setCurrentIndex(theSettings->value("search_def_orderby",0).toInt());
    YoutubeTime time;
    ui->timeCombo->setCurrentIndex(theSettings->value("search_def_time_id",(int)time.operation()).toInt());
    ui->timeEdit->setDateTime(theSettings->value("search_def_time",time.date()).toDateTime());
    ui->timeEdit->setDisplayFormat(LOCALE_DATETIME_FORMAT());
    ui->previewHeightSpin->setValue(theSettings->value("preview_size",QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT)).toSize().height());
    ui->symMaxCountSpin->setValue(theSettings->value("desc_sym_max_count",DESC_MAX_SYM_COUNT).toInt());
    ui->threadsSpin->setValue(theSettings->value("threads_count",THREADS_COUNT).toInt());
    ui->toolsLine->setText(theSettings->value("youtube_dl_path",YOUTUBE_DL_BIN).toString());

    ui->playerCombo->setItemData(0,0);
    for (i=0;i<PROVIDERS_COUNT;i++) {
        ui->playerCombo->addItem(QIcon(icons[i]),players[i],i+1);
    }

    QString player = theSettings->value("def_player","Ask").toString();
    for (i=(PROVIDERS_COUNT-1);i>=0;i--) {
        if (QFileInfo(paths[i]).exists()) {
            if (player == players[i]) ui->playerCombo->setCurrentIndex(i+1);
            continue;
        }
        int currentIndex = ui->playerCombo->currentIndex();
        if (currentIndex > (i+1)) currentIndex--;
        ui->playerCombo->removeItem(i+1);
        ui->playerCombo->setCurrentIndex(currentIndex);
    }

    for (i=0;video_heights[i] > 0;i++) {
        ui->sizeCombo->addItem(QString("%1p").arg(video_heights[i]),video_heights[i]);
    }

    QString prefVideoSize = theSettings->value("pref_video_size",QString("%1p").arg(DEF_VIDEO_HEIGHT)).toString();
    for (i=0;i<ui->sizeCombo->count();i++) {
        if (ui->sizeCombo->itemText(i) == prefVideoSize) {
            ui->sizeCombo->setCurrentIndex(i);
            break;
        }
    }

    for (i=0;!video_codecs[i].isEmpty();i++) {
        ui->codecCombo->addItem(video_codecs[i]);
    }

    QString prefVideoCodec = theSettings->value("pref_video_codec",DEF_VIDEO_CODEC).toString();
    for (i=0;i<ui->codecCombo->count();i++) {
        if (ui->codecCombo->itemText(i) == prefVideoCodec) {
            ui->codecCombo->setCurrentIndex(i);
            break;
        }
    }

    ui->fullscreenCheck->setChecked(theSettings->value("player_fullscreen",false).toBool());
    ui->subtitlesCheck->setChecked(theSettings->value("player_subtitles",true).toBool());
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::on_buttonBox_accepted() {
    theSettings->setValue("search_def_category",ui->categoryCombo->currentIndex());
    int index = ui->playerCombo->itemData(ui->playerCombo->currentIndex()).toInt();
    theSettings->setValue("def_player",(index == 0)?"Ask":players[index-1]);
    theSettings->setValue("search_def_author",ui->authorEdit->text());
    theSettings->setValue("search_def_orderby",ui->orderbyCombo->currentIndex());
    theSettings->setValue("search_def_time_id",ui->timeCombo->currentIndex());
    theSettings->setValue("search_def_time",ui->timeEdit->dateTime());
    theSettings->setValue("preview_size",QSize((ui->previewHeightSpin->value()*4)/3,ui->previewHeightSpin->value()));
    theSettings->setValue("desc_sym_max_count",ui->symMaxCountSpin->value());
    theSettings->setValue("threads_count",ui->threadsSpin->value());
    theSettings->setValue("youtube_dl_path",ui->toolsLine->text());
    theSettings->setValue("pref_video_size",ui->sizeCombo->itemText(ui->sizeCombo->currentIndex()));
    theSettings->setValue("pref_video_codec",ui->codecCombo->itemText(ui->codecCombo->currentIndex()));
    theSettings->setValue("player_fullscreen",ui->fullscreenCheck->isChecked());
    theSettings->setValue("player_subtitles",ui->subtitlesCheck->isChecked());
    accept();
}

void SettingsDialog::on_buttonBox_rejected() {
    reject();
}
