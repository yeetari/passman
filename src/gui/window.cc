#include "window.hh"

#include "entry_model.hh"

#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QTreeView>

MainWindow::MainWindow(Database &database) : m_database(database) {
    auto *file_menu = menuBar()->addMenu("&File");
    auto *database_menu = menuBar()->addMenu("&Database");

    auto *exit_action = file_menu->addAction("E&xit");
    connect(exit_action, &QAction::triggered, this, &MainWindow::exit_action);

    auto *splitter = new QSplitter;
    setCentralWidget(splitter);

    auto *tree_view = new QTreeView(splitter);

    auto *entry_view = new QTreeView(splitter);
    entry_view->setAlternatingRowColors(true);
    entry_view->setRootIsDecorated(false);
    entry_view->setUniformRowHeights(true);

    m_entry_model = std::make_unique<EntryModel>(database);
    entry_view->setModel(m_entry_model.get());

    splitter->setSizes({200, 1000});
    resize(1024, 768);
}

MainWindow::~MainWindow() = default;

void MainWindow::exit_action() {
    QApplication::quit();
}
