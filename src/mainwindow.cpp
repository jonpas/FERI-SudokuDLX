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
    if (size % 3 != 0) {
        qCritical() << "Grid size not divisible by 3!";
        return;
    }

    int sectionSize = size / 3;
    const QIntValidator *validator = new QIntValidator(1, size, this);

    // Initialize all grid rows
    // For later population (different order, as we add section by section)
    grid.reserve(size * size);
    for (int i = 0; i < size; ++i) {
        grid.append(SudokuGridRow());
    }

    // Add section by section and fill with cells
    // to achieve wanted styling
    for (int si = 0; si < sectionSize; ++si) {
        for (int sj = 0; sj < sectionSize; ++sj) {
            QFrame *widget = new QFrame;
            widget->setFrameShape(QFrame::Box);

            int widgetSize = CellSize * 3 + 2; // 2 for border spaces
            widget->setMinimumSize(widgetSize, widgetSize);
            widget->setMaximumSize(widgetSize, widgetSize);

            QGridLayout *gridLayout = new QGridLayout(widget);
            gridLayout->setSpacing(0);
            gridLayout->setMargin(0);

            for (int i = 0; i < sectionSize; ++i) {
                for (int j = 0; j < sectionSize; ++j) {
                    QLineEdit *cell = new QLineEdit(this);
                    cell->setAlignment(Qt::AlignCenter);
                    cell->setFont(QFont(cell->font().family(), CellSize / 2));
                    cell->setStyleSheet("QLineEdit { border: 1px solid grey; }");

                    cell->setValidator(validator);
                    cell->setMinimumSize(CellSize, CellSize);
                    cell->setMaximumSize(CellSize, CellSize);

                    gridLayout->addWidget(cell, i, j);

                    // Calculate row index from section and cell positions
                    // We add section by section and fill section as it's created
                    // but we have to add to the row in the entire grid
                    int rowIndex = i + si * sectionSize;
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
        int test[9][9] = {
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
                setCellValue(grid[i][j], test[i][j]);
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
        for (int j = 0; j < sudoku[i].size(); ++j) {
            setCellValue(grid[i][j], sudoku[i][j]);
        }
    }
}

// UI input getters/setters
int MainWindow::cellValue(QLineEdit *cell) const {
    if (cell->text() == "") {
        return 0;
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
                if (cellValue(cell) != 0) {
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
        gridToSudokuGrid(dlx.solution());

        double bench = std::chrono::duration<double>(benchEnd - benchStart).count();
        ui->statusBar->showMessage("Solved in " + QString::number(bench) + " seconds!");
    } else {
        ui->statusBar->showMessage("No solution!");
    }
}
