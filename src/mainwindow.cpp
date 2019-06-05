#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QValidator>

#include <chrono>

const int MainWindow::CellSize = 50;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    generateGrid(9);

    // Debug
    fillGridWithTestData();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::generateGrid(int size) {
    if (size != 9) {
        qCritical() << "Undefined behaviour for grid sizes other than 9!";
    }

    int regionSize = size / 3;
    const QIntValidator *validator = new QIntValidator(1, size, this);

    // Initialize all grid rows
    // For later population (different order, as we add region by region)
    grid.reserve(size * size);
    for (int i = 0; i < size; ++i) {
        grid.append(SudokuGridRow());
    }

    // Add region by region and fill with cells
    // to achieve wanted styling
    for (int si = 0; si < regionSize; ++si) {
        for (int sj = 0; sj < regionSize; ++sj) {
            QFrame *widget = new QFrame;
            widget->setFrameShape(QFrame::Box);

            int widgetSize = CellSize * regionSize + 2; // 2 for border spaces
            widget->setMinimumSize(widgetSize, widgetSize);
            widget->setMaximumSize(widgetSize, widgetSize);

            QGridLayout *gridLayout = new QGridLayout(widget);
            gridLayout->setSpacing(0);
            gridLayout->setMargin(0);

            for (int i = 0; i < regionSize; ++i) {
                for (int j = 0; j < regionSize; ++j) {
                    QLineEdit *cell = new QLineEdit(this);
                    cell->setAlignment(Qt::AlignCenter);
                    cell->setFont(QFont(cell->font().family(), CellSize / 2));
                    cell->setStyleSheet("QLineEdit { border: 1px solid grey; }");

                    cell->setValidator(validator);
                    cell->setMinimumSize(CellSize, CellSize);
                    cell->setMaximumSize(CellSize, CellSize);

                    gridLayout->addWidget(cell, i, j);

                    // Calculate row index from region and cell positions
                    // We add region by region and fill region as it's created
                    // but we have to add to the row in the entire grid
                    int rowIndex = i + si * regionSize;
                    grid[rowIndex].append(cell);

                    connect(cell, &QLineEdit::textEdited, this, &MainWindow::onCellTextEdited);
                }
            }

            ui->gridLayoutSudoku->addWidget(widget, si, sj);
        }
    }
}

