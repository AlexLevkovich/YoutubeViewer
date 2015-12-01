#include "youtubelistmodel.h"
#include "youtubeview.h"
#include <QSettings>
#include "default_values.h"

extern QSettings *theSettings;

const QString displayRoleTemplate = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:14pt; font-weight:600;\">%title%</span></p>"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>"
        "<table border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"2\">"
        "<tr>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">%category%</span></p></td>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%category_t%</p></td>"
        "<td rowspan=\"5\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-style:italic;\">%description%</span></p></td></tr>"
        "<tr>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">%uploader%</span></p></td>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%uploader_t%</p></td></tr>"
        "<tr>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">%length%</span></p></td>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%length_t%</p></td></tr>"
        "<tr>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">%rating%</span></p></td>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%rating_t%</p></td></tr>"
        "<tr>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">%added%</span></p></td>"
        "<td>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">%added_t%</p></td></tr></table>"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p></body></html>";


YoutubeListModel::YoutubeListModel(const QList<Media> & medias,YoutubeView *parent) : QAbstractItemModel(parent) {
    m_medias = medias;
    max_desc_sym_count = theSettings->value("desc_sym_max_count",DESC_MAX_SYM_COUNT).toInt();
}

int YoutubeListModel::columnCount(const QModelIndex & /*parent*/) const {
    return 1;
}

QString YoutubeListModel::displayRoleString(int index) const {
    if (displayRoles.contains(index)) return displayRoles[index];

    YoutubeListModel * p_this = (YoutubeListModel *)this;
    QObject * p_obj_this = (QObject *)this;
    QString displayRole = displayRoleTemplate;
    displayRole.replace("<span style=\" font-size:14pt; font-weight:600;\">%title%</span>",
                        QString("<span style=\" font-size:%1pt; font-weight:600;\">%title%</span>").arg((((QListView *)p_obj_this->parent())->font().pointSize()*14)/9));
    displayRole.replace("%title%",p_this->m_medias[index].title());
    displayRole.replace("%category%",tr("Category:"));
    displayRole.replace("%category_t%",p_this->m_medias[index].category());
    displayRole.replace("%uploader%",tr("Uploader:"));
    displayRole.replace("%uploader_t%",p_this->m_medias[index].author());
    displayRole.replace("%length%",tr("Length:"));
    displayRole.replace("%length_t%",p_this->m_medias[index].duration());
    displayRole.replace("%rating%",tr("Rating:"));
    displayRole.replace("%rating_t%",QString("%1").arg(p_this->m_medias[index].rating()));
    displayRole.replace("%added%",tr("Added:"));
    displayRole.replace("%added_t%",p_this->m_medias[index].date().date().toString());
    displayRole.replace("%description%",(max_desc_sym_count > 0)?(p_this->m_medias[index].description().left(max_desc_sym_count)+"..."):p_this->m_medias[index].description());

    p_this->displayRoles[index] = displayRole;

    return displayRole;
}

QVariant YoutubeListModel::data(const QModelIndex & index,int role) const {
    YoutubeListModel * p_this = (YoutubeListModel *)this;
    QObject * p_obj_this = (QObject *)this;
    switch (role) {
        case Qt::DecorationRole:
            return p_this->m_medias[index.row()].image();
        case Qt::DisplayRole:
            return displayRoleString(index.row());
        case Qt::SizeHintRole:
            return ((YoutubeView *)p_obj_this->parent())->itemDelegate()->sizeHint(QStyleOptionViewItem(),index);
        case Qt::UserRole:
            return QVariant::fromValue((void *)&p_this->m_medias[index.row()]);
    }
    return QVariant();
}

QModelIndex YoutubeListModel::index(int row,int /*column*/,const QModelIndex & /*parent*/) const {
    return createIndex(row,0);
}

QModelIndex YoutubeListModel::parent(const QModelIndex & /*index*/) const {
    return QModelIndex();
}

int YoutubeListModel::rowCount(const QModelIndex & parent) const {
    return parent.isValid()?0:m_medias.count();
}
