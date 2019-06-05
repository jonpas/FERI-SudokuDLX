#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QValidator>
#include <QInputDialog>

#include <cmath>
#include <chrono>

#include "tests.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Tests
    runTests();

    if (!generateGrid(9)) {
        qCritical() << "Invalid grid size! Only NxN grids supported.";
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::generateGrid(int size) {
    // Exit if size less than 4
    if (size < 4) {
        return false;
    }

    // Exit if size not perfect square
    double sizeSqrt = sqrt(size);
    double intpart;
    if (modf(sizeSqrt, &intpart) != 0.0) {
        return false;
    }

    // Clean previous grid if it exists
    deleteGrid();

    // Calculate required data before-hand
    int regionsPerRow = static_cast<int>(intpart);
    int columnsInRegion = size / regionsPerRow;

    // Base size based on 9x9 grid
    int cellSize = 9 * 3 * 2;
    // Scale for other sizes
    if (size < 9) {
        cellSize = static_cast<int>(cellSize * size * 0.565);
    } else if (size > 9) {
        cellSize = static_cast<int>(cellSize / (size * 0.109));
    }

    const QIntValidator *validator = new QIntValidator(1, size, this);

    // Initialize all grid rows
    // For later population (different order, as we add region by region)
    grid.reserve(size * size);
    for (int i = 0; i < size; ++i) {
        grid.append(UIGridRow());
    }

    // Add region by region and fill with cells
    // to achieve wanted styling
    for (int si = 0; si < regionsPerRow; ++si) {
        for (int sj = 0; sj < regionsPerRow; ++sj) {
            QFrame *widget = new QFrame(this);
            widget->setFrameShape(QFrame::Box);

            int widgetSize = cellSize * regionsPerRow + 2; // 2 for border spaces
            widget->setMinimumSize(widgetSize, widgetSize);
            widget->setMaximumSize(widgetSize, widgetSize);

            QGridLayout *gridLayout = new QGridLayout(widget);
            gridLayout->setSpacing(0);
            gridLayout->setMargin(0);

            for (int i = 0; i < columnsInRegion; ++i) {
                for (int j = 0; j < columnsInRegion; ++j) {
                    QLineEdit *cell = new QLineEdit(widget);
                    cell->setAlignment(Qt::AlignCenter);
                    cell->setFont(QFont(cell->font().family(), cellSize / 2));
                    cell->setStyleSheet("QLineEdit { border: 1px solid grey; }");

                    cell->setValidator(validator);
                    cell->setMinimumSize(cellSize, cellSize);
                    cell->setMaximumSize(cellSize, cellSize);

                    gridLayout->addWidget(cell, i, j);

                    // Calculate row index from region and cell positions
                    // We add region by region and fill region as it's created
                    // but we have to add to the row in the entire grid
                    int rowIndex = i + si * columnsInRegion;
                    grid[rowIndex].append(cell);

                    connect(cell, &QLineEdit::textEdited, this, &MainWindow::onCellTextEdited);
                }
            }

            ui->gridLayoutSudoku->addWidget(widget, si, sj);
        }
    }

    return true;
}

void MainWindow::deleteGrid() {
    QLayoutItem *item;
    while ((item = ui->gridLayoutSudoku->takeAt(0))) {
        delete item->widget();
    }

    grid.clear();
}

void MainWindow::resetGrid() {
    for (auto &row : grid) {
        for (auto &cell : row) {
            cell->setText("");
        }
    }
}

bool MainWindow::solveGrid(double &bench) {
    // Convert input data to primitive data
    // Instantiate DLX solver
    DLX dlx(UIGridToGrid());

    // Solve (convert problem to exact cover problem and solve with DLX)
    auto benchStart = std::chrono::high_resolution_clock::now();
    bool solved = dlx.solve();
    auto benchEnd = std::chrono::high_resolution_clock::now();

    if (solved) {
        // Apply to UI
        gridToUIGrid(dlx.solution());

        bench = std::chrono::duration<double, std::milli>(benchEnd - benchStart).count();
    }

    return solved;
}

void MainWindow::runTests() {
    double benchSum = 0.0;
    bool allPassed = true;

    // 9x9
    qInfo() << "Running 9x9 Tests:";
    generateGrid(9);
    for (auto &test : Tests::s9x9) {
        runTest(test, benchSum, allPassed);
        resetGrid();
    }

    // 16x16
    qInfo() << "Running 16x16 Tests:";
    generateGrid(16);
    for (auto &test : Tests::s16x16) {
        runTest(test, benchSum, allPassed);
        resetGrid();
    }

    if (allPassed) {
        qInfo() << "All tests PASSED!";
    } else {
        qInfo() << "Some tests FAILED or gave WRONG results!";
    }
    qInfo() << "Average time:" << benchSum / Tests::size() << "milliseconds";
}

void MainWindow::runTest(const std::tuple<QString, QString, QString> &test, double &benchSum, bool &allPassed) {
    stringGridToUIGrid(std::get<1>(test));

    double bench = 0.0;
    bool solved = solveGrid(bench);
    benchSum += bench;

    bool noSolution = std::get<2>(test) == "none";
    if ((solved && !noSolution) || (!solved && noSolution)) {
        QString result = UIGridToStringGrid();

        if (result == std::get<2>(test) || std::get<2>(test) == "any" || noSolution) {
            qInfo() << "- Passed:" << std::get<0>(test) << "(in" << bench << "milliseconds)";
        } else {
            qWarning() << "O Wrong:" << std::get<0>(test) << "(in" << bench << "milliseconds)";
            allPassed = false;
        }
    } else {
        qCritical() << "X Failed:" << std::get<0>(test) << "(in" << bench << "milliseconds)";
        allPassed = false;
    }
}

// Converters
Grid MainWindow::UIGridToGrid() const {
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

void MainWindow::gridToUIGrid(Grid sudoku) {
    resetGrid();

    for (int i = 0; i < sudoku.size(); ++i) {
        for (int j = 0; j < sudoku.at(i).size(); ++j) {
            setCellValue(grid.at(i).at(j), sudoku.at(i).at(j));
        }
    }
}

void MainWindow::stringGridToUIGrid(QString gridStr) {
    int i = 0;
    int j = 0;
    for (auto &valueStr : gridStr) {
        setCellValue(grid.at(i).at(j), valueStr.digitValue());

        if (++j >= grid.size()) {
            j = 0;
            ++i;
        }
    }
}

QString MainWindow::UIGridToStringGrid() {
    QString gridStr = "";
    for (auto &row : grid) {
        for (auto &cell : row) {
            int value = cellValue(cell);
            if (value < 1) {
                gridStr.append(".");
            } else {
                gridStr.append(QString::number(value));
            }
        }
    }

    return gridStr;
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
    // Manual high-bound (below 9) validation (validator doesn't handle it)
    int input = text.toInt();
    if (input < 1 || input > grid.size()) {
        int value;
        for (auto &row : grid) {
            for (auto &cell : row) {
                value = cellValue(cell);
                if (value == 0) {
                    cell->setText("1");
                }
                if (value > grid.size()) {
                    cell->setText(QString::number(grid.size()));
                }
            }
        }
    }
}

void MainWindow::on_spinBoxSize_valueChanged(int size) {
    // Set value by supported steps (varied)
    if (size < grid.size()) {
        QMap<int, int> steps = { {25, 16}, {16, 9}, {9, 4} };
        size = steps[size + 1];
    } else if (size > grid.size()) {
        QMap<int, int> steps = { {4, 9}, {9, 16}, {16, 25} };
        size = steps[size - 1];
    }
    ui->spinBoxSize->blockSignals(true);
    ui->spinBoxSize->setValue(size);
    ui->spinBoxSize->blockSignals(false);

    // Pretty size display ('9x9' instead of '9')
    ui->spinBoxSize->setSuffix("x" + QString::number(size));

    if (generateGrid(size)) {
        ui->statusBar->showMessage("Resized!");
    } else {
        ui->statusBar->showMessage("Invalid grid size! Only NxN grids supported.");
    }
}

void MainWindow::on_pushButtonImport_clicked() {
    bool ok;
    QString text = QInputDialog::getText(this, "Sudoku Import", "Input Sudoku problem in format: 53.2..4...", QLineEdit::Normal, nullptr, &ok);
    if (ok && !text.isEmpty()) {
        bool generated = true;
        if (text.size() != grid.size() * grid.size()) {
            double sizeSqrt = sqrt(text.size());
            double intpart;
            if (modf(sizeSqrt, &intpart) == 0.0) {
                generated = generateGrid(static_cast<int>(sqrt(text.size())));
            } else {
                generated = false;
            }
        }

        if (generated) {
            stringGridToUIGrid(text);
            ui->statusBar->showMessage("Imported!");
        } else {
            ui->statusBar->showMessage("Invalid grid size! Only NxN grids supported.");
        }
    } else {
        ui->statusBar->showMessage("Failed to import! Wrong data.");
    }
}

void MainWindow::on_pushButtonSolve_clicked() {
    double bench;
    bool solved = solveGrid(bench);

    if (solved) {
        ui->statusBar->showMessage("Solved in " + QString::number(bench) + " milliseconds!");
        qInfo() << "Solution:" << UIGridToStringGrid();
    } else {
        ui->statusBar->showMessage("No solution!");
    }
}

void MainWindow::on_pushButtonReset_clicked() {
    resetGrid();
}
