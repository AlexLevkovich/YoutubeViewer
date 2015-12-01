#ifndef YOUTUBELISTMODEL_H
#define YOUTUBELISTMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include "youtubesearch.h"

class YoutubeView;

class YoutubeListModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit YoutubeListModel(const QList<Media> & medias,YoutubeView *parent = 0);
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row,int column,const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

private:
    QString displayRoleString(int index) const;

    QList<Media> m_medias;
    QMap<int,QString> displayRoles;
    int max_desc_sym_count;
};

#endif // YOUTUBELISTMODEL_H
