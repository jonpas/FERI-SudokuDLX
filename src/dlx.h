#pragma once

#include <QObject>

#include <QDebug>

using GridRow = QList<int>;
using Grid = QList<GridRow>;

using SparseRow = QList<bool>;
using SparseMatrix = QList<SparseRow>;

class DLX {
public:
    static const int MaxSearchDepth;

    struct Node {
        Node *head;

        Node *up;
        Node *down;
        Node *left;
        Node *right;

        int size; // Column header
        GridRow row = {0, 0, 0}; // Row identification for mapping solutions to sudoku grid [candidate, row, column]
    };

    DLX(Grid sudoku);
    ~DLX();

    bool solve();
    Grid solvedGrid();

private:
    Grid sudoku;

    // Size and variations
    int size;
    int sizeSq;
    int sizeSqrt;
    int rows;
    int columns;

    Node *head;
    QList<Node *> nodesToClean;

    QList<Node *> solution;
    QList<Node *> origValues;

    SparseMatrix matrix;

    // DLX
    // Remove a column from the matrix
    void coverColumn(Node *column);
    // Reverse of cover
    void uncoverColumn(Node *column);
    bool search(int depth = 0);


    // Exact Cover Builder
    // Builds initial matrix containing all possibilities
    void buildSparseMatrix();
    // Builds a toroidal doubly linked list out of the sparse matrix
    void buildLinkedList();
    // Covers values that are already present in the grid
    void transformListToCurrentGrid();

    // Maps found solution back to 2D grid
    void mapSolutionToGrid();
};
