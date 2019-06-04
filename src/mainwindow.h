#pragma once

#include <QMainWindow>
#include <QLineEdit>

#include <QDebug>

using Cell = QList<QLineEdit *>;
using Grid = QList<Cell>;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static const QSize CellSize;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void generateGrid(int size);

private:
    Ui::MainWindow *ui;

    Grid grid;

private slots:
    void on_cell_textEdited(const QString &text);
};
