#ifndef YOUTUBECOMMENTSDIALOG_H
#define YOUTUBECOMMENTSDIALOG_H

#include <QDialog>
#include "youtubecommentsreader.h"

class Media;

namespace Ui {
class YoutubeCommentsDialog;
}

class YoutubeCommentsDialog : public QDialog {
    Q_OBJECT

public:
    explicit YoutubeCommentsDialog(Media * media,QWidget *parent = 0);
    ~YoutubeCommentsDialog();

private slots:
    void setHtml(const QString & html);
    void was_error(const QString & err);
    void on_backButton_clicked();
    void on_nextButton_clicked();

private:
    Ui::YoutubeCommentsDialog *ui;
    YoutubeCommentsReader comments_reader;
    bool m_was_error;
    QStringList prevPageTokens;
    int curr_page;
};

#endif // YOUTUBECOMMENTSDIALOG_H
