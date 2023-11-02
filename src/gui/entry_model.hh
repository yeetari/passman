#pragma once

#include <QAbstractTableModel>

class Database;

class EntryModel : public QAbstractTableModel {
    Q_OBJECT

private:
    Database &m_database;

public:
    explicit EntryModel(Database &database) : m_database(database) {}

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
};
