#include "dlx.h"

const int DLX::MaxSearchDepth = 1000;

DLX::DLX(Grid sudoku) : sudoku(sudoku) {
    // Initialize
    solution.reserve(MaxSearchDepth);
    for (auto &node : solution) {
        node = nullptr;
    }
}

bool DLX::solve() {
    buildSparseMatrix();
    buildLinkedList();
    transformListToCurrentGrid();
    return search();
}

Grid DLX::solvedGrid() {
    return sudoku;
}

// DLX
void DLX::coverColumn(Node *column) {
    column->left->right = column->right;
    column->right->left = column->left;

    for (Node *node = column->down; node != column; node = node->down) {
        for (Node *tmp = node->right; tmp != node; tmp = tmp->right) {
            tmp->down->up = tmp->up;
            tmp->up->down = tmp->down;
            --tmp->head->size;
        }
    }
}

void DLX::uncoverColumn(Node *column) {
    for (Node *node = column->up; node != column; node = node->up) {
        for (Node *tmp = node->left; tmp != node; tmp = tmp->left) {
            ++tmp->head->size;
            tmp->down->up = tmp;
            tmp->up->down = tmp;
        }
    }

    column->left->right = column;
    column->right->left = column;
}

bool DLX::search(int depth) {
    if (headNode->right == headNode) {
        mapSolutionToGrid();
        return true;
    }

    // Choose column with smallest size (deterministically)
    Node *column = headNode->right;
    for (Node *tmp = column->right; tmp != headNode; tmp = tmp->right) {
        if (tmp->size < column->size) {
            column = tmp;
        }
    }

    coverColumn(column);

    for (Node *tmp = column->down; tmp != column; tmp = tmp->down) {
        solution[depth] = tmp;

        for (Node *node = tmp->right; node != tmp; node = node->right) {
            coverColumn(node->head);
        }

        search(depth + 1);

        tmp = solution[depth];
        solution[depth] = nullptr;
        column = tmp->head;

        for (Node * node = tmp->left; node != tmp; node = node->left) {
            uncoverColumn(node->head);
        }
    }

    return false;
}

// Exact Cover Builder
void DLX::buildSparseMatrix() {
    // TODO
}

void DLX::buildLinkedList() {
    // TODO
}

void DLX::transformListToCurrentGrid() {
    // TODO
}

void DLX::mapSolutionToGrid() {
    // Map found solution values
    for (int i = 0; solution[i] != nullptr; ++i) {
        sudoku[solution[i]->row[1] - 1][solution[i]->row[2] - 1] = solution[i]->row[0];
    }

    // Map original values untouched by solution
    for (int i = 0; origValues[i] != nullptr; ++i) {
        sudoku[origValues[i]->row[1] - 1][origValues[i]->row[2] - 1] = origValues[i]->row[0];
    }
}
