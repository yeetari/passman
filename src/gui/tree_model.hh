#pragma once

#include <QAbstractItemModel>

class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    TreeModel() {}

    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
};
