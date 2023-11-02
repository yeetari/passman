#include "entry_model.hh"

#include "../db.hh"

#include <array>

namespace {

constexpr std::array k_headers{
    std::make_pair("Name", "Entry name"),
    std::make_pair("Email", "Email"),
    std::make_pair("Modified", "Last modified date"),
};

} // namespace

QVariant EntryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (section >= k_headers.size()) {
        return {};
    }
    switch (role) {
    case Qt::DisplayRole:
        return tr(k_headers[section].first);
    case Qt::ToolTipRole:
        return tr(k_headers[section].second);
    default:
        return {};
    }
}

QVariant EntryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return {};
    }
    return QString::fromStdString(m_database.entries()[index.row()].name());
}

int EntryModel::rowCount(const QModelIndex &parent) const {
    return m_database.entries().size();
}

int EntryModel::columnCount(const QModelIndex &) const {
    return k_headers.size();
}