void MainWindow::fillGridWithTestData() {
    if (grid.size() == 9) {
        // Exepcted results in comments on the right
        Grid test = {
            // Test cases from: http://sudopedia.enjoysudoku.com/Valid_Test_Cases.html
            // Last empty square
            /*
            { 2,5,6,  4,8,9,  1,7,3 },
            { 3,7,4,  6,1,5,  9,8,2 },
            { 9,8,1,  7,2,3,  4,5,6 },

            { 5,9,3,  2,7,4,  8,6,1 },
            { 7,1,2,  8,0,6,  5,4,9 }, // 3
            { 4,6,8,  5,9,1,  3,2,7 },

            { 6,3,5,  1,4,7,  2,9,8 },
            { 1,2,7,  9,5,8,  6,3,4 },
            { 8,4,9,  3,6,2,  7,1,5 }
            */

            // Naked singles
            /*
            { 3,0,5,  4,2,0,  8,1,0 }, // 6, 7, 9
            { 4,8,7,  9,0,1,  5,0,6 }, // 3, 2
            { 0,2,9,  0,5,6,  3,7,4 }, // 1, 8

            { 8,5,0,  7,9,3,  0,4,1 }, // 2, 6
            { 6,1,3,  2,0,8,  9,5,7 }, // 4
            { 0,7,4,  0,6,5,  2,8,0 }, // 9, 1, 3

            { 2,4,1,  3,0,9,  0,6,5 }, // 8, 7
            { 5,0,8,  6,7,0,  1,9,2 }, // 3, 4
            { 0,9,6,  5,1,2,  4,0,8 }  // 7, 3
            */

            // Hidden singles (region, column or row have only one possible square remaining for given value)
            /*
            { 0,0,2,  0,3,0,  0,0,8 },
            { 0,0,0,  0,0,8,  0,0,0 },
            { 0,3,1,  0,2,0,  0,0,0 },

            { 0,6,0,  0,5,0,  2,7,0 },
            { 0,1,0,  0,0,0,  0,5,0 },
            { 2,0,4,  0,6,0,  0,3,1 },

            { 0,0,0,  0,8,0,  6,0,5 },
            { 0,0,0,  0,0,0,  0,1,3 },
            { 0,0,5,  3,1,0,  4,0,0 }
            */

            // Test case from https://github.com/KarlHajal/DLX-Sudoku-Solver
            // Hard to solve

            { 0,0,0,  0,0,0,  0,0,0 },
            { 0,0,0,  0,0,3,  0,8,5 },
            { 0,0,1,  0,2,0,  0,0,0 },

            { 0,0,0,  5,0,7,  0,0,0 },
            { 0,0,4,  0,0,0,  1,0,0 },
            { 0,9,0,  0,0,0,  0,0,0 },

            { 5,0,0,  0,0,0,  0,7,3 },
            { 0,0,2,  0,1,0,  0,0,0 },
            { 0,0,0,  0,4,0,  0,0,9 }

        };

        for (int i = 0; i < 9; ++i) {
            for (int j = 0; j < 9; ++j) {
                setCellValue(grid.at(i).at(j), test.at(i).at(j));
            }
        }
    }
}

Grid MainWindow::sudokuGridToGrid() const {
    Grid sudoku;
    sudoku.reserve(grid.size());

    for (auto &row : grid) {
        GridRow sudokuRow;
        for (auto &cell : row) {
            sudokuRow.append(cellValue(cell));
        }
        sudoku.append(sudokuRow);
    }

    return sudoku;
}

void MainWindow::gridToSudokuGrid(Grid sudoku) {
    for (int i = 0; i < sudoku.size(); ++i) {
        for (int j = 0; j < sudoku.at(i).size(); ++j) {
            setCellValue(grid.at(i).at(j), sudoku.at(i).at(j));
        }
    }
}

// UI input getters/setters
int MainWindow::cellValue(QLineEdit *cell) const {
    if (cell->text() == "") {
        return -1;
    }
    return cell->text().toInt();
}

void MainWindow::setCellValue(QLineEdit *cell, int value) {
    if (value < 1) {
        cell->setText("");
    } else {
        cell->setText(QString::number(value));
    }
}

// Slots
void MainWindow::onCellTextEdited(const QString &text) {
    // Manual low-bound validation (validator doesn't handle it)
    if (text.toInt() < 1) {
        for (auto &row : grid) {
            for (auto &cell : row) {
                if (cellValue(cell) == 0) {
                    cell->setText("1");
                }
            }
        }
    }
}

void MainWindow::on_pushButtonReset_clicked() {
    for (auto &row : grid) {
        for (auto &cell : row) {
            cell->setText("");
        }
    }
}

void MainWindow::on_pushButtonSolve_clicked() {
    // Convert input data to primitive data
    // Instantiate DLX solver
    DLX dlx(sudokuGridToGrid());

    // Solve (convert problem to exact cover problem and solve with DLX)
    auto benchStart = std::chrono::high_resolution_clock::now();
    bool solved = dlx.solve();
    auto benchEnd = std::chrono::high_resolution_clock::now();

    if (solved) {
        // Apply to UI
        gridToSudokuGrid(dlx.solvedGrid());

        double bench = std::chrono::duration<double, std::milli>(benchEnd - benchStart).count();
        ui->statusBar->showMessage("Solved in " + QString::number(bench) + " milliseconds!");
    } else {
        ui->statusBar->showMessage("No solution!");
    }
}
