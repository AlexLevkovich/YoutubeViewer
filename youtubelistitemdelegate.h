#ifndef YOUTUBELISTITEMDELEGATE_H
#define YOUTUBELISTITEMDELEGATE_H

#include <QItemDelegate>
#include <QTextDocument>
#include <QMap>

class QListView;
class QPainter;

class YoutubeListItemDelegate : public QItemDelegate {
    Q_OBJECT
public:
    explicit YoutubeListItemDelegate(QListView *parent = 0);

    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private slots:
    void model_changed();

private:
    QTextDocument * createDocument(const QModelIndex & index);

    QMap<QModelIndex,QTextDocument *> cache;
};

#endif // YOUTUBELISTITEMDELEGATE_H
