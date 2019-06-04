#include "dlx.h"

DLX::DLX(Grid sudoku) : sudoku(sudoku) {

}

bool DLX::solve() {
    buildSparseMatrix();
    buildLinkedList();
    transformListToCurrentGrid();
    return search();
}

Grid DLX::solution() {
    return sudoku;
}

// DLX
void DLX::coverColumn() {

}

void DLX::uncoverColumn() {

}

bool DLX::search() {
    sudoku[0][0] = 1;
    return true;
}

// Exact Cover Builder
void DLX::buildSparseMatrix() {

}

void DLX::buildLinkedList() {

}

void DLX::transformListToCurrentGrid() {

}

void DLX::mapSolutionToGrid() {

}
