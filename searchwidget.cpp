#include "searchwidget.h"
#include "ui_searchwidget.h"

SearchWidget::SearchWidget(QWidget *parent) : QWidget(parent), ui(new Ui::SearchWidget) {
    ui->setupUi(this);
    connect(ui->lineEdit,SIGNAL(search_requested(const QString &,
                                                 const QString &,
                                                 const QString &,
                                                 YoutubeOrderBy,
                                                 YoutubeTime)),
                    this,SIGNAL(search_requested(const QString &,
                                                 const QString &,
                                                 const QString &,
                                                 YoutubeOrderBy,
                                                 YoutubeTime)));
    connect(ui->searchButton,SIGNAL(search_requested(const QString &,
                                                     const QString &,
                                                     const QString &,
                                                     YoutubeOrderBy,
                                                     YoutubeTime)),
                        this,SIGNAL(search_requested(const QString &,
                                                     const QString &,
                                                     const QString &,
                                                     YoutubeOrderBy,
                                                     YoutubeTime)));
    ui->searchButton->setSearchLinePointer(ui->lineEdit);
}

SearchWidget::~SearchWidget() {
    delete ui;
}

