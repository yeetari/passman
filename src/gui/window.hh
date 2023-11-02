#pragma once

#include <QMainWindow>
#include <memory>

class Database;
class EntryModel;

class MainWindow final : public QMainWindow {
    Q_OBJECT

private:
    Database &m_database;
    std::unique_ptr<EntryModel> m_entry_model;

public:
    explicit MainWindow(Database &database);
    ~MainWindow() override;

private slots:
    void exit_action();
};
