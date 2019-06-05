#pragma once

#include <QObject>

using GridRow = QList<int>;
using Grid = QList<GridRow>;

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
        int row[3]; // Row identification for mapping solutions to sudoku grid [candidate, row, column]
    };

    DLX(Grid sudoku);

    bool solve();
    Grid solvedGrid();

private:
    Grid sudoku;

    Node *headNode;

    QList<Node *> solution;
    QList<Node *> origValues;

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
