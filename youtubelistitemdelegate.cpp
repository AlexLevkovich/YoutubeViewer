#include "youtubelistitemdelegate.h"
#include <QAbstractTextDocumentLayout>
#include <QListView>
#include <QStyle>
#include <QPainter>
#include <QDebug>
#include <math.h>

#define TEXT_IDENT 5
#define max(a,b) ((a>b)?a:b)

YoutubeListItemDelegate::YoutubeListItemDelegate(QListView *parent) : QItemDelegate(parent) {
    connect(parent,SIGNAL(model_changed()),this,SLOT(model_changed()));
}

QTextDocument * YoutubeListItemDelegate::createDocument(const QModelIndex & index) {
    if (cache.contains(index)) return cache[index];

    QListView * view = (QListView *)parent();
    QTextDocument * doc = new QTextDocument(this);
    doc->setDocumentMargin(1);
    doc->setTextWidth(view->size().width() - view->contentsMargins().left() - view->contentsMargins().right() - TEXT_IDENT - view->iconSize().width());
    doc->setHtml(index.data(Qt::DisplayRole).toString());
    cache[index] = doc;
    return doc;
}

void YoutubeListItemDelegate::model_changed() {
    QMapIterator<QModelIndex,QTextDocument *> i(cache);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    cache.clear();
}

QSize YoutubeListItemDelegate::sizeHint(const QStyleOptionViewItem & /*option*/, const QModelIndex & index) const {
    YoutubeListItemDelegate * p_this = (YoutubeListItemDelegate *)this;
    QListView * view = (QListView *)parent();

    int item_width = view->size().width() - view->contentsMargins().left() - view->contentsMargins().right();
    return QSize(item_width,max(p_this->createDocument(index)->documentLayout()->documentSize().toSize().height(),view->iconSize().height()));
}

void YoutubeListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    YoutubeListItemDelegate * p_this = (YoutubeListItemDelegate *)this;

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect,option.palette.highlight());
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    }
    else {
        painter->fillRect(option.rect,option.palette.color(!fmod(index.row(),2)?QPalette::AlternateBase:QPalette::Base));
        painter->setPen(option.palette.color(QPalette::Text));
    }

    int delta = (option.rect.height() - option.decorationSize.height())/2;
    painter->drawPixmap(0,option.rect.y()+delta,QPixmap::fromImage(index.data(Qt::DecorationRole).value<QImage>()));
    painter->save();
    painter->translate(option.decorationSize.width()+TEXT_IDENT,option.rect.y());
    p_this->createDocument(index)->drawContents(painter);
    painter->restore();
}
