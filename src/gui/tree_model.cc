#include "tree_model.hh"

QVariant TreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return {};
    }

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
    return QModelIndex();
}

int TreeModel::rowCount(const QModelIndex &parent) const {
    return 0;
}

int TreeModel::columnCount(const QModelIndex &) const {
    return 1;
}
