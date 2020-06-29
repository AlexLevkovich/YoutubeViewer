#include "youtubecommentsdialog.h"
#include "ui_youtubecommentsdialog.h"
#include "youtubesearch.h"

YoutubeCommentsDialog::YoutubeCommentsDialog(Media * media,QWidget *parent) : QDialog(parent), ui(new Ui::YoutubeCommentsDialog) {
    ui->setupUi(this);
    ui->titleLabel->setText(media->title());
    QFont font = ui->titleLabel->font();
    font.setPointSize((font.pointSize()*14)/9);
    font.setBold(true);
    ui->titleLabel->setFont(font);
    ui->categoryLabel->setText(media->category());
    ui->videoidLabel->setText(media->id());
    ui->channelidLabel->setText(media->channel_id());
    ui->lengthLabel->setText(media->duration());
    ui->ratingLabel->setText(QString("%1").arg(media->rating()));
    ui->uploaderLabel->setText(media->author());
    ui->urlLabel->setText(QString("<html><head/><body><p><a href=\"%1\"><span style=\" text-decoration: underline; color:#0057ae;\">%1</span></a></p></body></html>").arg(media->url().toString()));
    ui->addedLabel->setText(media->date().toString());
    ui->descEdit->setText(media->description());
    ui->backButton->setMaximumWidth(ui->backButton->contentsMargins().left() + ui->backButton->fontMetrics().horizontalAdvance(ui->backButton->text()) + ui->backButton->contentsMargins().right());
    ui->nextButton->setMaximumWidth(ui->nextButton->contentsMargins().left() + ui->nextButton->fontMetrics().horizontalAdvance(ui->nextButton->text()) + ui->nextButton->contentsMargins().right());
    ui->backButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->waitView->setVisible(true);
    ui->commentsBrowser->setVisible(false);
    comments_reader.setUrl(media->comments_url());
    connect(&comments_reader,SIGNAL(completed(const QString &)),this,SLOT(setHtml(const QString &)));
    connect(&comments_reader,SIGNAL(error(const QString &)),this,SLOT(was_error(const QString &)));
    comments_reader.start_search("");
    m_was_error = false;
    prevPageTokens.append("");
    curr_page = 0;
}

YoutubeCommentsDialog::~YoutubeCommentsDialog() {
    delete ui;

}

void YoutubeCommentsDialog::setHtml(const QString & html) {
    ui->waitView->setVisible(false);
    ui->commentsBrowser->setVisible(true);
    ui->commentsBrowser->setHtml(html);
    ui->backButton->setEnabled(!m_was_error && (curr_page > 0));
    ui->nextButton->setEnabled(!m_was_error && !comments_reader.nextPageToken().isEmpty());
}

void YoutubeCommentsDialog::was_error(const QString & err) {
    ui->commentsBrowser->setHtml(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                                         "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
                                         "p, li { white-space: pre-wrap; }"
                                         "</style></head><body style=\" font-style:normal;\">"
                                         "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;"
                                         " -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600; color:#ff0000;\">%1"
                                         "</span></p></body></html>").arg(err));
    ui->backButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    m_was_error = true;
}

void YoutubeCommentsDialog::on_backButton_clicked() {
    curr_page--;
    comments_reader.start_search(prevPageTokens[curr_page]);
    ui->commentsBrowser->clear();
    ui->backButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->waitView->setVisible(true);
    ui->commentsBrowser->setVisible(false);
}

void YoutubeCommentsDialog::on_nextButton_clicked() {
    curr_page++;
    if (prevPageTokens.count() == curr_page) prevPageTokens.append(comments_reader.nextPageToken());
    comments_reader.start_search(comments_reader.nextPageToken());
    ui->commentsBrowser->clear();
    ui->backButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->waitView->setVisible(true);
    ui->commentsBrowser->setVisible(false);
}
