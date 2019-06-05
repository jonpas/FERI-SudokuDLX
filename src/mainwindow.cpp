#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QValidator>
#include <QInputDialog>

#include <chrono>

const int MainWindow::CellSize = 50;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    generateGrid(9);

    // Debug
    runTests();
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
        grid.append(UIGridRow());
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
    if (grid.size() == 9) {
        QList<std::tuple<QString, QString, QString>> tests;

        // Test cases from: http://sudopedia.enjoysudoku.com/Valid_Test_Cases.html
        tests.append({
            "Completed Puzzle",
            "974236158638591742125487936316754289742918563589362417867125394253649871491873625",
            "974236158638591742125487936316754289742918563589362417867125394253649871491873625"
        });
        tests.append({
            "Last Empty Square",
            "2564891733746159829817234565932748617128.6549468591327635147298127958634849362715",
            "256489173374615982981723456593274861712836549468591327635147298127958634849362715"
        });
        tests.append({
            "Naked Singles",
            "3.542.81.4879.15.6.29.5637485.793.416132.8957.74.6528.2413.9.655.867.192.965124.8",
            "365427819487931526129856374852793641613248957974165283241389765538674192796512438"
        });
        tests.append({
            "Hidden Singles",
            "..2.3...8.....8....31.2.....6..5.27..1.....5.2.4.6..31....8.6.5.......13..531.4..",
            "672435198549178362831629547368951274917243856254867931193784625486592713725316489"
        });

        // Test case from https://github.com/KarlHajal/DLX-Sudoku-Solver
        tests.append({
            "Hard to Brute-Force",
            "..............3.85..1.2.......5.7.....4...1...9.......5......73..2.1........4...9",
            "987654321246173985351928746128537694634892157795461832519286473472319568863745219"
        });

        // Test cases from http://magictour.free.fr/topn234
        tests.append({
            "Hard 1",
            "7.8...3.....6.1...5.........4.....263...8.......1...9..9.2....4....7.5...........",
            "768942315934651278512738649147593826329486157856127493693215784481379562275864931"
        });
        tests.append({
            "Hard 2",
            "7.8...3.....6.1...5.........4.....263...8.......1...9..9.2....4....7.5...........",
            "768942315934651278512738649147593826329486157856127493693215784481379562275864931"
        });
        tests.append({
            "Hard 3",
            "7.8...3.....2.1...5.........4.....263...8.......1...9..9.6....4....7.5...........",
            "728946315934251678516738249147593826369482157852167493293615784481379562675824931"
        });
        tests.append({
            "Hard 4",
            "3.7.4...........918........4.....7.....16.......25..........38..9....5...2.6.....",
            "317849265245736891869512473456398712732164958981257634174925386693481527528673149"
        });
        tests.append({
            "Hard 5",
            "5..7..6....38...........2..62.4............917............35.8.4.....1......9....",
            "582743619963821547174956238621479853348562791795318426217635984439287165856194372"
        });

        qInfo() << "Running Tests";
        double benchSum = 0.0;

        double bench;
        QString result;
        for (auto &test : tests) {
            stringGridToUIGrid(std::get<1>(test), 9);

            bool solved = solveGrid(bench);
            benchSum += bench;

            if (solved) {
                result = UIGridToStringGrid();

                if (result == std::get<2>(test)) {
                    qInfo() << "- Passed:" << std::get<0>(test) << "(in" << bench << "milliseconds)";
                } else {
                    qWarning() << "- Wrong:" << std::get<0>(test);
                }
            } else {
                qCritical() << "- Failed:" << std::get<0>(test);
            }

            resetGrid();
        }

        qInfo() << "Average time:" << benchSum / tests.size() << "milliseconds";
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

void MainWindow::stringGridToUIGrid(QString gridStr, int size) {
    int i = 0;
    int j = 0;
    for (auto &valueStr : gridStr) {
        setCellValue(grid.at(i).at(j), valueStr.digitValue());

        if (++j >= size) {
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

void MainWindow::on_pushButtonImport_clicked() {
    bool ok;
    QString text = QInputDialog::getText(this, "Sudoku (9x9) Import", "Input Sudoku (9x9) problem in format: 53.2..4...", QLineEdit::Normal, nullptr, &ok);
    if (ok && !text.isEmpty() && text.size() == 9 * 9) {
        stringGridToUIGrid(text, 9);
        ui->statusBar->showMessage("Imported!");
    } else {
        ui->statusBar->showMessage("Failed to import! Wrong data.");
    }
}

void MainWindow::on_pushButtonSolve_clicked() {
    double bench;
    bool solved = solveGrid(bench);

    if (solved) {
        ui->statusBar->showMessage("Solved in " + QString::number(bench) + " milliseconds!");
    } else {
        ui->statusBar->showMessage("No solution!");
    }
}

void MainWindow::on_pushButtonReset_clicked() {
    resetGrid();
}
