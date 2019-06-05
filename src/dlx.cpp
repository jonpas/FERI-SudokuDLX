#include "dlx.h"

#include <cmath>

const int DLX::MaxSearchDepth = 1000;

DLX::DLX(Grid sudoku) : sudoku(sudoku) {
    // Frequently used size variations - Reference DLX::buildSparseMatrix()
    size = sudoku.size();
    sizeSq = size * size;
    sizeSqrt = static_cast<int>(sqrt(size));
    rows = size * size * size;
    columns = 4 * size * size;

    // Initialize
    nodesToClean.reserve(columns * (size + 1)); // 9x9 => 324 * (9 + 1)
    solutions.reserve(MaxSearchDepth); // Maximum
    origValues.reserve(sizeSq); // Maximum: 9x9 => 81

    matrix.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        SparseRow row = SparseRow();
        for (int j = 0; j < columns; ++j) {
            row.append(false);
        }
        matrix.append(row);
    }
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
    coverGridValues();
    return search();
}

Grid DLX::solution() {
    mapSolutionToGrid();
    return sudoku;
}

// DLX
void DLX::coverColumn(Node *column) {
    // Remove column
    column->left->right = column->right;
    column->right->left = column->left;

    // Remove all rows in the column from other columns they are in
    for (Node *node = column->down; node != column; node = node->down) {
        for (Node *tmp = node->right; tmp != node; tmp = tmp->right) {
            tmp->up->down = tmp->down;
            tmp->down->up = tmp->up;
            --tmp->head->size;
        }
    }
}

void DLX::uncoverColumn(Node *column) {
    // Take advantage of the fact that every node that has been removed retains information about its neighbors

    // Re-add all rows in the column from other columns they were in
    for (Node *node = column->up; node != column; node = node->up) {
        for (Node *tmp = node->left; tmp != node; tmp = tmp->left) {
            ++tmp->head->size;
            tmp->up->down = tmp;
            tmp->down->up = tmp;
        }
    }

    // Re-add column
    column->left->right = column;
    column->right->left = column;
}

bool DLX::search(int depth) {
    // Exit if solution found
    if (head->right == head) {
        return true;
    }

    // Cover next column (with least number of nodes or the right one)
    Node *column = chooseNextColumn();
    coverColumn(column);

    for (Node *row = column->down; row != column; row = row->down) {
        solutions.append(row);

        // Cover to the right
        for (Node *right = row->right; right != row; right = right->right) {
            coverColumn(right->head);
        }

        // Search next depth (recursion) and exit if solved
        if (search(depth + 1)) {
            return true;
        }

        // Remove last solution (backtrack)
        solutions.removeOne(row);
        column = row->head;

        // Uncover to the left (backtrack)
        for (Node *left = row->left; left != row; left = left->left) {
            uncoverColumn(left->head);
        }
    }

    // Uncover last column (backtrack)
    uncoverColumn(column);

    // Not yet solved
    return false;
}

// Exact Cover Builder
void DLX::buildSparseMatrix() {
    int counter = 0;
    int j = 0;
    int x = 0;

    // Sparse Matrix:
    // Columns: Constraints of the puzzle (4 per number)
    // - Each number has its own set of constraints => size ^ 2 * 4 columns (9x9 => 324 columns)
    // Rows: Every position for every number => size ^ 3 rows (9x9 = 729 rows)
    // - Each row represents only one candidate position => 4 1s in a row, representing constraints of that position

    // Constraint 1: Position - Only one number in single cell
    for (int i = 0; i < matrix.size(); ++i) {
        matrix[i][j] = true;

        if (++counter >= sudoku.size()) {
            ++j;
            counter = 0;
        }
    }

    // Constraint 2: Row - Only one instance of a number in single row
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

    // Constraint 3: Column - Only one instance of a number in single column
    j = 2 * sizeSq;
    for (int i = 0; i < rows; ++i) {
        matrix[i][j] = true;

        if (++j >= 3 * sizeSq) {
            j = 2 * sizeSq;
        }
    }

    // Constraint 4: Region - Only one instance of a number in single region (at 9x9, region is 3x3)
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
    // Create head
    head = new Node;
    head->up = head;
    head->down = head;
    head->left = head;
    head->right = head;
    head->size = -1;
    head->head = head;

    // Create all column nodes
    Node *right = head;
    for (int i = 0; i < columns; ++i, right = right->right) {
        Node *node = new Node;
        nodesToClean.append(node);
        node->size = 0;

        // Link to all sides
        node->up = node;
        node->down = node;
        node->left = right;
        node->right = head;
        node->head = node;
        right->right = node;
    }

    GridRow id = {0, 1, 1};

    // Add a node for each 'true' present in sparse matrix and update column nodes accordingly
    for (int i = 0; i < rows; ++i) {
        // Update row identification
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

        // Update column nodes according to sparse matrix value
        Node *top = head->right;
        Node *prev = nullptr;
        for (int j = 0; j < columns; ++j, top = top->right) {
            // Add node for each 'true' sparse matrix value
            if (matrix.at(i).at(j)) {
                Node *node = new Node;
                nodesToClean.append(node);
                node->row = id;

                // First node in row
                if (prev == nullptr) {
                    prev = node;
                    prev->right = node;
                }

                // Link to all sides
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

                // Insert into column
                if (top->down == top) {
                    top->down = node;
                }
                prev = node;
            }
        }
    }
}

void DLX::coverGridValues() {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            // Cover column of value already present in the grid
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

// Helpers
DLX::Node *DLX::chooseNextColumn() {
    Node *column = head->right;
    for (Node *right = column->right; right != head; right = right->right) {
        // Select if less values in current right column than in original right column
        if (right->size < column->size) {
            column = right;
        }
    }
    return column;
}

void DLX::mapSolutionToGrid() {
    // Map found solution values
    for (int i = 0; i < solutions.size(); ++i) {
        sudoku[solutions.at(i)->row.at(1) - 1][solutions.at(i)->row.at(2) - 1] = solutions.at(i)->row.at(0);
    }

    // Map original values untouched by solution
    for (int i = 0; i < origValues.size(); ++i) {
        sudoku[origValues.at(i)->row.at(1) - 1][origValues.at(i)->row.at(2) - 1] = origValues.at(i)->row.at(0);
    }
}
