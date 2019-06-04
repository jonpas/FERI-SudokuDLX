#pragma once

#include <QMainWindow>
#include <QLineEdit>

#include <QDebug>

#include "dlx.h"

using SudokuGridRow = QList<QLineEdit *>;
using SudokuGrid = QList<SudokuGridRow>;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    static const int CellSize;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SudokuGrid grid;

    void generateGrid(int size);
    void fillGridWithTestData();
    // Converts Sudoku input grid (UI) to int grid (DLX)
    Grid sudokuGridToGrid() const;
    // Applies int grid (DLX) to Sudoku input grid (UI)
    void gridToSudokuGrid(Grid sudoku);

    // UI input getters/setters
    int cellValue(QLineEdit *cell) const;
    void setCellValue(QLineEdit *cell, int value);

private slots:
    void onCellTextEdited(const QString &text);
    void on_pushButtonReset_clicked();
    void on_pushButtonSolve_clicked();
};
