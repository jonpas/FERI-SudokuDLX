#pragma once

#include <QMainWindow>
#include <QLineEdit>

#include <QDebug>

#include "dlx.h"

using UIGridRow = QList<QLineEdit *>;
using UIGrid = QList<UIGridRow>;

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
    UIGrid grid;

    void generateGrid(int size);
    void resetGrid();
    // Solves current grid and saves benchmark in millseconds
    bool solveGrid(double &bench);
    void runTests();

    // Converters
    // Converts UI grid to int grid (DLX)
    Grid UIGridToGrid() const;
    // Applies int grid (DLX) to UI grid
    void gridToUIGrid(Grid sudoku);
    // Applies string grid (53.2.3...) to UI grid
    void stringGridToUIGrid(QString gridStr, int size);
    // Converts UI grid to string grid (53.2.3...)
    QString UIGridToStringGrid();

    // UI input getters/setters
    int cellValue(QLineEdit *cell) const;
    void setCellValue(QLineEdit *cell, int value);

private slots:
    void onCellTextEdited(const QString &text);
    void on_pushButtonImport_clicked();
    void on_pushButtonSolve_clicked();
    void on_pushButtonReset_clicked();
};
