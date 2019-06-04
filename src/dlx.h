#pragma once

#include <QObject>

using GridRow = QList<int>;
using Grid = QList<GridRow>;

class DLX {
public:
    DLX(Grid sudoku);

    bool solve();
    Grid solution();

private:
    Grid sudoku;

    // DLX
    void coverColumn();
    void uncoverColumn();
    bool search();

    // Exact Cover Builder
    // Converts Sudoku grid to exact cover problem
    void buildSparseMatrix();
    void buildLinkedList();
    void transformListToCurrentGrid();
    void mapSolutionToGrid();
};
