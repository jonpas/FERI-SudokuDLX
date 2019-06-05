#include "dlx.h"

#include <cmath>

const int DLX::MaxSearchDepth = 1000;

DLX::DLX(Grid sudoku) : sudoku(sudoku) {
    // Frequently used size variations
    size = sudoku.size();
    sizeSq = size * size;
    sizeSqrt = static_cast<int>(sqrt(size));
    rows = sudoku.size() * sudoku.size() * sudoku.size();
    columns = 4 * sudoku.size() * sudoku.size();

    // Initialize
    solution.reserve(MaxSearchDepth);
    for (int i = 0; i < MaxSearchDepth; ++i) {
        solution.append(nullptr);
    }

    matrix.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        SparseRow row = SparseRow();
        for (int j = 0; j < columns; ++j) {
            row.append(false);
        }
        matrix.append(row);
    }

    nodesToClean.reserve(columns * 10); // Approximation
}

DLX::~DLX() {
    for (auto &node : nodesToClean) {
        delete node;
    }

    delete head;
}

bool DLX::solve() {
    buildSparseMatrix();
    buildLinkedList();
    transformListToCurrentGrid();
    return search();
}

Grid DLX::solvedGrid() {
    mapSolutionToGrid();
    return sudoku;
}

// DLX
void DLX::coverColumn(Node *column) {
    column->left->right = column->right;
    column->right->left = column->left;

    for (Node *node = column->down; node != column; node = node->down) {
        for (Node *tmp = node->right; tmp != node; tmp = tmp->right) {
            tmp->up->down = tmp->down;
            tmp->down->up = tmp->up;
            --tmp->head->size;
        }
    }
}

void DLX::uncoverColumn(Node *column) {
    for (Node *node = column->up; node != column; node = node->up) {
        for (Node *tmp = node->left; tmp != node; tmp = tmp->left) {
            ++tmp->head->size;
            tmp->up->down = tmp;
            tmp->down->up = tmp;
        }
    }

    column->left->right = column;
    column->right->left = column;
}

bool DLX::search(int depth) {
    // Exit if solution found
    if (head->right == head) {
        return true;
    }

    // Choose column with smallest size (deterministically)
    Node *column = head->right;
    for (Node *tmp = column->right; tmp != head; tmp = tmp->right) {
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

        // Search next depth (recursion) and exit if solved
        if (search(depth + 1)) {
            return true;
        }

        tmp = solution.at(depth);
        solution[depth] = nullptr;
        column = tmp->head;

        for (Node * node = tmp->left; node != tmp; node = node->left) {
            uncoverColumn(node->head);
        }
    }

    // Not yet solved
    return false;
}

// Exact Cover Builder
void DLX::buildSparseMatrix() {
    int counter = 0;
    int j = 0;
    int x = 0;

    // Constraint 1 - Only one value in any given cell
    for (int i = 0; i < matrix.size(); ++i) {
        matrix[i][j] = true;

        ++counter;
        if (counter >= sudoku.size()) {
            ++j;
            counter = 0;
        }
    }

    // Constraint 2 - Only one instance of a number in single row
    counter = 1;
    for (j = sizeSq; j < 2 * sizeSq; ++j) {
        for (int i = x; i < counter * sizeSq; i += size) {
            matrix[i][j] = true;
        }

        if ((j + 1) % size == 0) {
            x = counter * sizeSq;
            ++counter;
        } else {
            ++x;
        }
    }

    // Constraint 3 - Only one instance of a number in single column
    j = 2 * sizeSq;
    for (int i = 0; i < rows; ++i) {
        matrix[i][j] = true;
        ++j;

        if (j >= 3 * sizeSq) {
            j = 2 * sizeSq;
        }
    }

    // Constraint 4 - Only one instance of a number in given region (at 9x9, region is 3x3)
    x = 0;
    for (j = 3 * sizeSq; j < columns; ++j) {
        for (int l = 0; l < sizeSqrt; ++l) {
            for (int k = 0; k < sizeSqrt; ++k) {
                matrix[x + l * size + k * sizeSq][j] = true;
            }
        }

        int tmp = j + 1 - 3 * sizeSq;
        if (tmp % (sizeSqrt * size) == 0) {
            x += (sizeSqrt - 1) * sizeSq + (sizeSqrt - 1) * size + 1;
        } else if (tmp % size == 0) {
            x += size * (sizeSqrt - 1) + 1;
        } else {
            ++x;
        }
    }
}

void DLX::buildLinkedList() {
    head = new Node;
    head->up = head;
    head->down = head;
    head->left = head;
    head->right = head;
    head->size = -1;
    head->head = head;

    Node *tmp = head;

    // Create all column nodes
    for (int i = 0; i < columns; ++i) {
        Node *node = new Node;
        nodesToClean.append(node);
        node->size = 0;
        node->up = node;
        node->down = node;
        node->left = tmp;
        node->right = head;
        node->head = node;
        tmp->right = node;
        tmp = node;
    }

    GridRow id = {0, 1, 1};

    // Add a node for each 'true' present in sparse matrix and update column nodes accordingly
    for (int i = 0; i < rows; ++i) {
        Node *top = head->right;
        Node *prev = nullptr;

        if (i != 0 && i % sizeSq == 0) {
            id[0] -= size - 1;
            ++id[1];
            id[2] -= size - 1;
        } else if (i != 0 && i % size == 0) {
            id[0] -= size - 1;
            ++id[2];
        } else {
            ++id[0];
        }

        for (int j = 0; j < columns; ++j, top = top->right) {
            if (matrix.at(i).at(j)) {
                Node *node = new Node;
                nodesToClean.append(node);
                node->row = id;

                if (prev == nullptr) {
                    prev = node;
                    prev->right = node;
                }

                node->left = prev;
                node->right = prev->right;
                node->right->left = node;
                prev->right = node;
                node->head = top;
                node->down = top;
                node->up = top->up;

                top->up->down = node;
                ++top->size;
                top->up = node;

                if (top->down == top) {
                    top->down = node;
                }
                prev = node;
            }
        }
    }
}

void DLX::transformListToCurrentGrid() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (sudoku.at(i).at(j) > 0) {
                Node *column = nullptr;
                Node *tmp = nullptr;

                bool exit = false;
                for (column = head->right; column != head; column = column->right) {
                    for (tmp = column->down; tmp != column; tmp = tmp->down) {
                        if (tmp->row.at(0) == sudoku.at(i).at(j)
                                && tmp->row.at(1) - 1 == i
                                && tmp->row.at(2) - 1 == j) {
                            exit = true;
                            break;
                        }
                    }
                    if (exit) break;
                }

                coverColumn(column);
                origValues.append(tmp);

                for (Node *node = tmp->right; node != tmp; node = node->right) {
                    coverColumn(node->head);
                }
            }
        }
    }
}

// Mapper
void DLX::mapSolutionToGrid() {
    // Map found solution values
    for (int i = 0; solution.at(i) != nullptr; ++i) {
        sudoku[solution.at(i)->row.at(1) - 1][solution.at(i)->row.at(2) - 1] = solution.at(i)->row.at(0);
    }

    // Map original values untouched by solution
    for (int i = 0; i < origValues.size(); ++i) {
        sudoku[origValues.at(i)->row.at(1) - 1][origValues.at(i)->row.at(2) - 1] = origValues.at(i)->row.at(0);
    }
}
