#pragma once

#include <QMainWindow>
#include <QLineEdit>

#include <QDebug>

#include "dlx.h"
#include "tests.h"

using UIGridRow = QList<QLineEdit *>;
using UIGrid = QList<UIGridRow>;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    UIGrid grid;

    bool generateGrid(int size);
    void deleteGrid();
    void resetGrid();
    // Solves current grid and saves benchmark in millseconds
    bool solveGrid(double &bench);
    void runTests();
    void runTest(const Tests::Test &test, double &benchSum, bool &allPassed);

    // Converters
    // Converts UI grid to int grid (DLX)
    Grid UIGridToGrid() const;
    // Applies int grid (DLX) to UI grid
    void gridToUIGrid(Grid sudoku);
    // Applies string grid (53.2..4...) to UI grid
    void stringGridToUIGrid(QString gridStr);
    // Converts UI grid to string grid (53.2..4...)
    QString UIGridToStringGrid();

    // UI input getters/setters
    int cellValue(QLineEdit *cell) const;
    void setCellValue(QLineEdit *cell, int value);

private slots:
    void onCellTextEdited(const QString &text);
    void on_spinBoxSize_valueChanged(int size);
    void on_pushButtonImport_clicked();
    void on_pushButtonSolve_clicked();
    void on_pushButtonReset_clicked();
};
