#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QValidator>
#include <QInputDialog>

#include <chrono>

const int MainWindow::CellSize = 50;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    generateGrid(9);

    // Unit tests
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
        QList<std::tuple<QString, QString, QString>> tests; // [title, input, expected result]

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

        // Test case from https://en.wikipedia.org/wiki/Sudoku_solving_algorithms
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
        tests.append({
            "Empty",
            ".................................................................................",
            "any" // Multiple solutions
        });
        tests.append({
            "Single Given",
            "........................................1........................................",
            "any" // 500+ solutions
        });
        tests.append({
            "Insufficient Givens",
            "...........5....9...4....1.2....3.5....7.....438...2......9.....1.4...6..........",
            "any" // 500+ solutions
        });
        tests.append({
            "Duplicate Given - Box",
            "..9.7...5..21..9..1...28....7...5..1..851.....5....3.......3..68........21.....87",
            "none" // No solution
        });
        tests.append({
            "Duplicate Given - Column",
            "6.159.....9..1............4.7.314..6.24.....5..3....1...6.....3...9.2.4......16..",
            "none" // No solution
        });
        tests.append({
            "Duplicate Given - Row",
            ".4.1..35.............2.5......4.89..26.....12.5.3....7..4...16.6....7....1..8..2.",
            "none" // No solution
        });
        tests.append({
            "Unsolvable Square",
            "..9.287..8.6..4..5..3.....46.........2.71345.........23.....5..9..4..8.7..125.3..",
            "none" // No solution
        });
        tests.append({
            "Unsolvable Box",
            ".9.3....1....8..46......8..4.5.6..3...32756...6..1.9.4..1......58..2....2....7.6.",
            "none" // No solution
        });
        tests.append({
            "Unsolvable Column",
            "....41....6.....2...2......32.6.........5..417.......2......23..48......5.1..2...",
            "none" // No solution
        });
        tests.append({
            "Unsolvable Row",
            "9..1....4.14.3.8....3....9....7.8..18....3..........3..21....7...9.4.5..5...16..3",
            "none" // No solution
        });
        tests.append({
            "Not Unique — 2 Solutions",
            ".39...12....9.7...8..4.1..6.42...79...........91...54.5..1.9..3...8.5....14...87.",
            "439658127156927384827431956342516798785294631691783542578149263263875419914362875" // 1st solution (found first by DLX)
            //"439658127156927384827431956642513798785294631391786542578149263263875419914362875" // Second solution

        });
        tests.append({
            "Not Unique — 3 Solutions",
            "..3.....6...98..2.9426..7..45...6............1.9.5.47.....25.4.6...785...........",
            "783542196516987324942631758457296813238714965169853472891325647624178539375469281" // 1st solution (found first by DLX)
            //"783542916516987324942631758457216839238794165169853472891325647624178593375469281" // 2nd solution
            //"783542916516987324942631758457216893238794165169853472891325647624178539375469281" // 3rd solution
        });
        tests.append({
            "Not Unique — 4 Solutions",
            "....9....6..4.7..8.4.812.3.7.......5..4...9..5..371..4.5..6..4.2.17.85.9.........",
            //"178693452623457198945812736716984325384526917592371684857169243231748569469235871" // 1st solution
            //"178693452623457198945812736716984325384526971592371684857169243231748569469235817" // 2nd solution
            "178693452623457198945812736762984315314526987589371624857169243231748569496235871" // 3rd solution (found first by DLX)
            //"178693452623457198945812736786924315314586927592371684857169243231748569469235871" // 4th solution
        });
        tests.append({
            "Not Unique — 10 Solutions",
            "59.....486.8...3.7...2.1.......4.....753.698.....9.......8.3...2.6...7.934.....65",
            "592637148618459327437281596923748651175326984864195273759863412286514739341972865" // 1st solution (found first by DLX)
            //"592637148618459327437281596963748251175326984824195673759863412286514739341972865" // 2nd solution
            //"592637148618459327734281596129748653475326981863195274957863412286514739341972865" // 3rd solution
            //"592637148618459327734281596129748653475326981863195472957863214286514739341972865" // 4th solution
            //"592637148618459327734281596169748253475326981823195674957863412286514739341972865" // 5th solution
            //"592637148618459327734281596829145673175326984463798251957863412286514739341972865" // 6th solution
            //"592637148618459327734281596829145673475326981163798254957863412286514739341972865" // 7th solution
            //"592637148618459327734281596829145673475326981163798452957863214286514739341972865" // 8th solution
            //"592637148618459327734281596869145273175326984423798651957863412286514739341972865" // 9th solution
            //"592637148618459327734281596869145273475326981123798654957863412286514739341972865" // 10th solution
        });
        tests.append({
            "Not Unique — 125 Solutions",
            "...3165..8..5..1...1.89724.9.1.85.2....9.1....4.263..1.5.....1.1..4.9..2..61.8...",
            //"592637148618459327437281596923748651175326984864195273759863412286514739341972865" // 1st solution
            //"274316589893524167615897243931785426562941378748263951359672814187459632426138795" // 2nd solution
            //"274316589893524167615897243931785426762941358548263791359672814187459632426138975" // 3rd solution
            //"274316589893524167615897243931785426762941358548263971359672814187459632426138795" // 4th solution
            //"274316589893524167615897243931785426762941835548263791459672318187439652326158974" // 5th solution
            //"274316589893524167615897243931785426762941835548263971459672318187439652326158794" // 6th solution
            "294316578867524139513897246931785624682941753745263981459632817178459362326178495" // Nth solution (found first by DLX)
            //(additional solutions omitted for brevity)                                          // 7th-120th solution
            //"724316598869524173315897246931785624682941357547263981458632719173459862296178435" // 121th solution
            //"724316598869524173315897246931785624682941735547263981453672819178459362296138457" // 122th solution
            //"724316598869524173315897246931785624682941735547263981458672319173459862296138457" // 123th solution
            //"724316598893524176615897243961785324382941765547263981459632817138479652276158439" // 124th solution
            //"724316598893524176615897243961785324382941765547263981459632817178459632236178459" // 125th solution

        });

        qInfo() << "Running Tests";
        double benchSum = 0.0;

        double bench;
        QString result;
        for (auto &test : tests) {
            stringGridToUIGrid(std::get<1>(test), 9);

            bool solved = solveGrid(bench);
            benchSum += bench;

            bool noSolution = std::get<2>(test) == "none";
            if (solved || (!solved && noSolution)) {
                result = UIGridToStringGrid();

                if (result == std::get<2>(test) || std::get<2>(test) == "any" || noSolution) {
                    qInfo() << "- Passed:" << std::get<0>(test) << "(in" << bench << "milliseconds)";
                } else {
                    qWarning() << "- Wrong:" << std::get<0>(test) << "(in" << bench << "milliseconds)";
                    qInfo() << "  -> Correct:" << result;
                }
            } else {
                qCritical() << "- Failed:" << std::get<0>(test) << "(in" << bench << "milliseconds)";
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
